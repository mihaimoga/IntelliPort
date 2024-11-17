/* Copyright (C) 2014-2025 Stefan-Mihai MOGA
This file is part of IntelliPort application developed by Stefan-Mihai MOGA.
IntelliPort is an alternative Windows version to the famous HyperTerminal!

IntelliPort is free software: you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the Open
Source Initiative, either version 3 of the License, or any later version.

IntelliPort is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
IntelliPort. If not, see <http://www.opensource.org/licenses/gpl-3.0.html>*/

// MainFrame.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "IntelliPort.h"
#include "MainFrame.h"
#include "ConfigureDlg.h"
#include "InputDlg.h"
#include "WebBrowserDlg.h"
#include "CheckForUpdatesDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWndEx)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWndEx)
	ON_WM_CREATE()
	// Global help commands
	ON_COMMAND(ID_HELP_FINDER, &CFrameWndEx::OnHelpFinder)
	ON_COMMAND(ID_HELP, &CFrameWndEx::OnHelp)
	ON_COMMAND(ID_CONTEXT_HELP, &CFrameWndEx::OnContextHelp)
	ON_COMMAND(ID_DEFAULT_HELP, &CFrameWndEx::OnHelpFinder)
	ON_COMMAND(ID_VIEW_CAPTION_BAR, &CMainFrame::OnViewCaptionBar)
	ON_UPDATE_COMMAND_UI(ID_VIEW_CAPTION_BAR, &CMainFrame::OnUpdateViewCaptionBar)
	ON_COMMAND(ID_TOOLS_OPTIONS, &CMainFrame::OnOptions)
	ON_COMMAND(ID_FILE_PRINT, &CMainFrame::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CMainFrame::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CMainFrame::OnFilePrintPreview)
	ON_UPDATE_COMMAND_UI(ID_FILE_PRINT_PREVIEW, &CMainFrame::OnUpdateFilePrintPreview)
	// Custom implementation
	ON_COMMAND(ID_CONFIG_SERIAL_PORT, &CMainFrame::OnConfigureSerialPort)
	ON_COMMAND(ID_OPEN_SERIAL_PORT, &CMainFrame::OnOpenSerialPort)
	ON_COMMAND(ID_CLOSE_SERIAL_PORT, &CMainFrame::OnCloseSerialPort)
	ON_COMMAND(ID_SEND_RECEIVE, &CMainFrame::OnSendReceive)
	ON_UPDATE_COMMAND_UI(ID_CONFIG_SERIAL_PORT, &CMainFrame::OnUpdateConfigureSerialPort)
	ON_UPDATE_COMMAND_UI(ID_OPEN_SERIAL_PORT, &CMainFrame::OnUpdateOpenSerialPort)
	ON_UPDATE_COMMAND_UI(ID_CLOSE_SERIAL_PORT, &CMainFrame::OnUpdateCloseSerialPort)
	ON_UPDATE_COMMAND_UI(ID_SEND_RECEIVE, &CMainFrame::OnUpdateSendReceive)
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_COMMAND(IDC_TWITTER, &CMainFrame::OnTwitter)
	ON_COMMAND(IDC_LINKEDIN, &CMainFrame::OnLinkedin)
	ON_COMMAND(IDC_FACEBOOK, &CMainFrame::OnFacebook)
	ON_COMMAND(IDC_INSTAGRAM, &CMainFrame::OnInstagram)
	ON_COMMAND(IDC_ISSUES, &CMainFrame::OnIssues)
	ON_COMMAND(IDC_DISCUSSIONS, &CMainFrame::OnDiscussions)
	ON_COMMAND(IDC_WIKI, &CMainFrame::OnWiki)
	ON_COMMAND(IDC_USER_MANUAL, &CMainFrame::OnUserManual)
	ON_COMMAND(IDC_CHECK_FOR_UPDATES, &CMainFrame::OnCheckForUpdates)
END_MESSAGE_MAP()

// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here
	theApp.m_nAppLook = theApp.GetInt(_T("ApplicationLook"), ID_VIEW_APPLOOK_OFF_2007_AQUA);
	m_pCurrentDateTime = CTime::GetCurrentTime();
	m_pRingBuffer.Create(0x10000);
	m_nThreadRunning = false;
	m_hSerialPortThread = nullptr;
	m_hSocketThread = nullptr;

	theApp.m_nBaudRate = -1;
	theApp.m_nDataBits = -1;
	theApp.m_nParity = -1;
	theApp.m_nStopBits = -1;
	theApp.m_nFlowControl = -1;
	theApp.m_nSocketType = -1;
	theApp.m_strServerIP = _T("127.0.0.1");
	theApp.m_nServerPort = 8080;
	theApp.m_strClientIP = _T("127.0.0.1");
	theApp.m_nClientPort = 8080;
}

CMainFrame::~CMainFrame()
{
	OnCloseSerialPort();
	m_pRingBuffer.Destroy();
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWndEx::OnCreate(lpCreateStruct) == -1)
		return -1;

	// enable Visual Studio 2005 style docking window behavior
	CDockingManager::SetDockingMode(DT_SMART);
	// enable Visual Studio 2005 style docking window auto-hide behavior
	EnableAutoHidePanes(CBRS_ALIGN_ANY);

	// set the visual manager used to draw all user interface elements
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

	m_wndRibbonBar.Create(this);
	m_wndRibbonBar.LoadFromResource(IDR_RIBBON);

	if (!m_wndStatusBar.Create(this))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	bool bNameValid;
	CString strTitlePane;
	bNameValid = strTitlePane.LoadString(IDS_STATUS_PANE1);
	ASSERT(bNameValid);
	m_wndStatusBar.AddElement(new CMFCRibbonStatusBarPane(
		ID_STATUSBAR_PANE1, strTitlePane, TRUE, NULL,
		_T("012345678901234567890123456789012345678901234567890123456789")), strTitlePane);
	VERIFY(strTitlePane.LoadString(IDS_LOG_HISTORY_CLEARED));
	SetStatusBarText(strTitlePane);

	// Create a caption bar:
	if (!CreateCaptionBar())
	{
		TRACE0("Failed to create caption bar\n");
		return -1;      // fail to create
	}

	VERIFY(m_dlgIncoming.Create(CIncomingDlg::IDD, this));

	m_nTimerID = SetTimer(1, 10, NULL);

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWndEx::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return TRUE;
}

bool CMainFrame::CreateCaptionBar()
{
	if (!m_wndCaptionBar.Create(WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, this, ID_VIEW_CAPTION_BAR, -1, TRUE))
	{
		TRACE0("Failed to create caption bar\n");
		return false;
	}

	bool bNameValid;
	CString strTemp, strTemp2;
	m_wndCaptionBar.SetBitmap(IDB_INFO, RGB(255, 255, 255), FALSE, CMFCCaptionBar::ALIGN_LEFT);
	bNameValid = strTemp.LoadString(IDS_CAPTION_IMAGE_TIP);
	ASSERT(bNameValid);
	bNameValid = strTemp2.LoadString(IDS_CAPTION_IMAGE_TEXT);
	ASSERT(bNameValid);
	m_wndCaptionBar.SetImageToolTip(strTemp, strTemp2);

	HideMessageBar();
	return true;
}

void CMainFrame::OnDestroy()
{
	// VERIFY(m_dlgIncoming.DestroyWindow());
	VERIFY(KillTimer(m_nTimerID));

	CFrameWndEx::OnDestroy();
}

void CMainFrame::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	if (nIDEvent == m_nTimerID)
	{
		CTime pDateTime = CTime::GetCurrentTime();
		CTimeSpan pTimeSpan = pDateTime - m_pCurrentDateTime;
		if ((pTimeSpan.GetTotalSeconds() >= 10) && (m_wndCaptionBar.IsPaneVisible()))
		{
			HideMessageBar();
		}

		char pBuffer[0x10000] = { 0, };
		m_pMutualAccess.lock();
		const int nLength = m_pRingBuffer.GetMaxReadSize();
		if (nLength > 0)
		{
			m_pRingBuffer.ReadBinary(pBuffer, nLength);
			pBuffer[nLength] = '\0';
			const std::string strRawText(pBuffer);
			CString strBuffer(utf8_to_wstring(strRawText).c_str());
			AddText(strBuffer);
		}
		m_pMutualAccess.unlock();
	}

	CFrameWndEx::OnTimer(nIDEvent);
}

