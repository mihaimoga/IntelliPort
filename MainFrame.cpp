/* This file is part of IntelliPort application developed by Mihai MOGA.

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
END_MESSAGE_MAP()

// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here
	theApp.m_nAppLook = theApp.GetInt(_T("ApplicationLook"), ID_VIEW_APPLOOK_OFF_2007_AQUA);
	m_pCurrentDateTime = CTime::GetCurrentTime();
	m_pRingBuffer.Create(0x10000);
	m_nThreadRunning = FALSE;
}

CMainFrame::~CMainFrame()
{
	m_nThreadRunning = FALSE;
	::Sleep(1000);
	m_pSerialPort.Close();
	m_pSocket.Close();
	m_pIncomming.Close();
	m_pRingBuffer.Destroy();
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWndEx::OnCreate(lpCreateStruct) == -1)
		return -1;

	BOOL bNameValid;

	// set the visual manager used to draw all user interface elements
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOffice2007));

	// set the visual style to be used the by the visual manager
	CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_Silver);

	m_wndRibbonBar.Create(this);
	m_wndRibbonBar.LoadFromResource(IDR_RIBBON);

	if (!m_wndStatusBar.Create(this))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	CString strTitlePane;
	bNameValid = strTitlePane.LoadString(IDS_STATUS_PANE1);
	ASSERT(bNameValid);
	m_wndStatusBar.AddElement(new CMFCRibbonStatusBarPane(
		ID_STATUSBAR_PANE1, strTitlePane, TRUE, NULL,
		_T("012345678901234567890123456789012345678901234567890123456789")), strTitlePane);
	strTitlePane.LoadString(IDS_LOG_HISTORY_CLEARED);
	SetStatusBarText(strTitlePane);

	// enable Visual Studio 2005 style docking window behavior
	CDockingManager::SetDockingMode(DT_SMART);
	// enable Visual Studio 2005 style docking window auto-hide behavior
	EnableAutoHidePanes(CBRS_ALIGN_ANY);

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

BOOL CMainFrame::CreateCaptionBar()
{
	if (!m_wndCaptionBar.Create(WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, this, ID_VIEW_CAPTION_BAR, -1, TRUE))
	{
		TRACE0("Failed to create caption bar\n");
		return FALSE;
	}

	BOOL bNameValid;
	CString strTemp, strTemp2;
	/*bNameValid = strTemp.LoadString(IDS_CAPTION_BUTTON);
	ASSERT(bNameValid);
	m_wndCaptionBar.SetButton(strTemp, ID_TOOLS_OPTIONS, CMFCCaptionBar::ALIGN_LEFT, FALSE);
	bNameValid = strTemp.LoadString(IDS_CAPTION_BUTTON_TIP);
	ASSERT(bNameValid);
	m_wndCaptionBar.SetButtonToolTip(strTemp);

	bNameValid = strTemp.LoadString(IDS_CAPTION_TEXT);
	ASSERT(bNameValid);
	m_wndCaptionBar.SetText(strTemp, CMFCCaptionBar::ALIGN_LEFT);*/

	m_wndCaptionBar.SetBitmap(IDB_INFO, RGB(255, 255, 255), FALSE, CMFCCaptionBar::ALIGN_LEFT);
	bNameValid = strTemp.LoadString(IDS_CAPTION_IMAGE_TIP);
	ASSERT(bNameValid);
	bNameValid = strTemp2.LoadString(IDS_CAPTION_IMAGE_TEXT);
	ASSERT(bNameValid);
	m_wndCaptionBar.SetImageToolTip(strTemp, strTemp2);

	HideMessageBar();
	return TRUE;
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
			CStringA strText(pBuffer);
			AddText(CString(strText));
		}
		m_pMutualAccess.unlock();
	}

	CFrameWndEx::OnTimer(nIDEvent);
}

BOOL CMainFrame::SetStatusBarText(CString strMessage)
{
	if (m_wndStatusBar.GetSafeHwnd() != NULL)
	{
		m_wndStatusBar.GetElement(0)->SetText(strMessage);
		m_wndStatusBar.Invalidate();
		m_wndStatusBar.UpdateWindow();
		return TRUE;
	}
	return FALSE;
}

