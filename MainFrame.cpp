/* Copyright (C) 2014-2026 Stefan-Mihai MOGA
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

/**
 * Constructor: Initializes the main frame window and all member variables.
 * Sets up application look, ring buffer, threading variables, and default connection parameters.
 */
CMainFrame::CMainFrame()
{
	// Load application visual style from settings
	theApp.m_nAppLook = theApp.GetInt(_T("ApplicationLook"), ID_VIEW_APPLOOK_OFF_2007_AQUA);
	
	// Initialize time tracking for caption bar auto-hide
	m_pCurrentDateTime = CTime::GetCurrentTime();
	
	// Create ring buffer for data communication (64KB)
	m_pRingBuffer.Create(0x10000);
	
	// Initialize threading variables
	m_nThreadRunning = false;
	m_hSerialPortThread = nullptr;
	m_hSocketThread = nullptr;
	m_nSerialPortThreadID = 0;
	m_nSocketTreadID = 0;
	m_nTimerID = 0;

	// Initialize serial port configuration to invalid state
	theApp.m_nBaudRate = -1;
	theApp.m_nDataBits = -1;
	theApp.m_nParity = -1;
	theApp.m_nStopBits = -1;
	theApp.m_nFlowControl = -1;
	
	// Initialize socket configuration
	theApp.m_nSocketType = -1;
	theApp.m_strServerIP = _T("127.0.0.1");
	theApp.m_nServerPort = 8080;
	theApp.m_strClientIP = _T("127.0.0.1");
	theApp.m_nClientPort = 8080;
}

/**
 * Destructor: Cleans up resources including serial port and ring buffer.
 */
CMainFrame::~CMainFrame()
{
	// Close any open serial port or socket connection
	OnCloseSerialPort();
	// Destroy the ring buffer
	m_pRingBuffer.Destroy();
}

/**
 * Called when the main frame window is being created.
 * Initializes the ribbon bar, status bar, caption bar, and timer.
 * @param lpCreateStruct Pointer to window creation parameters
 * @return 0 on success, -1 on failure
 */
int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWndEx::OnCreate(lpCreateStruct) == -1)
		return -1;

	// Enable Visual Studio 2005 style docking window behavior
	CDockingManager::SetDockingMode(DT_SMART);
	// Enable Visual Studio 2005 style docking window auto-hide behavior
	EnableAutoHidePanes(CBRS_ALIGN_ANY);

	// Set the visual manager used to draw all user interface elements
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

	// Create and initialize the ribbon bar
	m_wndRibbonBar.Create(this);
	m_wndRibbonBar.LoadFromResource(IDR_RIBBON);

	// Create the status bar
	if (!m_wndStatusBar.Create(this))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	// Configure status bar with a single pane
	bool bNameValid;
	CString strTitlePane;
	bNameValid = strTitlePane.LoadString(IDS_STATUS_PANE1);
	ASSERT(bNameValid);
	m_wndStatusBar.AddElement(new CMFCRibbonStatusBarPane(
		ID_STATUSBAR_PANE1, strTitlePane, TRUE, NULL,
		_T("012345678901234567890123456789012345678901234567890123456789")), strTitlePane);
	
	// Set initial status bar text
	VERIFY(strTitlePane.LoadString(IDS_LOG_HISTORY_CLEARED));
	SetStatusBarText(strTitlePane);

	// Create the caption bar for displaying notifications
	if (!CreateCaptionBar())
	{
		TRACE0("Failed to create caption bar\n");
		return -1;      // fail to create
	}

	// Create the incoming connection dialog (for TCP server mode)
	VERIFY(m_dlgIncoming.Create(CIncomingDlg::IDD, this));

	// Start timer for periodic ring buffer checking (every 10ms)
	m_nTimerID = SetTimer(1, 10, NULL);

	return 0;
}