bool CMainFrame::SetStatusBarText(const CString& strMessage)
{
	if (m_wndStatusBar.GetSafeHwnd() != nullptr)
	{
		m_wndStatusBar.GetElement(0)->SetText(strMessage);
		m_wndStatusBar.Invalidate();
		m_wndStatusBar.UpdateWindow();
		return true;
	}
	return false;
}

bool CMainFrame::SetCaptionBarText(const CString& strMessage)
{
	if (m_wndCaptionBar.GetSafeHwnd() != nullptr)
	{
		m_pCurrentDateTime = CTime::GetCurrentTime();
		m_wndCaptionBar.ShowWindow(SW_SHOW);
		m_wndCaptionBar.SetText(strMessage, CMFCCaptionBar::ALIGN_LEFT);
		m_wndCaptionBar.Invalidate();
		m_wndCaptionBar.UpdateWindow();
		RecalcLayout();
	}
	return false;
}

bool CMainFrame::HideMessageBar()
{
	if (m_wndCaptionBar.GetSafeHwnd() != nullptr)
	{
		m_wndCaptionBar.ShowWindow(SW_HIDE);
		m_wndCaptionBar.Invalidate();
		m_wndCaptionBar.UpdateWindow();
		RecalcLayout();
	}
	return true;
}

bool CMainFrame::AddText(CString strText)
{
	strText.Replace(CRLF, LF);
	strText.Replace(CR, LF);
	strText.Replace(LF, CRLF);
	CEdit& pEdit = reinterpret_cast<CEditView*>(GetActiveView())->GetEditCtrl();
	int outLength = pEdit.GetWindowTextLength();
	pEdit.SetSel(outLength, outLength);
	pEdit.ReplaceSel(strText, TRUE);
	pEdit.SetSel(-1, 0);

	return true;
}

// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWndEx::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWndEx::Dump(dc);
}
#endif //_DEBUG

// CMainFrame message handlers

void CMainFrame::OnViewCaptionBar()
{
	m_wndCaptionBar.ShowWindow(m_wndCaptionBar.IsVisible() ? SW_HIDE : SW_SHOW);
	RecalcLayout(FALSE);
}

void CMainFrame::OnUpdateViewCaptionBar(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_wndCaptionBar.IsVisible());
}

void CMainFrame::OnOptions()
{
	CMFCRibbonCustomizeDialog *pOptionsDlg = new CMFCRibbonCustomizeDialog(this, &m_wndRibbonBar);
	ASSERT(pOptionsDlg != NULL);

	pOptionsDlg->DoModal();
	delete pOptionsDlg;
}

void CMainFrame::OnFilePrint()
{
	if (IsPrintPreview())
	{
		PostMessage(WM_COMMAND, AFX_ID_PREVIEW_PRINT);
	}
}

void CMainFrame::OnFilePrintPreview()
{
	if (IsPrintPreview())
	{
		PostMessage(WM_COMMAND, AFX_ID_PREVIEW_CLOSE);  // force Print Preview mode closed
	}
}

void CMainFrame::OnUpdateFilePrintPreview(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(IsPrintPreview());
}

void CMainFrame::OnConfigureSerialPort()
{
	CConfigureDlg dlgConfigure(this);
	if (dlgConfigure.DoModal() == IDOK)
	{
	}
}

