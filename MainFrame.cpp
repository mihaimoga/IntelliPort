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

/**
 * @class CMainFrame
 * @brief Main application frame window for IntelliPort.
 * 
 * This class manages the main window of the IntelliPort application, including:
 * - Ribbon bar and status bar UI elements
 * - Serial port and network socket connections
 * - Background threads for data reception
 * - Ring buffer for asynchronous data handling
 * - Caption bar for displaying notifications
 * 
 * The frame supports three connection types:
 * - Serial port communication (RS-232)
 * - TCP socket (client/server mode)
 * - UDP socket (datagram mode)
 */

// Enable dynamic creation of this frame class
IMPLEMENT_DYNCREATE(CMainFrame, CFrameWndEx)

// Message map - connects Windows messages and commands to handler functions
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

/**
 * @brief Constructor for CMainFrame.
 * 
 * Initializes the main frame window and all member variables:
 * - Loads application visual style from settings
 * - Creates ring buffer for data communication (64KB)
 * - Initializes threading variables to null/false
 * - Sets default connection parameters (invalid state until configured)
 * - Initializes default network settings (localhost:8080)
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
 * @brief Destructor for CMainFrame.
 * 
 * Performs cleanup before the frame is destroyed:
 * - Closes any open serial port or socket connection
 * - Stops background reading threads
 * - Destroys the ring buffer and releases memory
 */
CMainFrame::~CMainFrame()
{
	// Close any open serial port or socket connection
	OnCloseSerialPort();
	// Destroy the ring buffer
	m_pRingBuffer.Destroy();
}

/**
 * @brief Handles the WM_CREATE message during window creation.
 * 
 * Initializes all user interface elements and starts the timer:
 * - Sets up Visual Studio 2005 style docking behavior
 * - Creates and loads the ribbon bar from resources
 * - Creates the status bar with a single pane
 * - Creates the caption bar for notifications
 * - Creates the incoming connection dialog (for TCP server mode)
 * - Starts a 10ms timer for ring buffer checking
 * 
 * @param lpCreateStruct Pointer to CREATESTRUCT containing window creation parameters.
 * @return 0 on success, -1 if creation fails.
 */
int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	// Call base class OnCreate first
	if (CFrameWndEx::OnCreate(lpCreateStruct) == -1)
		return -1;

	// Set up modern docking window behavior (VS 2005 style)
	CDockingManager::SetDockingMode(DT_SMART);
	// Allow panes to auto-hide when docked to any edge
	EnableAutoHidePanes(CBRS_ALIGN_ANY);

	// Use Windows-style visual theme for all UI elements
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

	// Create the ribbon bar (Office-style toolbar)
	m_wndRibbonBar.Create(this);
	// Load ribbon configuration from resources
	m_wndRibbonBar.LoadFromResource(IDR_RIBBON);

	// Create the status bar at the bottom of the window
	if (!m_wndStatusBar.Create(this))
	{
		TRACE0("Failed to create status bar\n");
		return -1;
	}

	// Load the status pane title from string resources
	bool bNameValid;
	CString strTitlePane;
	bNameValid = strTitlePane.LoadString(IDS_STATUS_PANE1);
	ASSERT(bNameValid);
	// Add a single status bar pane with fixed width (60 characters)
	m_wndStatusBar.AddElement(new CMFCRibbonStatusBarPane(
		ID_STATUSBAR_PANE1, strTitlePane, TRUE, NULL,
		_T("012345678901234567890123456789012345678901234567890123456789")), strTitlePane);

	// Load and display initial status message
	VERIFY(strTitlePane.LoadString(IDS_LOG_HISTORY_CLEARED));
	SetStatusBarText(strTitlePane);

	// Create the caption bar (message notification area)
	if (!CreateCaptionBar())
	{
		TRACE0("Failed to create caption bar\n");
		return -1;
	}

	// Create the "waiting for connection" dialog (hidden initially)
	VERIFY(m_dlgIncoming.Create(CIncomingDlg::IDD, this));

	// Start a timer that fires every 10 milliseconds
	// Timer is used to check ring buffer for incoming data
	m_nTimerID = SetTimer(1, 10, NULL);

	return 0;
}