/**
 * Called before the window is created. Allows modification of window creation parameters.
 * @param cs Reference to the CREATESTRUCT structure containing window creation parameters
 * @return TRUE if the window should be created, FALSE otherwise
 */
BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWndEx::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return TRUE;
}

/**
 * Creates and configures the caption bar for displaying notifications.
 * Sets up the caption bar with an info icon and tooltip.
 * @return true on success, false on failure
 */
bool CMainFrame::CreateCaptionBar()
{
	// Create the caption bar window
	if (!m_wndCaptionBar.Create(WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, this, ID_VIEW_CAPTION_BAR, -1, TRUE))
	{
		TRACE0("Failed to create caption bar\n");
		return false;
	}

	bool bNameValid;
	CString strTemp, strTemp2;
	
	// Set the info icon on the caption bar
	m_wndCaptionBar.SetBitmap(IDB_INFO, RGB(255, 255, 255), FALSE, CMFCCaptionBar::ALIGN_LEFT);
	
	// Load and set the tooltip text
	bNameValid = strTemp.LoadString(IDS_CAPTION_IMAGE_TIP);
	ASSERT(bNameValid);
	bNameValid = strTemp2.LoadString(IDS_CAPTION_IMAGE_TEXT);
	ASSERT(bNameValid);
	m_wndCaptionBar.SetImageToolTip(strTemp, strTemp2);

	// Initially hide the message bar
	HideMessageBar();
	return true;
}

/**
 * Called when the window is being destroyed.
 * Cleans up the timer and performs final cleanup.
 */
void CMainFrame::OnDestroy()
{
	// Kill the timer
	VERIFY(KillTimer(m_nTimerID));

	CFrameWndEx::OnDestroy();
}

/**
 * Timer callback function that periodically checks the ring buffer for incoming data
 * and handles caption bar auto-hide after 10 seconds.
 * @param nIDEvent Timer identifier
 */
void CMainFrame::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == m_nTimerID)
	{
		// Check if caption bar should be hidden (after 10 seconds)
		CTime pDateTime = CTime::GetCurrentTime();
		CTimeSpan pTimeSpan = pDateTime - m_pCurrentDateTime;
		if ((pTimeSpan.GetTotalSeconds() >= 10) && (m_wndCaptionBar.IsPaneVisible()))
		{
			HideMessageBar();
		}

		// Read data from ring buffer and display in the view
		char pBuffer[0x1000] = { 0, };
		m_pMutualAccess.lock();
		const int nLength = m_pRingBuffer.GetMaxReadSize();
		if (nLength > 0)
		{
			// Read binary data from ring buffer
			m_pRingBuffer.ReadBinary(pBuffer, nLength);
			pBuffer[nLength] = '\0';
			
			// Convert UTF-8 to wide string and display
			const std::string strRawText(pBuffer);
			CString strBuffer(utf8_to_wstring(strRawText).c_str());
			AddText(strBuffer);
		}
		m_pMutualAccess.unlock();
	}

	CFrameWndEx::OnTimer(nIDEvent);
}

/**
 * Sets the text displayed in the status bar.
 * @param strMessage The message to display in the status bar
 * @return true if successful, false otherwise
 */
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

/**
 * Sets the text displayed in the caption bar and shows it.
 * The caption bar will auto-hide after 10 seconds.
 * @param strMessage The message to display in the caption bar
 * @return true if successful, false otherwise
 */
bool CMainFrame::SetCaptionBarText(const CString& strMessage)
{
	if (m_wndCaptionBar.GetSafeHwnd() != nullptr)
	{
		// Update timestamp for auto-hide timer
		m_pCurrentDateTime = CTime::GetCurrentTime();
		
		// Show and update the caption bar
		m_wndCaptionBar.ShowWindow(SW_SHOW);
		m_wndCaptionBar.SetText(strMessage, CMFCCaptionBar::ALIGN_LEFT);
		m_wndCaptionBar.Invalidate();
		m_wndCaptionBar.UpdateWindow();
		RecalcLayout();
	}
	return false;
}

