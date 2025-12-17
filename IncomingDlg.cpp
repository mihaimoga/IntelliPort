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

// IncomingDlg.cpp : implementation file
//

#include "stdafx.h"
#include "IntelliPort.h"
#include "IncomingDlg.h"

// CIncomingDlg dialog

IMPLEMENT_DYNAMIC(CIncomingDlg, CDialogEx)

CIncomingDlg::CIncomingDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CIncomingDlg::IDD, pParent)
{
}

CIncomingDlg::~CIncomingDlg()
{
}

void CIncomingDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CIncomingDlg, CDialogEx)
END_MESSAGE_MAP()

// CIncomingDlg message handlers
