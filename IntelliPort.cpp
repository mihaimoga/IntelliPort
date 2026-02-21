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

// IntelliPort.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "IntelliPort.h"
#include "MainFrame.h"
#include "IntelliPortDoc.h"
#include "IntelliPortView.h"
#include "VersionInfo.h"
#include "HLinkCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/**
 * @brief Converts a UTF-8 encoded string to a wide string (UTF-16).
 * @param string The UTF-8 encoded input string to convert.
 * @return A wide string (std::wstring) representation of the input.
 * @throws std::runtime_error If the conversion fails.
 */
std::wstring utf8_to_wstring(const std::string& string)
{
	// Handle empty string case early
	if (string.empty())
	{
		return L"";
	}

	// First call to get the required buffer size
	const auto size_needed = MultiByteToWideChar(CP_UTF8, 0, string.data(), (int)string.size(), nullptr, 0);
	if (size_needed <= 0)
	{
		throw std::runtime_error("MultiByteToWideChar() failed: " + std::to_string(size_needed));
	}

	// Allocate buffer and perform the actual conversion
	std::wstring result(size_needed, 0);
	MultiByteToWideChar(CP_UTF8, 0, string.data(), (int)string.size(), result.data(), size_needed);
	return result;
}

/**
 * @brief Converts a wide string (UTF-16) to a UTF-8 encoded string.
 * @param wide_string The wide string input to convert.
 * @return A UTF-8 encoded string (std::string) representation of the input.
 * @throws std::runtime_error If the conversion fails.
 */
std::string wstring_to_utf8(const std::wstring& wide_string)
{
	// Handle empty string case early
	if (wide_string.empty())
	{
		return "";
	}

	// First call to get the required buffer size
	const auto size_needed = WideCharToMultiByte(CP_UTF8, 0, wide_string.data(), (int)wide_string.size(), nullptr, 0, nullptr, nullptr);
	if (size_needed <= 0)
	{
		throw std::runtime_error("WideCharToMultiByte() failed: " + std::to_string(size_needed));
	}

	// Allocate buffer and perform the actual conversion
	std::string result(size_needed, 0);
	WideCharToMultiByte(CP_UTF8, 0, wide_string.data(), (int)wide_string.size(), result.data(), size_needed, nullptr, nullptr);
	return result;
}

/**
 * @class CIntelliPortApp
 * @brief Main application class for IntelliPort.
 * 
 * This class manages the application lifecycle, document templates,
 * and user interface settings for the IntelliPort serial terminal application.
 */

// CIntelliPortApp

BEGIN_MESSAGE_MAP(CIntelliPortApp, CWinAppEx)
	ON_COMMAND(ID_APP_ABOUT, &CIntelliPortApp::OnAppAbout)
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, &CWinAppEx::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, &CWinAppEx::OnFileOpen)
	// Standard print setup command
	ON_COMMAND(ID_FILE_PRINT_SETUP, &CWinAppEx::OnFilePrintSetup)
END_MESSAGE_MAP()

/**
 * @brief Constructor for CIntelliPortApp.
 * 
 * Initializes the application settings including Restart Manager support,
 * application ID, and connection parameters.
 */
CIntelliPortApp::CIntelliPortApp()
{
	// Enable HTML Help support for context-sensitive help
	EnableHtmlHelp();

	// Enable full Restart Manager support for application recovery
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_ALL_ASPECTS;
#ifdef _MANAGED
	// If the application is built using Common Language Runtime support (/clr):
	//     1) This additional setting is needed for Restart Manager support to work properly.
	//     2) In your project, you must add a reference to System.Windows.Forms in order to build.
	System::Windows::Forms::Application::SetUnhandledExceptionMode(System::Windows::Forms::UnhandledExceptionMode::ThrowException);
#endif

	// Set unique application ID for taskbar grouping and jump lists
	// TODO: replace application ID string below with unique ID string; recommended
	// format for string is CompanyName.ProductName.SubProduct.VersionInformation
	SetAppID(_T("IntelliPort.AppID.NoVersion"));

	// Initialize all connection and communication parameters to default values
	m_nConnection = 0;    // Connection type (serial, TCP, UDP, etc.)
	m_nBaudRate = 0;      // Serial port baud rate
	m_nDataBits = 0;      // Serial port data bits
	m_nParity = 0;        // Serial port parity setting
	m_nStopBits = 0;      // Serial port stop bits
	m_nFlowControl = 0;   // Serial port flow control
	m_nSocketType = 0;    // Network socket type
	m_nServerPort = 0;    // Server port number
	m_nClientPort = 0;    // Client port number
	m_nAppLook = 0;       // Application visual theme
}

// The one and only CIntelliPortApp object

CIntelliPortApp theApp;