void CMainFrame::OnOpenSerialPort()
{
	try
	{
		CString strFormat, strMessage;
		switch (theApp.m_nConnection)
		{
			case 0:
			{
				CString strFullPortName;
				strFullPortName.Format(_T("\\\\.\\%s"), static_cast<LPCWSTR>(theApp.m_strSerialName));
				m_pSerialPort.Open(
					strFullPortName,
					theApp.m_nBaudRate,
					(CSerialPort::Parity) theApp.m_nParity,
					(BYTE)theApp.m_nDataBits,
					(CSerialPort::StopBits) theApp.m_nStopBits,
					(CSerialPort::FlowControl) theApp.m_nFlowControl,
					FALSE);

				if (m_pSerialPort.IsOpen())
				{
					m_nThreadRunning = true;
					m_hSerialPortThread = CreateThread(nullptr, 0, SerialPortThreadFunc, this, 0, &m_nSerialPortThreadID);
					VERIFY(strFormat.LoadString(IDS_SERIAL_PORT_OPENED));
					strMessage.Format(strFormat, static_cast<LPCWSTR>(theApp.m_strSerialName));
					SetCaptionBarText(strMessage);
				}
				break;
			}
			case 1:
			case 2:
			{
				CString strServerIP = theApp.m_strServerIP;
				UINT nServerPort = theApp.m_nServerPort;
				CString strClientIP = theApp.m_strClientIP;
				UINT nClientPort = theApp.m_nClientPort;

				if (theApp.m_nConnection == 1) // TCP Socket
				{
					if (theApp.m_nSocketType == 1) // Client
					{
						m_pSocket.CreateAndConnect(strServerIP, nServerPort);
					}
					else // TCP Server
					{
						m_pSocket.SetBindAddress(strClientIP);
						m_pSocket.CreateAndBind(nClientPort, SOCK_STREAM, AF_INET);

						m_dlgIncoming.ShowWindow(SW_SHOW);
						m_dlgIncoming.CenterWindow(this);
						m_dlgIncoming.Invalidate();
						m_dlgIncoming.UpdateWindow();
						m_pSocket.Listen();
						m_pSocket.Accept(m_pIncomming);
						m_dlgIncoming.ShowWindow(SW_HIDE);
					}
				}
				else // UDP Socket
				{
					m_pSocket.SetBindAddress(strClientIP);
					m_pSocket.CreateAndBind(nClientPort, SOCK_DGRAM, AF_INET);

					strServerIP = strClientIP;
					nServerPort = nClientPort;
				}

				if (m_pSocket.IsCreated())
				{
					m_nThreadRunning = true;
					m_hSocketThread = CreateThread(nullptr, 0, SocketThreadFunc, this, 0, &m_nSocketTreadID);
					VERIFY(strFormat.LoadString(IDS_SOCKET_CREATED));
					strMessage.Format(strFormat, ((theApp.m_nConnection == 1) ? _T("TCP") : _T("UDP")), static_cast<LPCWSTR>(strServerIP), nServerPort);
					SetCaptionBarText(strMessage);
				}
				break;
			}
		}
	}
	catch (CSerialException& pException)
	{
		const int nErrorLength = 0x100;
		TCHAR lpszErrorMessage[nErrorLength] = { 0, };
		pException.GetErrorMessage2(lpszErrorMessage, nErrorLength);
		TRACE(_T("%s\n"), lpszErrorMessage);
		// pException->Delete();
		SetCaptionBarText(lpszErrorMessage);
		m_nThreadRunning = false;
	}
	catch (CWSocketException* pException)
	{
		const int nErrorLength = 0x100;
		TCHAR lpszErrorMessage[nErrorLength] = { 0, };
		pException->GetErrorMessage(lpszErrorMessage, nErrorLength);
		TRACE(_T("%s\n"), lpszErrorMessage);
		pException->Delete();
		SetCaptionBarText(lpszErrorMessage);
		m_nThreadRunning = false;
	}
}

