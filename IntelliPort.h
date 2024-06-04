/* Copyright (C) 2014-2024 Stefan-Mihai MOGA
This file is part of IntelliPort application developed by Stefan-Mihai MOGA.

IntelliPort is free software: you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the Open
Source Initiative, either version 3 of the License, or any later version.

IntelliPort is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
IntelliPort. If not, see <http://www.opensource.org/licenses/gpl-3.0.html>*/

// IntelliPort.h : main header file for the IntelliPort application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols

std::wstring utf8_to_wstring(const std::string& str);
std::string wstring_to_utf8(const std::wstring& str);

// CIntelliPortApp:
// See IntelliPort.cpp for the implementation of this class
//

class CIntelliPortApp : public CWinAppEx
{
public:
	int m_nConnection;
	CString m_strSerialName;
	int m_nBaudRate;
	int m_nDataBits;
	int m_nParity;
	int m_nStopBits;
	int m_nFlowControl;
	int m_nSocketType;
	CString m_strServerIP;
	int m_nServerPort;
	CString m_strClientIP;
	int m_nClientPort;

public:
	CIntelliPortApp();

// Overrides
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// Implementation
	UINT  m_nAppLook;
	virtual void PreLoadState();
	virtual void LoadCustomState();
	virtual void SaveCustomState();

protected:
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CIntelliPortApp theApp;