/**
 * @brief Initializes the application instance.
 * 
 * Performs initialization tasks including:
 * - Common controls initialization
 * - Socket and OLE library initialization
 * - Rich edit control initialization
 * - Document template registration
 * - Command line parsing and processing
 * 
 * @return TRUE if initialization succeeds, FALSE otherwise.
 */
BOOL CIntelliPortApp::InitInstance()
{
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	// Initialize common controls required by the application
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	// Call base class initialization
	CWinAppEx::InitInstance();

	// Initialize Windows Sockets for network communication
	if (!AfxSocketInit())
	{
		AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
		return FALSE;
	}

	// Initialize OLE libraries for COM support and drag-drop functionality
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}

	// Enable use of ActiveX controls within dialogs
	AfxEnableControlContainer();

	// Disable taskbar thumbnails and previews
	EnableTaskbarInteraction(FALSE);

	// Initialize RichEdit 2.0 control for advanced text editing
	AfxInitRichEdit2();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need

	// Set registry key for storing application settings
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Mihai Moga"));
	// Load application settings and Most Recently Used (MRU) file list (max 10 items)
	LoadStdProfileSettings(10);

	// Initialize context menu manager for right-click menus
	InitContextMenuManager();

	// Initialize keyboard manager for customizable keyboard shortcuts
	InitKeyboardManager();

	// Initialize tooltip manager and configure tooltip appearance
	InitTooltipManager();
	CMFCToolTipInfo ttParams;
	ttParams.m_bVislManagerTheme = TRUE;  // Use visual manager theme for tooltips
	theApp.GetTooltipManager()->SetTooltipParams(AFX_TOOLTIP_TYPE_ALL,
		RUNTIME_CLASS(CMFCToolTipCtrl), &ttParams);

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views
	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,                    // Resource ID for menus and icons
		RUNTIME_CLASS(CIntelliPortDoc),   // Document class
		RUNTIME_CLASS(CMainFrame),        // Main SDI frame window
		RUNTIME_CLASS(CIntelliPortView)); // View class
	if (!pDocTemplate)
		return FALSE;
	AddDocTemplate(pDocTemplate);

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	// Enable Windows Shell integration for file associations
	EnableShellOpen();
	RegisterShellFileTypes(TRUE);

	// Process command line arguments (file open, print, etc.)
	// Will return FALSE if app was launched with /RegServer, /Register, /Unregserver or /Unregister.
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	// Display and update the main application window
	m_pMainWnd->ShowWindow(SW_SHOW);
	m_pMainWnd->UpdateWindow();
	// Optional: Set specific window size and position
	// m_pMainWnd->MoveWindow(CRect(0, 0, 1214, 907));
	// m_pMainWnd->CenterWindow();

	// Enable drag-and-drop file opening
	// Note: In an SDI app, this should occur after ProcessShellCommand
	m_pMainWnd->DragAcceptFiles();

	return TRUE;
}

/**
 * @brief Handles application exit and cleanup.
 * 
 * Performs cleanup of OLE libraries and other resources before
 * the application terminates.
 * 
 * @return Exit code for the application.
 */
int CIntelliPortApp::ExitInstance()
{
	// Clean up OLE libraries before application exits
	// FALSE parameter means don't force termination if OLE is still in use
	AfxOleTerm(FALSE);

	// Call base class cleanup and return exit code
	return CWinAppEx::ExitInstance();
}

// CIntelliPortApp message handlers

/**
 * @class CAboutDlg
 * @brief About dialog for the IntelliPort application.
 * 
 * Displays application version information, copyright notice,
 * and links to website, email, and contributors page.
 */
class CAboutDlg : public CDialogEx
{
public:
	/**
	 * @brief Constructor for the About dialog.
	 */
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

protected:
	/**
	 * @brief Exchanges data between dialog controls and member variables.
	 * @param pDX Pointer to the data exchange context.
	 */
	virtual void DoDataExchange(CDataExchange* pDX);

// Implementation
public:
	/**
	 * @brief Initializes the About dialog.
	 * 
	 * Loads version information and sets up hyperlinks for website,
	 * email, and contributors page.
	 * 
	 * @return TRUE if initialization succeeds, FALSE otherwise.
	 */
	virtual BOOL OnInitDialog();
	/**
	 * @brief Handles cleanup when the dialog is destroyed.
	 */
	afx_msg void OnDestroy();

protected:
	CStatic m_ctrlVersion;
	CEdit m_ctrlWarning;
	CVersionInfo m_pVersionInfo;
	CHLinkCtrl m_ctrlWebsite;
	CHLinkCtrl m_ctrlEmail;
	CHLinkCtrl m_ctrlContributors;

	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_VERSION, m_ctrlVersion);
	DDX_Control(pDX, IDC_WARNING, m_ctrlWarning);
	DDX_Control(pDX, IDC_WEBSITE, m_ctrlWebsite);
	DDX_Control(pDX, IDC_EMAIL, m_ctrlEmail);
	DDX_Control(pDX, IDC_CONTRIBUTORS, m_ctrlContributors);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
	ON_WM_DESTROY()