/**
 * @brief Called before the window is created.
 * 
 * Allows modification of window creation parameters before the window is actually created.
 * Can be overridden to change window class, styles, or other creation parameters.
 * 
 * @param cs Reference to the CREATESTRUCT structure containing window creation parameters.
 * @return TRUE if the window should be created, FALSE to prevent creation.
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
 * @brief Creates and configures the caption bar for displaying notifications.
 * 
 * The caption bar is used to display temporary status messages and notifications:
 * - Positioned at the top of the frame window
 * - Displays an info icon on the left side
 * - Includes tooltip support
 * - Initially hidden (shown when messages are displayed)
 * - Auto-hides after 10 seconds
 * 
 * @return true if caption bar was created successfully, false on failure.
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
 * @brief Handles the WM_DESTROY message during window destruction.
 * 
 * Performs cleanup before the window is destroyed:
 * - Kills the timer used for ring buffer checking
 * - Calls base class OnDestroy for standard cleanup
 */
void CMainFrame::OnDestroy()
{
	// Kill the timer
	VERIFY(KillTimer(m_nTimerID));

	CFrameWndEx::OnDestroy();
}

/**
 * @brief Handles the WM_TIMER message for periodic processing.
 * 
 * Called every 10 milliseconds to:
 * - Check the ring buffer for incoming data from serial port or socket
 * - Convert UTF-8 data to Unicode and display in the edit view
 * - Auto-hide the caption bar after 10 seconds of display
 * 
 * The ring buffer is accessed with mutex locking to ensure thread safety
 * between the UI thread and background reading threads.
 * 
 * @param nIDEvent Timer identifier (must match m_nTimerID).
 */
void CMainFrame::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == m_nTimerID)
	{
		// Calculate how long the caption bar has been visible
		CTime pDateTime = CTime::GetCurrentTime();
		CTimeSpan pTimeSpan = pDateTime - m_pCurrentDateTime;
		// Auto-hide caption bar after 10 seconds
		if ((pTimeSpan.GetTotalSeconds() >= 10) && (m_wndCaptionBar.IsPaneVisible()))
		{
			HideMessageBar();
		}

		// Read incoming data from ring buffer (up to 4KB)
		char pBuffer[0x1000] = { 0, };
		// Lock mutex to prevent conflicts with reading threads
		m_pMutualAccess.lock();
		// Check how much data is available in the ring buffer
		const int nLength = m_pRingBuffer.GetMaxReadSize();
		if (nLength > 0)
		{
			// Read binary data from ring buffer into local buffer
			m_pRingBuffer.ReadBinary(pBuffer, nLength);
			// Null-terminate the buffer for string safety
			pBuffer[nLength] = '\0';

			// Convert UTF-8 encoded data to Unicode (wide string)
			const std::string strRawText(pBuffer);
			CString strBuffer(utf8_to_wstring(strRawText).c_str());
			// Display the text in the edit view
			AddText(strBuffer);
		}
		// Release mutex lock
		m_pMutualAccess.unlock();
	}

	CFrameWndEx::OnTimer(nIDEvent);
}

/**
 * @brief Sets the text displayed in the status bar.
 * 
 * Updates the text in the first (and only) pane of the status bar.
 * Forces an immediate repaint to ensure the text is displayed.
 * 
 * @param strMessage The message to display in the status bar.
 * @return true if the status bar exists and text was set, false otherwise.
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
 * @brief Sets the text displayed in the caption bar and shows it.
 * 
 * Updates the caption bar message and makes it visible:
 * - Resets the timestamp for the 10-second auto-hide timer
 * - Shows the caption bar if it was hidden
 * - Aligns text to the left
 * - Forces a repaint and layout recalculation
 * 
 * @param strMessage The message to display in the caption bar.
 * @return true if the caption bar exists (note: currently always returns false).
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
 * @brief Hides the caption bar.
 * 
 * Makes the caption bar invisible and recalculates the frame layout
 * to adjust the space occupied by other elements.
 * 
 * @return true if successful (always returns true).
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
 * @brief Adds text to the active view's edit control.
 * 
 * Appends text to the end of the edit control with proper line ending normalization:
 * - Converts all line endings to CRLF (Windows standard)
 * - Handles CRLF, CR, and LF input formats
 * - Appends to the end of existing text
 * - Maintains cursor position at the end
 * 
 * @param strText The text to add to the view.
 * @return true if successful (always returns true).
 */
