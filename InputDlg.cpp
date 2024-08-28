/* Copyright (C) 2014-2024 Stefan-Mihai MOGA
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

// CInputDlg dialog

IMPLEMENT_DYNAMIC(CInputDlg, CDialogEx)

CInputDlg::CInputDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CInputDlg::IDD, pParent)
{
	VERIFY(m_fontTerminal.CreateFont(
		-MulDiv(10, GetDeviceCaps(::GetDC(nullptr), LOGPIXELSY), 72), // nHeight
		0,                         // nWidth
		0,                         // nEscapement
		0,                         // nOrientation
		FW_NORMAL,                 // nWeight
		FALSE,                     // bItalic
		FALSE,                     // bUnderline
		0,                         // cStrikeOut
		ANSI_CHARSET,              // nCharSet
		OUT_DEFAULT_PRECIS,        // nOutPrecision
		CLIP_DEFAULT_PRECIS,       // nClipPrecision
		ANTIALIASED_QUALITY,       // nQuality
		DEFAULT_PITCH | FF_MODERN, // nPitchAndFamily
		_T("Consolas")));			// lpszFacename 
}

CInputDlg::~CInputDlg()
{
}

void CInputDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SEND_DATA, m_pSendData);
}

BEGIN_MESSAGE_MAP(CInputDlg, CDialogEx)
	ON_WM_DESTROY()
END_MESSAGE_MAP()

// CInputDlg message handlers

BOOL CInputDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_pSendData.SetFont(&m_fontTerminal);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CInputDlg::OnDestroy()
{
	CDialogEx::OnDestroy();
}

void CInputDlg::OnOK()
{
	m_pSendData.GetWindowText(m_strSendData);

	CDialogEx::OnOK();
}