END_MESSAGE_MAP()

/**
 * @brief Retrieves the full path of the current module (executable).
 * 
 * This function retrieves the file path of the currently running executable,
 * dynamically adjusting the buffer size if needed.
 * 
 * @param pdwLastError Optional pointer to receive the last error code.
 * @return The full path to the module, or an empty string if the function fails.
 */
CString GetModuleFileName(_Inout_opt_ DWORD* pdwLastError = nullptr)
{
	CString strModuleFileName;
	DWORD dwSize{ _MAX_PATH };  // Start with standard MAX_PATH size
	while (true)
	{
		// Allocate buffer and attempt to get module file name
		TCHAR* pszModuleFileName{ strModuleFileName.GetBuffer(dwSize) };
		const DWORD dwResult{ ::GetModuleFileName(nullptr, pszModuleFileName, dwSize) };

		if (dwResult == 0)
		{
			// Function failed - return error code and empty string
			if (pdwLastError != nullptr)
				*pdwLastError = GetLastError();
			strModuleFileName.ReleaseBuffer(0);
			return CString{};
		}
		else if (dwResult < dwSize)
		{
			// Success - buffer was large enough
			if (pdwLastError != nullptr)
				*pdwLastError = ERROR_SUCCESS;
			strModuleFileName.ReleaseBuffer(dwResult);
			return strModuleFileName;
		}
		else if (dwResult == dwSize)
		{
			// Buffer too small - double the size and try again
			strModuleFileName.ReleaseBuffer(0);
			dwSize *= 2;
		}
	}
}

BOOL CAboutDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Get the path to the current executable
	CString strFullPath{ GetModuleFileName() };
	if (strFullPath.IsEmpty())
#pragma warning(suppress: 26487)
		return FALSE;

	// Load and display version information from the executable
	if (m_pVersionInfo.Load(strFullPath.GetString()))
	{
		// Extract product name and version string
		CString strName = m_pVersionInfo.GetProductName().c_str();
		CString strVersion = m_pVersionInfo.GetProductVersionAsString().c_str();

		// Clean up version string format (remove spaces, replace commas with dots)
		strVersion.Replace(_T(" "), _T(""));
		strVersion.Replace(_T(","), _T("."));

		// Extract only major.minor version (e.g., "1.2" from "1.2.3.4")
		const int nFirst = strVersion.Find(_T('.'));
		const int nSecond = strVersion.Find(_T('.'), nFirst + 1);
		strVersion.Truncate(nSecond);

		// Display version with platform architecture (32-bit or 64-bit)
#if _WIN32 || _WIN64
#if _WIN64
		m_ctrlVersion.SetWindowText(strName + _T(" version ") + strVersion + _T(" (64-bit)"));
#else
		m_ctrlVersion.SetWindowText(strName + _T(" version ") + strVersion + _T(" (32-bit)"));
#endif
#endif
	}

	// Display GPL license information
	m_ctrlWarning.SetWindowText(_T("This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version. This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>."));

	// Set up hyperlinks for website, email, and contributors page
	m_ctrlWebsite.SetHyperLink(_T("https://www.moga.doctor/"));
	m_ctrlEmail.SetHyperLink(_T("mailto:stefan-mihai@moga.doctor"));
	m_ctrlContributors.SetHyperLink(_T("https://github.com/mihaimoga/IntelliPort/graphs/contributors"));

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CAboutDlg::OnDestroy()
{
	CDialogEx::OnDestroy();
}

/**
 * @brief Displays the About dialog.
 * 
 * This command handler shows the application's About dialog box with
 * version information and copyright details.
 */
void CIntelliPortApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

/**
 * @brief Performs pre-load state initialization.
 * 
 * Loads menu resources and initializes the context menu manager
 * before the application state is loaded.
 */
void CIntelliPortApp::PreLoadState()
{
	// Load the Edit menu name from string resources
	BOOL bNameValid;
	CString strName;
	bNameValid = strName.LoadString(IDS_EDIT_MENU);
	ASSERT(bNameValid);

	// Register the Edit context menu with the context menu manager
	GetContextMenuManager()->AddMenu(strName, IDR_POPUP_EDIT);
}

/**
 * @brief Loads custom application state from the registry.
 * 
 * This method can be overridden to load custom settings that are
 * not handled by the default state persistence mechanism.
 */
void CIntelliPortApp::LoadCustomState()
{
}

/**
 * @brief Saves custom application state to the registry.
 * 
 * This method can be overridden to save custom settings that are
 * not handled by the default state persistence mechanism.
 */
void CIntelliPortApp::SaveCustomState()
{
}

// CIntelliPortApp message handlers