bool CMainFrame::AddText(CString strText)
{
	// Normalize line endings to Windows standard (CRLF)
	// Step 1: Convert CRLF (\r\n) to LF (\n) to avoid double conversion
	strText.Replace(CRLF, LF);
	// Step 2: Convert any remaining CR (\r) to LF (\n)
	strText.Replace(CR, LF);
	// Step 3: Convert all LF (\n) to CRLF (\r\n) for Windows
	strText.Replace(LF, CRLF);

	// Get reference to the edit control in the active view
	CEdit& pEdit = reinterpret_cast<CEditView*>(GetActiveView())->GetEditCtrl();
	// Get current text length to find end position
	int outLength = pEdit.GetWindowTextLength();
	// Move cursor to the end of existing text
	pEdit.SetSel(outLength, outLength);
	// Insert new text at cursor position (with undo support)
	pEdit.ReplaceSel(strText, TRUE);
	// Reset selection (deselect text)
	pEdit.SetSel(-1, 0);

	return true;
}

// CMainFrame diagnostics

#ifdef _DEBUG
/**
 * @brief Validates the main frame object in debug builds.
 * 
 * Performs diagnostic validation to ensure the object is in a valid state.
 * Used by MFC's debugging facilities to detect object corruption.
 */
void CMainFrame::AssertValid() const
{
	CFrameWndEx::AssertValid();
}

/**
 * @brief Dumps diagnostic information about the main frame.
 * 
 * Outputs diagnostic information to the specified dump context for debugging.
 * Called by MFC's diagnostic facilities.
 * 
 * @param dc Reference to the CDumpContext for outputting diagnostic information.
 */
void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWndEx::Dump(dc);
}
#endif //_DEBUG

// CMainFrame message handlers

/**
 * @brief Toggles the visibility of the caption bar.
 * 
 * Shows the caption bar if it's hidden, hides it if it's visible.
 * Recalculates the frame layout after changing visibility.
 */
void CMainFrame::OnViewCaptionBar()
{
	m_wndCaptionBar.ShowWindow(m_wndCaptionBar.IsVisible() ? SW_HIDE : SW_SHOW);
	RecalcLayout(FALSE);
}

/**
 * @brief Updates the UI state for the caption bar view menu item.
 * 
 * Sets the check mark on the menu item based on caption bar visibility.
 * 
 * @param pCmdUI Pointer to the CCmdUI object representing the menu item.
 */
void CMainFrame::OnUpdateViewCaptionBar(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_wndCaptionBar.IsVisible());
}

/**
 * @brief Displays the ribbon customization dialog.
 * 
 * Shows a modal dialog that allows users to customize the ribbon bar:
 * - Add/remove buttons
 * - Customize quick access toolbar
 * - Modify keyboard shortcuts
 */
void CMainFrame::OnOptions()
{
	CMFCRibbonCustomizeDialog *pOptionsDlg = new CMFCRibbonCustomizeDialog(this, &m_wndRibbonBar);
	ASSERT(pOptionsDlg != NULL);

	pOptionsDlg->DoModal();
	delete pOptionsDlg;
}

/**
 * @brief Handles the File Print command.
 * 
 * If currently in print preview mode, forwards the print command to the preview.
 * Otherwise, the base class handles normal printing.
 */
void CMainFrame::OnFilePrint()
{
	if (IsPrintPreview())
	{
		PostMessage(WM_COMMAND, AFX_ID_PREVIEW_PRINT);
	}
}

/**
 * @brief Handles the File Print Preview command.
 * 
 * Toggles print preview mode. If already in preview mode,
 * closes the preview and returns to normal view.
 */