void CMainFrame::OnCloseSerialPort()
{
	if (m_nThreadRunning)
	{
		m_nThreadRunning = false;
		DWORD nThreadCount = 0;
		HANDLE hThreadArray[2] = { 0, 0 };
		if (m_hSerialPortThread != nullptr)
		{
			hThreadArray[nThreadCount++] = m_hSerialPortThread;
		}
		if (m_hSocketThread != nullptr)
		{
			hThreadArray[nThreadCount++] = m_hSocketThread;
		}
		if (nThreadCount > 0)
		{
			WaitForMultipleObjects(nThreadCount, hThreadArray, TRUE, INFINITE);
		}
	}

	try
	{
		CString strFormat, strMessage;
		switch (theApp.m_nConnection)
		{
			case 0:
			{
				if (!m_pSerialPort.IsOpen())
				{
					VERIFY(strFormat.LoadString(IDS_SERIAL_PORT_CLOSED));
					strMessage.Format(strFormat, static_cast<LPCWSTR>(theApp.m_strSerialName));
					SetCaptionBarText(strMessage);
				}
				break;
			}
			case 1:
			case 2:
			{
				CString strServerIP = theApp.m_strServerIP;
				UINT nServerPort = theApp.m_nServerPort;
				CString strClientIP = theApp.m_strClientIP;
				UINT nClientPort = theApp.m_nClientPort;

				if (!m_pSocket.IsCreated())
				{
					if (theApp.m_nConnection == 1) // TCP Socket
					{
						if (theApp.m_nSocketType == 1) // Client
						{
						}
						else // TCP Server
						{
						}
					}
					else // UDP Socket
					{
						strServerIP = strClientIP;
						nServerPort = nClientPort;
					}

					VERIFY(strFormat.LoadString(IDS_SOCKET_CLOSED));
					strMessage.Format(strFormat, ((theApp.m_nConnection == 1) ? _T("TCP") : _T("UDP")), static_cast<LPCWSTR>(strServerIP), nServerPort);
					SetCaptionBarText(strMessage);
				}
				break;
			}
		}
	}
	catch (CSerialException& pException)
	{
		const int nErrorLength = 0x100;
		TCHAR lpszErrorMessage[nErrorLength] = { 0, };
		pException.GetErrorMessage2(lpszErrorMessage, nErrorLength);
		TRACE(_T("%s\n"), lpszErrorMessage);
		// pException->Delete();
		SetCaptionBarText(lpszErrorMessage);
		m_nThreadRunning = false;
	}
	catch (CWSocketException* pException)
	{
		const int nErrorLength = 0x100;
		TCHAR lpszErrorMessage[nErrorLength] = { 0, };
		pException->GetErrorMessage(lpszErrorMessage, nErrorLength);
		TRACE(_T("%s\n"), lpszErrorMessage);
		pException->Delete();
		SetCaptionBarText(lpszErrorMessage);
		m_nThreadRunning = false;
	}
}

void CMainFrame::OnSendReceive()
{
	try
	{
		CInputDlg dlgInput(this);
		if (dlgInput.DoModal() == IDOK)
		{
			const std::wstring strRawText(dlgInput.m_strSendData);
			// convert Unicode characters to UTF8
			CStringA pBuffer(wstring_to_utf8(strRawText).c_str());

			const int nLength = pBuffer.GetLength();

			switch (theApp.m_nConnection)
			{
				case 0:
				{
					m_pMutualAccess.lock();
					m_pSerialPort.Write(pBuffer.GetBufferSetLength(nLength), nLength);
					m_pMutualAccess.unlock();
					pBuffer.ReleaseBuffer();
					break;
				}
				case 1:
				case 2:
				{
					CString strServerIP = theApp.m_strServerIP;
					const UINT nServerPort = theApp.m_nServerPort;

					if (theApp.m_nConnection == 1) // TCP Socket
					{
						if (theApp.m_nSocketType == 1) // Client
						{
							if (m_pSocket.IsWritable(1000))
							{
								m_pMutualAccess.lock();
								m_pSocket.Send(pBuffer.GetBufferSetLength(nLength), nLength, 0);
								pBuffer.ReleaseBuffer();
								m_pMutualAccess.unlock();
							}
						}
						else
						{
							if (m_pIncomming.IsWritable(1000))
							{
								m_pMutualAccess.lock();
								m_pIncomming.Send(pBuffer.GetBufferSetLength(nLength), nLength, 0);
								pBuffer.ReleaseBuffer();
								m_pMutualAccess.unlock();
							}
						}
					}
					else
					{
						if (m_pSocket.IsWritable(1000))
						{
							m_pMutualAccess.lock();
							m_pSocket.SendTo(pBuffer.GetBufferSetLength(nLength), nLength, nServerPort, strServerIP, 0);
							pBuffer.ReleaseBuffer();
							m_pMutualAccess.unlock();
						}
					}
					break;
				}
			}
		}
	}
	catch (CSerialException& pException)
	{
		const int nErrorLength = 0x100;
		TCHAR lpszErrorMessage[nErrorLength] = { 0, };
		pException.GetErrorMessage2(lpszErrorMessage, nErrorLength);
		TRACE(_T("%s\n"), lpszErrorMessage);
		// pException->Delete();
		SetCaptionBarText(lpszErrorMessage);
		m_nThreadRunning = false;
	}
	catch (CWSocketException* pException)
	{
		const int nErrorLength = 0x100;
		TCHAR lpszErrorMessage[nErrorLength] = { 0, };
		pException->GetErrorMessage(lpszErrorMessage, nErrorLength);
		TRACE(_T("%s\n"), lpszErrorMessage);
		pException->Delete();
		SetCaptionBarText(lpszErrorMessage);
		m_nThreadRunning = false;
	}
}