/**
 * Hides the caption bar.
 * @return true if successful
 */
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

/**
 * Adds text to the active view's edit control.
 * Normalizes line endings to CRLF format before adding.
 * @param strText The text to add to the view
 * @return true if successful
 */
bool CMainFrame::AddText(CString strText)
{
	// Normalize line endings to CRLF
	strText.Replace(CRLF, LF);   // First convert CRLF to LF
	strText.Replace(CR, LF);     // Then convert standalone CR to LF
	strText.Replace(LF, CRLF);   // Finally convert all LF to CRLF
	
	// Append text to the end of the edit control
	CEdit& pEdit = reinterpret_cast<CEditView*>(GetActiveView())->GetEditCtrl();
	int outLength = pEdit.GetWindowTextLength();
	pEdit.SetSel(outLength, outLength); // Move cursor to end
	pEdit.ReplaceSel(strText, TRUE);    // Insert new text
	pEdit.SetSel(-1, 0);                // Reset selection

	return true;
}

// CMainFrame diagnostics

#ifdef _DEBUG
/**
 * Validates the main frame object in debug builds.
 */
void CMainFrame::AssertValid() const
{
	CFrameWndEx::AssertValid();
}

/**
 * Dumps diagnostic information about the main frame to the dump context.
 * @param dc Reference to the CDumpContext for outputting diagnostic information
 */
void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWndEx::Dump(dc);
}
#endif //_DEBUG

// CMainFrame message handlers

/**
 * Toggles the visibility of the caption bar.
 */
void CMainFrame::OnViewCaptionBar()
{
	m_wndCaptionBar.ShowWindow(m_wndCaptionBar.IsVisible() ? SW_HIDE : SW_SHOW);
	RecalcLayout(FALSE);
}

/**
 * Updates the UI state for the caption bar view menu item.
 * @param pCmdUI Pointer to command UI object
 */
void CMainFrame::OnUpdateViewCaptionBar(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_wndCaptionBar.IsVisible());
}

/**
 * Displays the ribbon customization dialog.
 */
void CMainFrame::OnOptions()
{
	CMFCRibbonCustomizeDialog *pOptionsDlg = new CMFCRibbonCustomizeDialog(this, &m_wndRibbonBar);
	ASSERT(pOptionsDlg != NULL);

	pOptionsDlg->DoModal();
	delete pOptionsDlg;
}

/**
 * Handles the File Print command.
 * If in print preview mode, sends print command to preview.
 */
void CMainFrame::OnFilePrint()
{
	if (IsPrintPreview())
	{
		PostMessage(WM_COMMAND, AFX_ID_PREVIEW_PRINT);
	}
}

/**
 * Handles the File Print Preview command.
 * Closes print preview if already in preview mode.
 */
void CMainFrame::OnFilePrintPreview()
{
	if (IsPrintPreview())
	{
		PostMessage(WM_COMMAND, AFX_ID_PREVIEW_CLOSE);  // force Print Preview mode closed
	}
}

/**
 * Updates the UI state for the print preview menu item.
 * @param pCmdUI Pointer to command UI object
 */
void CMainFrame::OnUpdateFilePrintPreview(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(IsPrintPreview());
}

/**
 * Displays the serial port/socket configuration dialog.
 * Allows user to configure connection parameters.
 */
void CMainFrame::OnConfigureSerialPort()
{
	CConfigureDlg dlgConfigure(this);
	if (dlgConfigure.DoModal() == IDOK)
	{
		// Configuration is saved within the dialog
	}
}

/**
 * Opens a connection based on the configured settings.
 * Supports serial port, TCP socket (client/server), and UDP socket connections.
 * Creates a background thread to handle data reception.
 */
