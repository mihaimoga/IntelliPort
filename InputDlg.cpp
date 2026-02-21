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

// InputDlg.cpp : implementation file
//

#include "stdafx.h"
#include "IntelliPort.h"
#include "InputDlg.h"

/**
 * @class CInputDlg
 * @brief Dialog for inputting data to send through the serial/network connection.
 * 
 * This dialog provides a text input field for users to enter data that will be
 * transmitted through the active connection (serial port, TCP, or UDP).
 */

// Enable runtime type information for this dialog class
IMPLEMENT_DYNAMIC(CInputDlg, CDialogEx)

/**
 * @brief Constructor for CInputDlg.
 * 
 * Initializes the dialog and creates a Consolas monospace font for the input field,
 * which is appropriate for terminal/serial communication display.
 * 
 * @param pParent Pointer to the parent window, or NULL for no parent.
 */
CInputDlg::CInputDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CInputDlg::IDD, pParent)
{
	// Create a monospace terminal font (Consolas, 10pt) for clear text display
	VERIFY(m_fontTerminal.CreateFont(
		-MulDiv(10, GetDeviceCaps(::GetDC(nullptr), LOGPIXELSY), 72), // nHeight - 10 point size
		0,                         // nWidth - default width
		0,                         // nEscapement - no rotation
		0,                         // nOrientation - no text angle
		FW_NORMAL,                 // nWeight - normal weight
		FALSE,                     // bItalic - not italic
		FALSE,                     // bUnderline - not underlined
		0,                         // cStrikeOut - no strikethrough
		ANSI_CHARSET,              // nCharSet - ANSI character set
		OUT_DEFAULT_PRECIS,        // nOutPrecision - default precision
		CLIP_DEFAULT_PRECIS,       // nClipPrecision - default clipping
		ANTIALIASED_QUALITY,       // nQuality - antialiased for smooth rendering
		DEFAULT_PITCH | FF_MODERN, // nPitchAndFamily - monospace modern font
		_T("Consolas")));          // lpszFacename - Consolas font
}

/**
 * @brief Destructor for CInputDlg.
 * 
 * Cleans up resources. The font object is automatically destroyed.
 */
CInputDlg::~CInputDlg()
{
}

/**
 * @brief Exchanges data between dialog controls and member variables.
 * 
 * Maps the IDC_SEND_DATA control to the m_pSendData member variable
 * for easy access to the input field.
 * 
 * @param pDX Pointer to the data exchange context.
 */
void CInputDlg::DoDataExchange(CDataExchange* pDX)
{
	// Call base class data exchange
	CDialogEx::DoDataExchange(pDX);

	// Map the send data edit control to the member variable
	DDX_Control(pDX, IDC_SEND_DATA, m_pSendData);
}

// Message map - connects Windows messages to handler functions
BEGIN_MESSAGE_MAP(CInputDlg, CDialogEx)
	ON_WM_DESTROY()  // Handle WM_DESTROY message for cleanup
END_MESSAGE_MAP()

// CInputDlg message handlers

/**
 * @brief Initializes the input dialog.
 * 
 * Sets the terminal font for the send data input control to provide
 * a monospace display suitable for terminal communication.
 * 
 * @return TRUE to set focus to the first control, FALSE if focus was set manually.
 */
BOOL CInputDlg::OnInitDialog()
{
	// Call base class initialization
	CDialogEx::OnInitDialog();

	// Apply the monospace terminal font to the input field
	m_pSendData.SetFont(&m_fontTerminal);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

/**
 * @brief Handles cleanup when the dialog is destroyed.
 * 
 * Performs any necessary cleanup before the dialog window is destroyed.
 */
void CInputDlg::OnDestroy()
{
	// Call base class destroy handler to ensure proper cleanup
	CDialogEx::OnDestroy();
}

/**
 * @brief Handles the OK button click event.
 * 
 * Retrieves the text entered by the user from the input control and stores it
 * in m_strSendData before closing the dialog with IDOK result.
 */
void CInputDlg::OnOK()
{
	// Retrieve the text from the input control and store in member variable
	m_pSendData.GetWindowText(m_strSendData);

	// Close the dialog with OK result
	CDialogEx::OnOK();
}