void CMainFrame::OnUpdateConfigureSerialPort(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(!m_pSerialPort.IsOpen() && !m_pSocket.IsCreated());
}

void CMainFrame::OnUpdateOpenSerialPort(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(!m_pSerialPort.IsOpen() && !m_pSocket.IsCreated());
}

void CMainFrame::OnUpdateCloseSerialPort(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_pSerialPort.IsOpen() || m_pSocket.IsCreated());
}

void CMainFrame::OnUpdateSendReceive(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_pSerialPort.IsOpen() || m_pSocket.IsCreated());
}

DWORD WINAPI SerialPortThreadFunc(LPVOID pParam)
{
	COMSTAT status = { 0, };
	char pBuffer[0x10000] = { 0, };
	CMainFrame* pMainFrame = (CMainFrame*) pParam;
	CRingBuffer& pRingBuffer = pMainFrame->m_pRingBuffer;
	CSerialPort& pSerialPort = pMainFrame->m_pSerialPort;
	std::mutex& pMutualAccess = pMainFrame->m_pMutualAccess;

	while (pMainFrame->m_nThreadRunning)
	{
		try
		{
			memset(&status, 0, sizeof(status));
			pSerialPort.GetStatus(status);
			if (status.cbInQue > 0)
			{
				memset(pBuffer, 0, sizeof(pBuffer));
				const int nLength = pSerialPort.Read(pBuffer, sizeof(pBuffer));
				pMutualAccess.lock();
				pRingBuffer.WriteBinary(pBuffer, nLength);
				pMutualAccess.unlock();
			}
			else
			{
				::Sleep(10);
			}
		}
		catch (CSerialException& pException)
		{
			const int nErrorLength = 0x100;
			TCHAR lpszErrorMessage[nErrorLength] = { 0, };
			pException.GetErrorMessage2(lpszErrorMessage, nErrorLength);
			TRACE(_T("%s\n"), lpszErrorMessage);
			// pException->Delete();
			pMainFrame->SetCaptionBarText(lpszErrorMessage);
			break;
		}
	}
	pSerialPort.Close();
	pMainFrame->m_nThreadRunning = false;
	pMainFrame->m_hSerialPortThread = nullptr;
	return 0;
}

