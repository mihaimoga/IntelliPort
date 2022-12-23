/* This file is part of IntelliPort application developed by Mihai MOGA.

IntelliPort is free software: you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the Open
Source Initiative, either version 3 of the License, or any later version.

IntelliPort is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
IntelliPort. If not, see <http://www.opensource.org/licenses/gpl-3.0.html>*/

// ConfigureDlg.h : header file
//

#pragma once

#include "enumser.h"

// CConfigureDlg dialog

class CConfigureDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CConfigureDlg)

public:
	CConfigureDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CConfigureDlg();

// Dialog Data
	enum { IDD = IDD_CONFIGUREDLG };

protected:
	std::vector<UINT> m_arrSerialPortNames;
	CComboBox m_pConnection;
	CComboBox m_pSerialPortNames;
	CComboBox m_pBaudRate;
	CComboBox m_pDataBits;
	CComboBox m_pParity;
	CComboBox m_pStopBits;
	CComboBox m_pFlowControl;
	CComboBox m_pSocketType;
	CIPAddressCtrl m_pServerIP;
	CEdit m_pServerPort;
	CIPAddressCtrl m_pClientIP;
	CEdit m_pClientPort;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	virtual void OnOK();
	afx_msg void OnSelchangeConnection();
	afx_msg void OnSelchangeSocketType();

	DECLARE_MESSAGE_MAP()
};