BOOL CMainFrame::SetCaptionBarText(CString strMessage)
{
	if (m_wndCaptionBar.GetSafeHwnd() != NULL)
	{
		m_pCurrentDateTime = CTime::GetCurrentTime();
		m_wndCaptionBar.ShowWindow(SW_SHOW);
		m_wndCaptionBar.SetText(strMessage, CMFCCaptionBar::ALIGN_LEFT);
		m_wndCaptionBar.Invalidate();
		m_wndCaptionBar.UpdateWindow();
		RecalcLayout();
	}
	return FALSE;
}

BOOL CMainFrame::HideMessageBar()
{
	if (m_wndCaptionBar.GetSafeHwnd() != NULL)
	{
		m_wndCaptionBar.ShowWindow(SW_HIDE);
		m_wndCaptionBar.Invalidate();
		m_wndCaptionBar.UpdateWindow();
		RecalcLayout();
	}
	return TRUE;
}

#define CRLF _T("\r\n")
#define CR _T("\r")
#define LF _T("\n")

BOOL CMainFrame::AddText(CString strText)
{
	strText.Replace(CRLF, LF);
	strText.Replace(CR, LF);
	strText.Replace(LF, CRLF);
	CEdit& pEdit = reinterpret_cast<CEditView*>(GetActiveView())->GetEditCtrl();
	int outLength = pEdit.GetWindowTextLength();
	pEdit.SetSel(outLength, outLength);
	pEdit.ReplaceSel(strText, TRUE);
	pEdit.SetSel(-1, 0);

	return FALSE;
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
	CString strFormat, strMessage;
	switch (theApp.GetInt(_T("Connection"), 0))
	{
		case 0:
		{
			CString strFullPortName;
			strFullPortName.Format(_T("\\\\.\\%s"), theApp.GetString(_T("SerialName"), _T("COM1")));
			m_pSerialPort.Open(
				strFullPortName,
				theApp.GetInt(_T("BaudRate"), CBR_115200),
				(CSerialPort::Parity) theApp.GetInt(_T("Parity"), (int)CSerialPort::Parity::NoParity),
				(BYTE) theApp.GetInt(_T("DataBits"), 8),
				(CSerialPort::StopBits) theApp.GetInt(_T("StopBits"), (int)CSerialPort::StopBits::OneStopBit),
				(CSerialPort::FlowControl) theApp.GetInt(_T("FlowControl"), (int)CSerialPort::FlowControl::NoFlowControl),
				FALSE);

			if (m_pSerialPort.IsOpen())
			{
				m_nThreadRunning = TRUE;
				AfxBeginThread(SerialPortThreadFunc, this);
				strFormat.LoadString(IDS_SERIAL_PORT_OPENED);
				strMessage.Format(strFormat, theApp.GetString(_T("SerialName"), _T("COM1")));
				SetCaptionBarText(strMessage);
			}
			break;
		}
		case 1:
		case 2:
		{
			CString strServerIP = theApp.GetString(_T("ServerIP"), _T("127.0.0.1"));
			if (strServerIP.IsEmpty()) strServerIP = _T("127.0.0.1");
			UINT nServerPort = theApp.GetInt(_T("ServerPort"), 0);
			CString strClientIP = theApp.GetString(_T("ClientIP"), _T("127.0.0.1"));
			if (strClientIP.IsEmpty()) strClientIP = _T("127.0.0.1");
			UINT nClientPort = theApp.GetInt(_T("ClientPort"), 0);

			m_pSocket.SetBindAddress(strClientIP);
			m_pSocket.CreateAndBind(nClientPort, ((theApp.GetInt(_T("Connection"), 0) == 1) ? SOCK_STREAM : SOCK_DGRAM), AF_INET);

			if (m_pSocket.IsCreated())
			{
				if (theApp.GetInt(_T("Connection"), 0) == 1) // TCP Socket
				{
					if (theApp.GetInt(_T("SocketType"), 0) == 1) // Client
					{
						m_pSocket.CreateAndConnect(strServerIP, nServerPort);
					}
					else // TCP Server
					{
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
					strServerIP = strClientIP;
					nServerPort = nClientPort;
				}

				m_nThreadRunning = TRUE;
				AfxBeginThread(SocketThreadFunc, this);
				strFormat.LoadString(IDS_SOCKET_CREATED);
				strMessage.Format(strFormat, ((theApp.GetInt(_T("Connection"), 0) == 1) ? _T("TCP") : _T("UDP")), strServerIP, nServerPort);
				SetCaptionBarText(strMessage);
			}
			break;
		}
	}
}

void CMainFrame::OnCloseSerialPort()
{
	CString strFormat, strMessage;
	m_nThreadRunning = FALSE;
	::Sleep(1000);
	switch (theApp.GetInt(_T("Connection"), 0))
	{
		case 0:
		{
			m_pSerialPort.Close();

			if (!m_pSerialPort.IsOpen())
			{
				strFormat.LoadString(IDS_SERIAL_PORT_CLOSED);
				strMessage.Format(strFormat, theApp.GetString(_T("SerialName"), _T("COM1")));
				SetCaptionBarText(strMessage);
			}
			break;
		}
		case 1:
		case 2:
		{
			CString strServerIP = theApp.GetString(_T("ServerIP"), _T("127.0.0.1"));
			if (strServerIP.IsEmpty()) strServerIP = _T("127.0.0.1");
			UINT nServerPort = theApp.GetInt(_T("ServerPort"), 0);
			CString strClientIP = theApp.GetString(_T("ClientIP"), _T("127.0.0.1"));
			if (strClientIP.IsEmpty()) strClientIP = _T("127.0.0.1");
			UINT nClientPort = theApp.GetInt(_T("ClientPort"), 0);

			m_pSocket.Close();
			m_pIncomming.Close();

			if (!m_pSocket.IsCreated())
			{
				if (theApp.GetInt(_T("Connection"), 0) == 1) // TCP Socket
				{
					if (theApp.GetInt(_T("SocketType"), 0) == 1) // Client
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

				strFormat.LoadString(IDS_SOCKET_CLOSED);
				strMessage.Format(strFormat, ((theApp.GetInt(_T("Connection"), 0) == 1) ? _T("TCP") : _T("UDP")), strServerIP, nServerPort);
				SetCaptionBarText(strMessage);
			}
			break;
		}
	}
}

void CMainFrame::OnSendReceive()
{
	CInputDlg dlgInput(this);
	if (dlgInput.DoModal() == IDOK)
	{
		CStringA pBuffer(dlgInput.m_strSendData);
		const int nLength = pBuffer.GetLength();

		switch (theApp.GetInt(_T("Connection"), 0))
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
				CString strServerIP = theApp.GetString(_T("ServerIP"), _T("127.0.0.1"));
				if (strServerIP.IsEmpty()) strServerIP = _T("127.0.0.1");
				const UINT nServerPort = theApp.GetInt(_T("ServerPort"), 0);

				if (theApp.GetInt(_T("Connection"), 0) == 1) // TCP Socket
				{
					if (theApp.GetInt(_T("SocketType"), 0) == 1) // Client
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

UINT SerialPortThreadFunc(LPVOID pParam)
{
	COMSTAT status = { 0, };
	char pBuffer[0x10000] = { 0, };
	CMainFrame* pMainFrame = (CMainFrame*) pParam;
	CRingBuffer& pRingBuffer = pMainFrame->m_pRingBuffer;
	CSerialPort& pSerialPort = pMainFrame->m_pSerialPort;
	std::mutex& pMutualAccess = pMainFrame->m_pMutualAccess;

	while (pMainFrame->m_nThreadRunning)
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
	return 0;
}

UINT SocketThreadFunc(LPVOID pParam)
{
	char pBuffer[0x10000] = { 0, };
	CMainFrame* pMainFrame = (CMainFrame*) pParam;
	CRingBuffer& pRingBuffer = pMainFrame->m_pRingBuffer;
	CWSocket& pSocket = pMainFrame->m_pSocket;
	CWSocket& pIncomming = pMainFrame->m_pIncomming;
	std::mutex& pMutualAccess = pMainFrame->m_pMutualAccess;
	BOOL bIsTCP = (theApp.GetInt(_T("Connection"), 0) == 1);
	BOOL bIsClient = (theApp.GetInt(_T("SocketType"), 0) == 1);

	CString strServerIP = theApp.GetString(_T("ServerIP"), _T("127.0.0.1"));
	if (strServerIP.IsEmpty()) strServerIP = _T("127.0.0.1");
	UINT nServerPort = theApp.GetInt(_T("ServerPort"), 0);

	while (pMainFrame->m_nThreadRunning)
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
	return 0;
}