DWORD WINAPI SocketThreadFunc(LPVOID pParam)
{
	char pBuffer[0x10000] = { 0, };
	CMainFrame* pMainFrame = (CMainFrame*) pParam;
	CRingBuffer& pRingBuffer = pMainFrame->m_pRingBuffer;
	CWSocket& pSocket = pMainFrame->m_pSocket;
	CWSocket& pIncomming = pMainFrame->m_pIncomming;
	std::mutex& pMutualAccess = pMainFrame->m_pMutualAccess;
	bool bIsTCP = (theApp.m_nConnection == 1);
	bool bIsClient = (theApp.m_nSocketType == 1);

	CString strServerIP = theApp.m_strServerIP;
	UINT nServerPort = theApp.m_nServerPort;

	while (pMainFrame->m_nThreadRunning)
	{
		try
		{
			if (bIsTCP)
			{
				if (bIsClient)
				{
					if (pSocket.IsReadible(1000))
					{
						memset(pBuffer, 0, sizeof(pBuffer));
						const int nLength = pSocket.Receive(pBuffer, sizeof(pBuffer), 0);
						pMutualAccess.lock();
						pRingBuffer.WriteBinary(pBuffer, nLength);
						pMutualAccess.unlock();
					}
					else
					{
						::Sleep(10);
					}
				}
				else
				{
					if (pIncomming.IsReadible(1000))
					{
						memset(pBuffer, 0, sizeof(pBuffer));
						const int nLength = pIncomming.Receive(pBuffer, sizeof(pBuffer), 0);
						pMutualAccess.lock();
						pRingBuffer.WriteBinary(pBuffer, nLength);
						pMutualAccess.unlock();
					}
					else
					{
						::Sleep(10);
					}
				}
			}
			else
			{
				if (pSocket.IsReadible(1000))
				{
					memset(pBuffer, 0, sizeof(pBuffer));
					const int nLength = pSocket.ReceiveFrom(pBuffer, sizeof(pBuffer), strServerIP, nServerPort, 0);
					pMutualAccess.lock();
					pRingBuffer.WriteBinary(pBuffer, nLength);
					pMutualAccess.unlock();
				}
				else
				{
					::Sleep(10);
				}
			}
		}
		catch (CWSocketException* pException)
		{
			const int nErrorLength = 0x100;
			TCHAR lpszErrorMessage[nErrorLength] = { 0, };
			pException->GetErrorMessage(lpszErrorMessage, nErrorLength);
			TRACE(_T("%s\n"), lpszErrorMessage);
			pException->Delete();
			pMainFrame->SetCaptionBarText(lpszErrorMessage);
			break;
		}
	}
	pIncomming.Close();
	pSocket.Close();
	pMainFrame->m_nThreadRunning = false;
	pMainFrame->m_hSocketThread = nullptr;
	return 0;
}

void CMainFrame::OnTwitter()
{
	::ShellExecute(GetSafeHwnd(), _T("open"), _T("https://x.com/stefanmihaimoga"), nullptr, nullptr, SW_SHOW);
}

void CMainFrame::OnLinkedin()
{
	::ShellExecute(GetSafeHwnd(), _T("open"), _T("https://www.linkedin.com/in/stefanmihaimoga/"), nullptr, nullptr, SW_SHOW);
}

void CMainFrame::OnFacebook()
{
	::ShellExecute(GetSafeHwnd(), _T("open"), _T("https://www.facebook.com/stefanmihaimoga"), nullptr, nullptr, SW_SHOW);
}

void CMainFrame::OnInstagram()
{
	::ShellExecute(GetSafeHwnd(), _T("open"), _T("https://www.instagram.com/stefanmihaimoga/"), nullptr, nullptr, SW_SHOW);
}

void CMainFrame::OnIssues()
{
	::ShellExecute(GetSafeHwnd(), _T("open"), _T("https://github.com/mihaimoga/IntelliPort/issues"), nullptr, nullptr, SW_SHOW);
}

void CMainFrame::OnDiscussions()
{
	::ShellExecute(GetSafeHwnd(), _T("open"), _T("https://github.com/mihaimoga/IntelliPort/discussions"), nullptr, nullptr, SW_SHOW);
}

void CMainFrame::OnWiki()
{
	::ShellExecute(GetSafeHwnd(), _T("open"), _T("https://github.com/mihaimoga/IntelliPort/wiki"), nullptr, nullptr, SW_SHOW);
}

void CMainFrame::OnUserManual()
{
	CWebBrowserDlg dlgWebBrowser(this);
	dlgWebBrowser.DoModal();
}

void CMainFrame::OnCheckForUpdates()
{
	CCheckForUpdatesDlg dlgCheckForUpdates(this);
	dlgCheckForUpdates.DoModal();
}