void CMainFrame::OnFilePrintPreview()
{
	if (IsPrintPreview())
	{
		PostMessage(WM_COMMAND, AFX_ID_PREVIEW_CLOSE);  // force Print Preview mode closed
	}
}

/**
 * @brief Updates the UI state for the print preview menu item.
 * 
 * Sets the check mark on the menu item when in print preview mode.
 * 
 * @param pCmdUI Pointer to the CCmdUI object representing the menu item.
 */
void CMainFrame::OnUpdateFilePrintPreview(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(IsPrintPreview());
}

/**
 * @brief Displays the connection configuration dialog.
 * 
 * Shows a modal dialog allowing the user to configure:
 * - Connection type (Serial/TCP/UDP)
 * - Serial port settings (port, baud rate, data bits, parity, stop bits, flow control)
 * - Socket settings (server/client mode, IP addresses, ports)
 * 
 * Configuration is saved to the application object when OK is clicked.
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
 * @brief Opens a connection based on the configured settings.
 * 
 * Creates and opens the appropriate connection type:
 * 
 * Serial Port (Connection Type 0):
 * - Opens the COM port with configured parameters
 * - Starts SerialPortThreadFunc in a background thread
 * 
 * TCP Socket (Connection Type 1):
 * - Client mode: Connects to the configured server
 * - Server mode: Binds, listens, and accepts incoming connection
 * - Starts SocketThreadFunc in a background thread
 * 
 * UDP Socket (Connection Type 2):
 * - Creates and binds UDP socket for bidirectional communication
 * - Starts SocketThreadFunc in a background thread
 * 
 * Displays success/error messages in the caption bar.
 * Handles CSerialException and CWSocketException errors gracefully.
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
				// Windows requires \\.\COMx format for COM ports >= 10
				CString strFullPortName;
				strFullPortName.Format(_T("\\\\.\\%s"), static_cast<LPCWSTR>(theApp.m_strSerialName));

				// Open the serial port with all configured parameters
				// FALSE = non-overlapped (synchronous) mode
				m_pSerialPort.Open(
					strFullPortName,
					theApp.m_nBaudRate,                             // Baud rate (e.g., 9600, 115200)
					(CSerialPort::Parity) theApp.m_nParity,         // Parity (None, Odd, Even)
					(BYTE)theApp.m_nDataBits,                       // Data bits (7 or 8)
					(CSerialPort::StopBits) theApp.m_nStopBits,     // Stop bits (1, 1.5, or 2)
					(CSerialPort::FlowControl) theApp.m_nFlowControl, // Flow control
					FALSE);

				if (m_pSerialPort.IsOpen())
				{
					// Set flag to keep thread running
					m_nThreadRunning = true;
					// Create background thread to read incoming data
					m_hSerialPortThread = CreateThread(nullptr, 0, SerialPortThreadFunc, this, 0, &m_nSerialPortThreadID);

					// Show success message in caption bar
					VERIFY(strFormat.LoadString(IDS_SERIAL_PORT_OPENED));
					strMessage.Format(strFormat, static_cast<LPCWSTR>(theApp.m_strSerialName));
					SetCaptionBarText(strMessage);
				}
				break;
			}
			case 1: // TCP Socket Connection
			case 2: // UDP Socket Connection
			{
				// Get configured IP addresses and port numbers
				CString strServerIP = theApp.m_strServerIP;
				UINT nServerPort = theApp.m_nServerPort;
				CString strClientIP = theApp.m_strClientIP;
				UINT nClientPort = theApp.m_nClientPort;

				if (theApp.m_nConnection == 1) // TCP Socket
				{
					if (theApp.m_nSocketType == 1) // TCP Client
					{
						// Connect to remote TCP server
						m_pSocket.CreateAndConnect(strServerIP, nServerPort);
					}
					else // TCP Server
					{
						// Set up TCP server to accept incoming connections
						// Bind to specified local IP address and port
						m_pSocket.SetBindAddress(strClientIP);
						// Create socket with SOCK_STREAM (TCP) type
						m_pSocket.CreateAndBind(nClientPort, SOCK_STREAM, AF_INET);

						// Display "waiting for connection" dialog
						m_dlgIncoming.ShowWindow(SW_SHOW);
						m_dlgIncoming.CenterWindow(this);
						m_dlgIncoming.Invalidate();
						m_dlgIncoming.UpdateWindow();

						// Listen for incoming connections (backlog = 5)
						m_pSocket.Listen();
						// Block until a client connects (stores in m_pIncomming)
						m_pSocket.Accept(m_pIncomming);
						// Hide the waiting dialog
						m_dlgIncoming.ShowWindow(SW_HIDE);
					}
				}
				else // UDP Socket
				{
					// Create UDP socket for bidirectional communication
					// Bind to local address/port
					m_pSocket.SetBindAddress(strClientIP);
					// Create socket with SOCK_DGRAM (UDP) type
					m_pSocket.CreateAndBind(nClientPort, SOCK_DGRAM, AF_INET);

					// For UDP, use client IP/port for display
					strServerIP = strClientIP;
					nServerPort = nClientPort;
				}

				if (m_pSocket.IsCreated())
				{
					// Set flag to keep thread running
					m_nThreadRunning = true;
					// Create background thread to read incoming data
					m_hSocketThread = CreateThread(nullptr, 0, SocketThreadFunc, this, 0, &m_nSocketTreadID);

					// Show success message in caption bar
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
		// Handle serial port errors (port not available, access denied, etc.)
		const int nErrorLength = 0x100;
		TCHAR lpszErrorMessage[nErrorLength] = { 0, };
		pException.GetErrorMessage2(lpszErrorMessage, nErrorLength);
		TRACE(_T("%s\n"), lpszErrorMessage);
		SetCaptionBarText(lpszErrorMessage);
		m_nThreadRunning = false;
	}
	catch (CWSocketException* pException)
	{
		// Handle socket errors (connection refused, network unreachable, etc.)
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
 * @brief Closes the active connection (serial port or socket).
 * 
 * Performs graceful shutdown of the connection:
 * - Sets m_nThreadRunning to false to signal thread termination
 * - Waits for all background threads to complete (INFINITE timeout)
 * - Closes serial port or socket resources
 * - Displays confirmation message in caption bar
 * 
 * Handles both serial port and socket connections (TCP/UDP).
 * Thread synchronization ensures all I/O operations complete before closing.
 */