void CMainFrame::OnOpenSerialPort()
{
	try
	{
		CString strFormat, strMessage;
		switch (theApp.m_nConnection)
		{
			case 0: // Serial Port Connection
			{
				// Format the full port name (e.g., "\\.\COM1")
				CString strFullPortName;
				strFullPortName.Format(_T("\\\\.\\%s"), static_cast<LPCWSTR>(theApp.m_strSerialName));
				
				// Open the serial port with configured parameters
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
					// Start the serial port reading thread
					m_nThreadRunning = true;
					m_hSerialPortThread = CreateThread(nullptr, 0, SerialPortThreadFunc, this, 0, &m_nSerialPortThreadID);
					
					// Display success message
					VERIFY(strFormat.LoadString(IDS_SERIAL_PORT_OPENED));
					strMessage.Format(strFormat, static_cast<LPCWSTR>(theApp.m_strSerialName));
					SetCaptionBarText(strMessage);
				}
				break;
			}
			case 1: // TCP Socket Connection
			case 2: // UDP Socket Connection
			{
				CString strServerIP = theApp.m_strServerIP;
				UINT nServerPort = theApp.m_nServerPort;
				CString strClientIP = theApp.m_strClientIP;
				UINT nClientPort = theApp.m_nClientPort;

				if (theApp.m_nConnection == 1) // TCP Socket
				{
					if (theApp.m_nSocketType == 1) // TCP Client
					{
						// Connect to TCP server
						m_pSocket.CreateAndConnect(strServerIP, nServerPort);
					}
					else // TCP Server
					{
						// Create TCP server and wait for incoming connection
						m_pSocket.SetBindAddress(strClientIP);
						m_pSocket.CreateAndBind(nClientPort, SOCK_STREAM, AF_INET);

						// Show waiting dialog
						m_dlgIncoming.ShowWindow(SW_SHOW);
						m_dlgIncoming.CenterWindow(this);
						m_dlgIncoming.Invalidate();
						m_dlgIncoming.UpdateWindow();
						
						// Listen and accept incoming connection
						m_pSocket.Listen();
						m_pSocket.Accept(m_pIncomming);
						m_dlgIncoming.ShowWindow(SW_HIDE);
					}
				}
				else // UDP Socket
				{
					// Create UDP socket
					m_pSocket.SetBindAddress(strClientIP);
					m_pSocket.CreateAndBind(nClientPort, SOCK_DGRAM, AF_INET);

					strServerIP = strClientIP;
					nServerPort = nClientPort;
				}

				if (m_pSocket.IsCreated())
				{
					// Start the socket reading thread
					m_nThreadRunning = true;
					m_hSocketThread = CreateThread(nullptr, 0, SocketThreadFunc, this, 0, &m_nSocketTreadID);
					
					// Display success message
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
		// Handle serial port errors
		const int nErrorLength = 0x100;
		TCHAR lpszErrorMessage[nErrorLength] = { 0, };
		pException.GetErrorMessage2(lpszErrorMessage, nErrorLength);
		TRACE(_T("%s\n"), lpszErrorMessage);
		SetCaptionBarText(lpszErrorMessage);
		m_nThreadRunning = false;
	}
	catch (CWSocketException* pException)
	{
		// Handle socket errors
		const int nErrorLength = 0x100;
		TCHAR lpszErrorMessage[nErrorLength] = { 0, };
		pException->GetErrorMessage(lpszErrorMessage, nErrorLength);
		TRACE(_T("%s\n"), lpszErrorMessage);
		pException->Delete();
		SetCaptionBarText(lpszErrorMessage);
		m_nThreadRunning = false;
	}
}

/**
 * Closes the active connection (serial port or socket).
 * Stops the background thread and releases resources.
 */
void CMainFrame::OnCloseSerialPort()
{
	// Stop the reading thread if running
	if (m_nThreadRunning)
	{
		m_nThreadRunning = false;
		DWORD nThreadCount = 0;
		HANDLE hThreadArray[2] = { 0, 0 };
		
		// Collect active thread handles
		if (m_hSerialPortThread != nullptr)
		{
			hThreadArray[nThreadCount++] = m_hSerialPortThread;
		}
		if (m_hSocketThread != nullptr)
		{
			hThreadArray[nThreadCount++] = m_hSocketThread;
		}
		
		// Wait for all threads to terminate
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
			case 0: // Serial Port
			{
				if (!m_pSerialPort.IsOpen())
				{
					VERIFY(strFormat.LoadString(IDS_SERIAL_PORT_CLOSED));
					strMessage.Format(strFormat, static_cast<LPCWSTR>(theApp.m_strSerialName));
					SetCaptionBarText(strMessage);
				}
				break;
			}
			case 1: // TCP Socket
			case 2: // UDP Socket
			{
				CString strServerIP = theApp.m_strServerIP;
				UINT nServerPort = theApp.m_nServerPort;
				CString strClientIP = theApp.m_strClientIP;
				UINT nClientPort = theApp.m_nClientPort;

				if (!m_pSocket.IsCreated())
				{
					if (theApp.m_nConnection == 2) // UDP Socket
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

/**
 * Displays the input dialog and sends data through the active connection.
 * Supports sending data through serial port, TCP socket, or UDP socket.
 */
void CMainFrame::OnSendReceive()
{
	int nLength = 0;
	CInputDlg dlgInput(this);
	if (dlgInput.DoModal() == IDOK)
	{
		// Convert Unicode to UTF-8 for transmission
		const std::wstring strRawText(dlgInput.m_strSendData);
		CStringA pBuffer(wstring_to_utf8(strRawText).c_str());
		nLength = pBuffer.GetLength();
		
		if (nLength > 0)
		{
			switch (theApp.m_nConnection)
			{
				case 0: // Serial Port
				{
					m_pMutualAccess.lock();
					try
					{
						// Write data to serial port
						m_pSerialPort.Write(pBuffer.GetBuffer(nLength), nLength);
						pBuffer.ReleaseBuffer();
					}
					catch (CSerialException& pException)
					{
						const int nErrorLength = 0x100;
						TCHAR lpszErrorMessage[nErrorLength] = { 0, };
						pException.GetErrorMessage2(lpszErrorMessage, nErrorLength);
						TRACE(_T("%s\n"), lpszErrorMessage);
						SetCaptionBarText(lpszErrorMessage);
						m_nThreadRunning = false;
						MessageBeep(MB_ICONERROR);
					}
					m_pMutualAccess.unlock();
					MessageBeep(MB_OK);
					break;
				}
				case 1: // TCP Socket
				case 2: // UDP Socket
				{
					CString strServerIP = theApp.m_strServerIP;
					const UINT nServerPort = theApp.m_nServerPort;

					if (theApp.m_nConnection == 1) // TCP Socket
					{
						if (theApp.m_nSocketType == 1) // TCP Client
						{
							if (m_pSocket.IsWritable(1000))
							{
								m_pMutualAccess.lock();
								try
								{
									m_pSocket.Send(pBuffer.GetBuffer(nLength), nLength, 0);
									pBuffer.ReleaseBuffer();
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
									MessageBeep(MB_ICONERROR);
								}
								m_pMutualAccess.unlock();
								MessageBeep(MB_OK);
							}
						}
						else // TCP Server
						{
							if (m_pIncomming.IsWritable(1000))
							{
								m_pMutualAccess.lock();
								try
								{
									m_pIncomming.Send(pBuffer.GetBuffer(nLength), nLength, 0);
									pBuffer.ReleaseBuffer();
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
									MessageBeep(MB_ICONERROR);
								}
								m_pMutualAccess.unlock();
								MessageBeep(MB_OK);
							}
						}
					}
					else // UDP Socket
					{
						if (m_pSocket.IsWritable(1000))
						{
							m_pMutualAccess.lock();
							try
							{
								m_pSocket.SendTo(pBuffer.GetBuffer(nLength), nLength, nServerPort, strServerIP, 0);
								pBuffer.ReleaseBuffer();
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
								MessageBeep(MB_ICONERROR);
							}
							m_pMutualAccess.unlock();
							MessageBeep(MB_OK);
						}
					}
					break;
				}
			}
		}
	}
}

/**
 * Updates the UI state for the configure serial port command.
 * Enabled only when no connection is active.
 * @param pCmdUI Pointer to command UI object
 */
void CMainFrame::OnUpdateConfigureSerialPort(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(!m_pSerialPort.IsOpen() && !m_pSocket.IsCreated());
}

/**
 * Updates the UI state for the open serial port command.
 * Enabled only when no connection is active.
 * @param pCmdUI Pointer to command UI object
 */
void CMainFrame::OnUpdateOpenSerialPort(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(!m_pSerialPort.IsOpen() && !m_pSocket.IsCreated());
}

/**
 * Updates the UI state for the close serial port command.
 * Enabled only when a connection is active.
 * @param pCmdUI Pointer to command UI object
 */
void CMainFrame::OnUpdateCloseSerialPort(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_pSerialPort.IsOpen() || m_pSocket.IsCreated());
}

/**
 * Updates the UI state for the send/receive command.
 * Enabled only when a connection is active.
 * @param pCmdUI Pointer to command UI object
 */
void CMainFrame::OnUpdateSendReceive(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_pSerialPort.IsOpen() || m_pSocket.IsCreated());
}

/**
 * Background thread function for reading data from the serial port.
 * Continuously reads data and writes it to the ring buffer.
 * @param pParam Pointer to the CMainFrame instance
 * @return Thread exit code (always 0)
 */
DWORD WINAPI SerialPortThreadFunc(LPVOID pParam)
{
	int nLength = 0;
	COMSTAT status = { 0, };
	char pBuffer[0x1000] = { 0, };
	CMainFrame* pMainFrame = (CMainFrame*) pParam;
	CRingBuffer& pRingBuffer = pMainFrame->m_pRingBuffer;
	CSerialPort& pSerialPort = pMainFrame->m_pSerialPort;
	std::mutex& pMutualAccess = pMainFrame->m_pMutualAccess;

	while (pMainFrame->m_nThreadRunning)
	{
		try
		{
			// Check if there's data in the serial port buffer
			memset(&status, 0, sizeof(status));
			pSerialPort.GetStatus(status);
			if (status.cbInQue > 0)
			{
				// Read available data
				memset(pBuffer, 0, sizeof(pBuffer));
				nLength = pSerialPort.Read(pBuffer, sizeof(pBuffer));
			}
		}
		catch (CSerialException& pException)
		{
			const int nErrorLength = 0x100;
			TCHAR lpszErrorMessage[nErrorLength] = { 0, };
			pException.GetErrorMessage2(lpszErrorMessage, nErrorLength);
			TRACE(_T("%s\n"), lpszErrorMessage);
			pMainFrame->SetCaptionBarText(lpszErrorMessage);
			MessageBeep(MB_ICONERROR);
			break;
		}
		
		// Write data to ring buffer if any was read
		if (nLength > 0)
		{
			pMutualAccess.lock();
			pRingBuffer.WriteBinary(pBuffer, nLength);
			pMutualAccess.unlock();
			nLength = 0;
		}
	}
	
	// Clean up before thread exit
	pSerialPort.Close();
	pMainFrame->m_nThreadRunning = false;
	pMainFrame->m_hSerialPortThread = nullptr;
	return 0;
}

/**
 * Background thread function for reading data from the socket.
 * Continuously reads data and writes it to the ring buffer.
 * Supports both TCP and UDP sockets.
 * @param pParam Pointer to the CMainFrame instance
 * @return Thread exit code (always 0)
 */
DWORD WINAPI SocketThreadFunc(LPVOID pParam)
{
	int nLength = 0;
	char pBuffer[0x1000] = { 0, };
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
					// Read from TCP client socket
					if (pSocket.IsReadible(1000))
					{
						memset(pBuffer, 0, sizeof(pBuffer));
						nLength = pSocket.Receive(pBuffer, sizeof(pBuffer), 0);
					}
				}
				else
				{
					// Read from TCP server incoming connection
					if (pIncomming.IsReadible(1000))
					{
						memset(pBuffer, 0, sizeof(pBuffer));
						nLength = pIncomming.Receive(pBuffer, sizeof(pBuffer), 0);
					}
				}
			}
			else
			{
				// Read from UDP socket
				if (pSocket.IsReadible(1000))
				{
					memset(pBuffer, 0, sizeof(pBuffer));
					nLength = pSocket.ReceiveFrom(pBuffer, sizeof(pBuffer), strServerIP, nServerPort, 0);
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
			MessageBeep(MB_ICONERROR);
			break;
		}
		
		// Write data to ring buffer if any was received
		if (nLength > 0)
		{
			pMutualAccess.lock();
			pRingBuffer.WriteBinary(pBuffer, nLength);
			pMutualAccess.unlock();
			nLength = 0;
		}
	}
	
	// Clean up before thread exit
	pIncomming.Close();
	pSocket.Close();
	pMainFrame->m_nThreadRunning = false;
	pMainFrame->m_hSocketThread = nullptr;
	return 0;
}

/**
 * Opens the developer's Twitter profile in the default browser.
 */
void CMainFrame::OnTwitter()
{
	::ShellExecute(GetSafeHwnd(), _T("open"), _T("https://x.com/stefanmihaimoga"), nullptr, nullptr, SW_SHOW);
}

/**
 * Opens the developer's LinkedIn profile in the default browser.
 */
void CMainFrame::OnLinkedin()
{
	::ShellExecute(GetSafeHwnd(), _T("open"), _T("https://www.linkedin.com/in/stefanmihaimoga/"), nullptr, nullptr, SW_SHOW);
}

/**
 * Opens the developer's Facebook profile in the default browser.
 */
void CMainFrame::OnFacebook()
{
	::ShellExecute(GetSafeHwnd(), _T("open"), _T("https://www.facebook.com/stefanmihaimoga"), nullptr, nullptr, SW_SHOW);
}

/**
 * Opens the developer's Instagram profile in the default browser.
 */
void CMainFrame::OnInstagram()
{
	::ShellExecute(GetSafeHwnd(), _T("open"), _T("https://www.instagram.com/stefanmihaimoga/"), nullptr, nullptr, SW_SHOW);
}

/**
 * Opens the GitHub Issues page in the default browser.
 */
void CMainFrame::OnIssues()
{
	::ShellExecute(GetSafeHwnd(), _T("open"), _T("https://github.com/mihaimoga/IntelliPort/issues"), nullptr, nullptr, SW_SHOW);
}

/**
 * Opens the GitHub Discussions page in the default browser.
 */
void CMainFrame::OnDiscussions()
{
	::ShellExecute(GetSafeHwnd(), _T("open"), _T("https://github.com/mihaimoga/IntelliPort/discussions"), nullptr, nullptr, SW_SHOW);
}

/**
 * Opens the GitHub Wiki page in the default browser.
 */
void CMainFrame::OnWiki()
{
	::ShellExecute(GetSafeHwnd(), _T("open"), _T("https://github.com/mihaimoga/IntelliPort/wiki"), nullptr, nullptr, SW_SHOW);
}

/**
 * Displays the user manual in a web browser dialog.
 */
void CMainFrame::OnUserManual()
{
	CWebBrowserDlg dlgWebBrowser(this);
	dlgWebBrowser.DoModal();
}

/**
 * Displays the Check for Updates dialog.
 */
void CMainFrame::OnCheckForUpdates()
{
	CCheckForUpdatesDlg dlgCheckForUpdates(this);
	dlgCheckForUpdates.DoModal();
}