void CMainFrame::OnCloseSerialPort()
{
	// Signal threads to stop running
	if (m_nThreadRunning)
	{
		// Set flag to false - threads will check this and exit
		m_nThreadRunning = false;
		DWORD nThreadCount = 0;
		HANDLE hThreadArray[2] = { 0, 0 };

		// Build array of active thread handles
		if (m_hSerialPortThread != nullptr)
		{
			hThreadArray[nThreadCount++] = m_hSerialPortThread;
		}
		if (m_hSocketThread != nullptr)
		{
			hThreadArray[nThreadCount++] = m_hSocketThread;
		}

		// Wait for all threads to finish (blocking call)
		// TRUE = wait for ALL threads, INFINITE = no timeout
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
 * @brief Displays the input dialog and sends data through the active connection.
 * 
 * Shows a modal input dialog for the user to enter data to send:
 * - Converts Unicode input to UTF-8 for transmission
 * - Sends data through the appropriate connection type:
 *   * Serial Port: Uses CSerialPort::Write()
 *   * TCP Client: Uses CWSocket::Send()
 *   * TCP Server: Uses incoming socket Send()
 *   * UDP: Uses CWSocket::SendTo() with configured server address
 * 
 * Uses mutex locking to prevent conflicts with reading thread.
 * Plays a beep sound on success or error.
 * Displays error messages in caption bar if transmission fails.
 */
void CMainFrame::OnSendReceive()
{
	int nLength = 0;
	// Show input dialog for user to enter data
	CInputDlg dlgInput(this);
	if (dlgInput.DoModal() == IDOK)
	{
		// Convert Unicode (UTF-16) to UTF-8 for transmission
		// Most serial/network protocols use UTF-8 encoding
		const std::wstring strRawText(dlgInput.m_strSendData);
		CStringA pBuffer(wstring_to_utf8(strRawText).c_str());
		nLength = pBuffer.GetLength();

		if (nLength > 0)
		{
			switch (theApp.m_nConnection)
			{
				case 0: // Serial Port
				{
					// Lock mutex to prevent conflicts with reading thread
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
					// Release mutex lock
					m_pMutualAccess.unlock();
					// Play success sound
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
 * @brief Updates the UI state for the configure command.
 * 
 * Enables the configure menu item only when no connection is active.
 * Prevents configuration changes while connected.
 * 
 * @param pCmdUI Pointer to the CCmdUI object representing the menu item.
 */
void CMainFrame::OnUpdateConfigureSerialPort(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(!m_pSerialPort.IsOpen() && !m_pSocket.IsCreated());
}

/**
 * @brief Updates the UI state for the open connection command.
 * 
 * Enables the open menu item only when no connection is currently active.
 * Prevents multiple simultaneous connections.
 * 
 * @param pCmdUI Pointer to the CCmdUI object representing the menu item.
 */
void CMainFrame::OnUpdateOpenSerialPort(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(!m_pSerialPort.IsOpen() && !m_pSocket.IsCreated());
}

/**
 * @brief Updates the UI state for the close connection command.
 * 
 * Enables the close menu item only when a connection is active.
 * Disabled when no connection exists.
 * 
 * @param pCmdUI Pointer to the CCmdUI object representing the menu item.
 */
void CMainFrame::OnUpdateCloseSerialPort(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_pSerialPort.IsOpen() || m_pSocket.IsCreated());
}

/**
 * @brief Updates the UI state for the send/receive command.
 * 
 * Enables the send/receive menu item only when a connection is active.
 * Data can only be sent when connected.
 * 
 * @param pCmdUI Pointer to the CCmdUI object representing the menu item.
 */
void CMainFrame::OnUpdateSendReceive(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_pSerialPort.IsOpen() || m_pSocket.IsCreated());
}

/**
 * @brief Background thread function for reading data from the serial port.
 * 
 * Runs continuously while m_nThreadRunning is true:
 * - Checks serial port status to see if data is available (cbInQue)
 * - Reads available data into a local buffer (up to 4KB)
 * - Writes data to the ring buffer with mutex protection
 * - Handles CSerialException errors by displaying message and breaking loop
 * 
 * Thread cleanup:
 * - Closes the serial port
 * - Sets m_nThreadRunning to false
 * - Nulls the thread handle
 * 
 * @param pParam Pointer to the CMainFrame instance (cast from LPVOID).
 * @return Thread exit code (always 0).
 */
DWORD WINAPI SerialPortThreadFunc(LPVOID pParam)
{
	int nLength = 0;
	COMSTAT status = { 0, };
	char pBuffer[0x1000] = { 0, }; // 4KB buffer for reading
	// Cast parameter to CMainFrame pointer
	CMainFrame* pMainFrame = (CMainFrame*) pParam;
	// Get references to shared resources
	CRingBuffer& pRingBuffer = pMainFrame->m_pRingBuffer;
	CSerialPort& pSerialPort = pMainFrame->m_pSerialPort;
	std::mutex& pMutualAccess = pMainFrame->m_pMutualAccess;

	// Main reading loop - continues until thread is stopped
	while (pMainFrame->m_nThreadRunning)
	{
		try
		{
			// Query serial port status to check for incoming data
			memset(&status, 0, sizeof(status));
			pSerialPort.GetStatus(status);
			// cbInQue = number of bytes in input buffer
			if (status.cbInQue > 0)
			{
				// Read available data from serial port
				memset(pBuffer, 0, sizeof(pBuffer));
				nLength = pSerialPort.Read(pBuffer, sizeof(pBuffer));
			}
		}
		catch (CSerialException& pException)
		{
			// Handle errors and notify user
			const int nErrorLength = 0x100;
			TCHAR lpszErrorMessage[nErrorLength] = { 0, };
			pException.GetErrorMessage2(lpszErrorMessage, nErrorLength);
			TRACE(_T("%s\n"), lpszErrorMessage);
			pMainFrame->SetCaptionBarText(lpszErrorMessage);
			MessageBeep(MB_ICONERROR);
			// Break out of loop on error
			break;
		}

		// If data was read, write it to the ring buffer
		if (nLength > 0)
		{
			// Lock mutex to prevent conflicts with UI thread
			pMutualAccess.lock();
			pRingBuffer.WriteBinary(pBuffer, nLength);
			pMutualAccess.unlock();
			// Reset length for next iteration
			nLength = 0;
		}
	}

	// Cleanup before thread exits
	pSerialPort.Close();
	pMainFrame->m_nThreadRunning = false;
	pMainFrame->m_hSerialPortThread = nullptr;
	return 0;
}

/**
 * @brief Background thread function for reading data from the socket.
 * 
 * Runs continuously while m_nThreadRunning is true:
 * 
 * TCP Client Mode:
 * - Checks if socket is readable with 1 second timeout
 * - Receives data from the connected server
 * 
 * TCP Server Mode:
 * - Checks if incoming connection socket is readable
 * - Receives data from the connected client
 * 
 * UDP Mode:
 * - Checks if socket is readable
 * - Receives datagram and source address
 * 
 * All received data is written to the ring buffer with mutex protection.
 * Handles CWSocketException errors by displaying message and breaking loop.
 * 
 * Thread cleanup:
 * - Closes all socket handles
 * - Sets m_nThreadRunning to false
 * - Nulls the thread handle
 * 
 * @param pParam Pointer to the CMainFrame instance (cast from LPVOID).
 * @return Thread exit code (always 0).
 */
DWORD WINAPI SocketThreadFunc(LPVOID pParam)
{
	int nLength = 0;
	char pBuffer[0x1000] = { 0, }; // 4KB buffer for reading
	// Cast parameter to CMainFrame pointer
	CMainFrame* pMainFrame = (CMainFrame*) pParam;
	// Get references to shared resources
	CRingBuffer& pRingBuffer = pMainFrame->m_pRingBuffer;
	CWSocket& pSocket = pMainFrame->m_pSocket;
	CWSocket& pIncomming = pMainFrame->m_pIncomming;
	std::mutex& pMutualAccess = pMainFrame->m_pMutualAccess;
	// Cache connection type to avoid repeated global access
	bool bIsTCP = (theApp.m_nConnection == 1);
	bool bIsClient = (theApp.m_nSocketType == 1);

	// Variables for UDP source address
	CString strServerIP = theApp.m_strServerIP;
	UINT nServerPort = theApp.m_nServerPort;

	// Main reading loop - continues until thread is stopped
	while (pMainFrame->m_nThreadRunning)
	{
		try
		{
			if (bIsTCP)
			{
				if (bIsClient)
				{
					// TCP Client: Read from server connection
					// IsReadible checks if data is available (1 second timeout)
					if (pSocket.IsReadible(1000))
					{
						memset(pBuffer, 0, sizeof(pBuffer));
						nLength = pSocket.Receive(pBuffer, sizeof(pBuffer), 0);
					}
				}
				else
				{
					// TCP Server: Read from accepted client connection
					if (pIncomming.IsReadible(1000))
					{
						memset(pBuffer, 0, sizeof(pBuffer));
						nLength = pIncomming.Receive(pBuffer, sizeof(pBuffer), 0);
					}
				}
			}
			else
			{
				// UDP: Read datagram (also returns sender's address)
				if (pSocket.IsReadible(1000))
				{
					memset(pBuffer, 0, sizeof(pBuffer));
					nLength = pSocket.ReceiveFrom(pBuffer, sizeof(pBuffer), strServerIP, nServerPort, 0);
				}
			}
		}
		catch (CWSocketException* pException)
		{
			// Handle errors and notify user
			const int nErrorLength = 0x100;
			TCHAR lpszErrorMessage[nErrorLength] = { 0, };
			pException->GetErrorMessage(lpszErrorMessage, nErrorLength);
			TRACE(_T("%s\n"), lpszErrorMessage);
			pException->Delete();
			pMainFrame->SetCaptionBarText(lpszErrorMessage);
			MessageBeep(MB_ICONERROR);
			// Break out of loop on error
			break;
		}

		// If data was received, write it to the ring buffer
		if (nLength > 0)
		{
			// Lock mutex to prevent conflicts with UI thread
			pMutualAccess.lock();
			pRingBuffer.WriteBinary(pBuffer, nLength);
			pMutualAccess.unlock();
			// Reset length for next iteration
			nLength = 0;
		}
	}

	// Cleanup before thread exits
	pIncomming.Close();
	pSocket.Close();
	pMainFrame->m_nThreadRunning = false;
	pMainFrame->m_hSocketThread = nullptr;
	return 0;
}

/**
 * @brief Opens the developer's Twitter/X profile in the default browser.
 * 
 * Launches the default web browser to display the developer's social media profile.
 */
void CMainFrame::OnTwitter()
{
	::ShellExecute(GetSafeHwnd(), _T("open"), _T("https://x.com/stefanmihaimoga"), nullptr, nullptr, SW_SHOW);
}

/**
 * @brief Opens the developer's LinkedIn profile in the default browser.
 * 
 * Launches the default web browser to display the developer's professional profile.
 */
void CMainFrame::OnLinkedin()
{
	::ShellExecute(GetSafeHwnd(), _T("open"), _T("https://www.linkedin.com/in/stefanmihaimoga/"), nullptr, nullptr, SW_SHOW);
}

/**
 * @brief Opens the developer's Facebook profile in the default browser.
 * 
 * Launches the default web browser to display the developer's social media profile.
 */
void CMainFrame::OnFacebook()
{
	::ShellExecute(GetSafeHwnd(), _T("open"), _T("https://www.facebook.com/stefanmihaimoga"), nullptr, nullptr, SW_SHOW);
}

/**
 * @brief Opens the developer's Instagram profile in the default browser.
 * 
 * Launches the default web browser to display the developer's social media profile.
 */
void CMainFrame::OnInstagram()
{
	::ShellExecute(GetSafeHwnd(), _T("open"), _T("https://www.instagram.com/stefanmihaimoga/"), nullptr, nullptr, SW_SHOW);
}

/**
 * @brief Opens the GitHub Issues page in the default browser.
 * 
 * Launches the default web browser to display the project's issue tracker
 * where users can report bugs or request features.
 */
void CMainFrame::OnIssues()
{
	::ShellExecute(GetSafeHwnd(), _T("open"), _T("https://github.com/mihaimoga/IntelliPort/issues"), nullptr, nullptr, SW_SHOW);
}

/**
 * @brief Opens the GitHub Discussions page in the default browser.
 * 
 * Launches the default web browser to display the project's discussion forum
 * where users can ask questions and share ideas.
 */
void CMainFrame::OnDiscussions()
{
	::ShellExecute(GetSafeHwnd(), _T("open"), _T("https://github.com/mihaimoga/IntelliPort/discussions"), nullptr, nullptr, SW_SHOW);
}

/**
 * @brief Opens the GitHub Wiki page in the default browser.
 * 
 * Launches the default web browser to display the project's wiki documentation.
 */
void CMainFrame::OnWiki()
{
	::ShellExecute(GetSafeHwnd(), _T("open"), _T("https://github.com/mihaimoga/IntelliPort/wiki"), nullptr, nullptr, SW_SHOW);
}

/**
 * @brief Displays the user manual in a web browser dialog.
 * 
 * Shows a modal dialog with an embedded web browser control
 * displaying the application's user manual/help documentation.
 */
void CMainFrame::OnUserManual()
{
	CWebBrowserDlg dlgWebBrowser(this);
	dlgWebBrowser.DoModal();
}

/**
 * @brief Displays the Check for Updates dialog.
 * 
 * Shows a modal dialog that checks for application updates
 * and allows the user to download newer versions if available.
 */
void CMainFrame::OnCheckForUpdates()
{
	CCheckForUpdatesDlg dlgCheckForUpdates(this);
	dlgCheckForUpdates.DoModal();
}
