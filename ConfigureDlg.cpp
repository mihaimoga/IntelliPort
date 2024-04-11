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

// ConfigureDlg.cpp : implementation file
//

#include "stdafx.h"
#include "IntelliPort.h"
#include "ConfigureDlg.h"
#include "enumser.h"
#include "SerialPort.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CConfigureDlg dialog

IMPLEMENT_DYNAMIC(CConfigureDlg, CDialogEx)

CConfigureDlg::CConfigureDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CConfigureDlg::IDD, pParent)
{
}

CConfigureDlg::~CConfigureDlg()
{
}

void CConfigureDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CONNECTION, m_pConnection);
	DDX_Control(pDX, IDC_NAMES, m_pSerialPortNames);
	DDX_Control(pDX, IDC_BAUD_RATE, m_pBaudRate);
	DDX_Control(pDX, IDC_DATA_BITS, m_pDataBits);
	DDX_Control(pDX, IDC_PARITY, m_pParity);
	DDX_Control(pDX, IDC_STOP_BITS, m_pStopBits);
	DDX_Control(pDX, IDC_FLOW_CONTROL, m_pFlowControl);
	DDX_Control(pDX, IDC_SOCKET_TYPE, m_pSocketType);
	DDX_Control(pDX, IDC_SERVER_IP, m_pServerIP);
	DDX_Control(pDX, IDC_SERVER_PORT, m_pServerPort);
	DDX_Control(pDX, IDC_CLIENT_IP, m_pClientIP);
	DDX_Control(pDX, IDC_CLIENT_PORT, m_pClientPort);
}

BEGIN_MESSAGE_MAP(CConfigureDlg, CDialogEx)
	ON_WM_DESTROY()
	ON_CBN_SELCHANGE(IDC_CONNECTION, &CConfigureDlg::OnSelchangeConnection)
	ON_CBN_SELCHANGE(IDC_SOCKET_TYPE, &CConfigureDlg::OnSelchangeSocketType)
END_MESSAGE_MAP()

// CConfigureDlg message handlers
BOOL CConfigureDlg::OnInitDialog()
{
	CString nItemText;
	CString strSerialPortName;
	CDialogEx::OnInitDialog();

	m_pConnection.ResetContent();
	m_pConnection.AddString(_T("Serial Port"));
	m_pConnection.AddString(_T("TCP Socket"));
	m_pConnection.AddString(_T("UDP Socket"));
	if (theApp.m_nConnection != -1)
	{
		m_pConnection.SetCurSel(theApp.m_nConnection);
	}
	else
	{
		m_pConnection.SetCurSel(0);
	}

	if (CEnumerateSerial::UsingGetCommPorts(m_arrSerialPortNames))
	{
		m_pSerialPortNames.ResetContent();
		for (int nIndex = 0; nIndex < m_arrSerialPortNames.size(); nIndex++)
		{
			strSerialPortName.Format(_T("COM%u"), m_arrSerialPortNames[nIndex]);
			m_pSerialPortNames.AddString(strSerialPortName);
		}
		bool bFound = false;
		if (!theApp.m_strSerialName.IsEmpty())
		{
			for (int nIndex = 0; nIndex < m_arrSerialPortNames.size(); nIndex++)
			{
				m_pSerialPortNames.GetLBText(nIndex, nItemText);
				if (theApp.m_strSerialName.Compare(nItemText) == 0)
				{
					bFound = true;
					m_pSerialPortNames.SetCurSel(nIndex);
					break;
				}
			}
			if (!bFound)
				m_pSerialPortNames.SetCurSel(0);

		}
		else
		{
			m_pSerialPortNames.SetCurSel(0);
		}
	}

	m_pBaudRate.ResetContent();
	m_pBaudRate.AddString(_T("110"));
	m_pBaudRate.AddString(_T("300"));
	m_pBaudRate.AddString(_T("600"));
	m_pBaudRate.AddString(_T("1200"));
	m_pBaudRate.AddString(_T("2400"));
	m_pBaudRate.AddString(_T("4800"));
	m_pBaudRate.AddString(_T("9600"));
	m_pBaudRate.AddString(_T("14400"));
	m_pBaudRate.AddString(_T("19200"));
	m_pBaudRate.AddString(_T("38400"));
	m_pBaudRate.AddString(_T("57600"));
	m_pBaudRate.AddString(_T("115200"));
	m_pBaudRate.AddString(_T("128000"));
	m_pBaudRate.AddString(_T("256000"));
	switch (theApp.m_nBaudRate)
	{
		case CBR_110:
		{
			m_pBaudRate.SetCurSel(0);
			break;
		}
		case CBR_300:
		{
			m_pBaudRate.SetCurSel(1);
			break;
		}
		case CBR_600:
		{
			m_pBaudRate.SetCurSel(2);
			break;
		}
		case CBR_1200:
		{
			m_pBaudRate.SetCurSel(3);
			break;
		}
		case CBR_2400:
		{
			m_pBaudRate.SetCurSel(4);
			break;
		}
		case CBR_4800:
		{
			m_pBaudRate.SetCurSel(5);
			break;
		}
		case CBR_9600:
		{
			m_pBaudRate.SetCurSel(6);
			break;
		}
		case CBR_14400:
		{
			m_pBaudRate.SetCurSel(7);
			break;
		}
		case CBR_19200:
		{
			m_pBaudRate.SetCurSel(8);
			break;
		}
		case CBR_38400:
		{
			m_pBaudRate.SetCurSel(9);
			break;
		}
		case CBR_57600:
		{
			m_pBaudRate.SetCurSel(0);
			break;
		}
		case CBR_115200:
		{
			m_pBaudRate.SetCurSel(11);
			break;
		}
		case CBR_128000:
		{
			m_pBaudRate.SetCurSel(12);
			break;
		}
		case CBR_256000:
		{
			m_pBaudRate.SetCurSel(13);
			break;
		}
		default:
			m_pBaudRate.SetCurSel(11);
	}

	m_pDataBits.ResetContent();
	m_pDataBits.AddString(_T("7"));
	m_pDataBits.AddString(_T("8"));
	switch (theApp.m_nDataBits)
	{
		case 7:
		{
			m_pDataBits.SetCurSel(0);
			break;
		}
		case 8:
		{
			m_pDataBits.SetCurSel(1);
			break;
		}
		default:
			m_pDataBits.SetCurSel(1);
	}

	m_pParity.ResetContent();
	m_pParity.AddString(_T("None"));
	m_pParity.AddString(_T("Odd"));
	m_pParity.AddString(_T("Even"));
	m_pParity.AddString(_T("Mark"));
	m_pParity.AddString(_T("Space"));
	switch (theApp.m_nParity)
	{
		case (int)CSerialPort::Parity::NoParity:
		{
			m_pParity.SetCurSel(0);
			break;
		}
		case (int)CSerialPort::Parity::OddParity:
		{
			m_pParity.SetCurSel(1);
			break;
		}
		case (int)CSerialPort::Parity::EvenParity:
		{
			m_pParity.SetCurSel(2);
			break;
		}
		case (int)CSerialPort::Parity::MarkParity:
		{
			m_pParity.SetCurSel(3);
			break;
		}
		case (int)CSerialPort::Parity::SpaceParity:
		{
			m_pParity.SetCurSel(4);
			break;
		}
		default:
			m_pParity.SetCurSel(0);
	}

	m_pStopBits.ResetContent();
	m_pStopBits.AddString(_T("1"));
	m_pStopBits.AddString(_T("1.5"));
	m_pStopBits.AddString(_T("2"));
	switch (theApp.m_nStopBits)
	{
		case (int)CSerialPort::StopBits::OneStopBit:
		{
			m_pStopBits.SetCurSel(0);
			break;
		}
		case (int)CSerialPort::StopBits::OnePointFiveStopBits:
		{
			m_pStopBits.SetCurSel(1);
			break;
		}
		case (int)CSerialPort::StopBits::TwoStopBits:
		{
			m_pStopBits.SetCurSel(2);
			break;
		}
		default:
			m_pStopBits.SetCurSel(0);
	}

	m_pFlowControl.ResetContent();
	m_pFlowControl.AddString(_T("None"));
	m_pFlowControl.AddString(_T("CTS/RTS"));
	m_pFlowControl.AddString(_T("CTS/DTR"));
	m_pFlowControl.AddString(_T("DSR/RTS"));
	m_pFlowControl.AddString(_T("DSR/DTR"));
	m_pFlowControl.AddString(_T("Xon/Xoff"));
	switch (theApp.m_nFlowControl)
	{
		case (int)CSerialPort::FlowControl::NoFlowControl:
		{
			m_pFlowControl.SetCurSel(0);
			break;
		}
		case (int)CSerialPort::FlowControl::CtsRtsFlowControl:
		{
			m_pFlowControl.SetCurSel(1);
			break;
		}
		case (int)CSerialPort::FlowControl::CtsDtrFlowControl:
		{
			m_pFlowControl.SetCurSel(2);
			break;
		}
		case (int)CSerialPort::FlowControl::DsrRtsFlowControl:
		{
			m_pFlowControl.SetCurSel(3);
			break;
		}
		case (int)CSerialPort::FlowControl::DsrDtrFlowControl:
		{
			m_pFlowControl.SetCurSel(4);
			break;
		}
		case (int)CSerialPort::FlowControl::XonXoffFlowControl:
		{
			m_pFlowControl.SetCurSel(5);
			break;
		}
		default:
			m_pFlowControl.SetCurSel(0);
	}

	m_pSocketType.ResetContent();
	m_pSocketType.AddString(_T("Server"));
	m_pSocketType.AddString(_T("Client"));
	if (theApp.m_nSocketType != -1)
	{
		m_pSocketType.SetCurSel(theApp.m_nSocketType);
	}
	else
	{
		m_pSocketType.SetCurSel(0);
	}

	CString strServerIP = theApp.m_strServerIP;
	const int nServer1 = strServerIP.Find(_T('.'), 0);
	const int nServer2 = strServerIP.Find(_T('.'), nServer1 + 1);
	const int nServer3 = strServerIP.Find(_T('.'), nServer2 + 1);
	m_pServerIP.SetAddress((BYTE) _tstoi(strServerIP.Mid(0, nServer1)),
		(BYTE) _tstoi(strServerIP.Mid(nServer1 + 1, nServer2 - nServer1)),
		(BYTE) _tstoi(strServerIP.Mid(nServer2 + 1, nServer3 - nServer2)),
		(BYTE) _tstoi(strServerIP.Mid(nServer3 + 1, strServerIP.GetLength() - nServer3)));

	CString strServerPort;
	strServerPort.Format(_T("%d"), theApp.m_nServerPort);
	m_pServerPort.SetWindowText(strServerPort);
	m_pServerPort.SetLimitText(5);

	CString strClientIP = theApp.m_strClientIP;
	const int nClient1 = strClientIP.Find(_T('.'), 0);
	const int nClient2 = strClientIP.Find(_T('.'), nClient1 + 1);
	const int nClient3 = strClientIP.Find(_T('.'), nClient2 + 1);
	m_pClientIP.SetAddress((BYTE) _tstoi(strClientIP.Mid(0, nClient1)),
		(BYTE) _tstoi(strClientIP.Mid(nClient1 + 1, nClient2 - nClient1)),
		(BYTE) _tstoi(strClientIP.Mid(nClient2 + 1, nClient3 - nClient2)),
		(BYTE) _tstoi(strClientIP.Mid(nClient3 + 1, strClientIP.GetLength() - nClient3)));

	CString strClientPort;
	strClientPort.Format(_T("%d"), theApp.m_nClientPort);
	m_pClientPort.SetWindowText(strClientPort);
	m_pClientPort.SetLimitText(5);

	OnSelchangeConnection();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CConfigureDlg::OnDestroy()
{
	CDialogEx::OnDestroy();
}

void CConfigureDlg::OnOK()
{
	theApp.m_nConnection = m_pConnection.GetCurSel();

	CString strSerialPort;
	if (m_pSerialPortNames.GetCount() > 0)
		m_pSerialPortNames.GetLBText(m_pSerialPortNames.GetCurSel(), strSerialPort);
	theApp.m_strSerialName = strSerialPort;

	switch (m_pBaudRate.GetCurSel())
	{
		case 0:
		{
			theApp.m_nBaudRate = CBR_110;
			break;
		}
		case 1:
		{
			theApp.m_nBaudRate = CBR_300;
			break;
		}
		case 2:
		{
			theApp.m_nBaudRate = CBR_600;
			break;
		}
		case 3:
		{
			theApp.m_nBaudRate = CBR_1200;
			break;
		}
		case 4:
		{
			theApp.m_nBaudRate = CBR_2400;
			break;
		}
		case 5:
		{
			theApp.m_nBaudRate = CBR_4800;
			break;
		}
		case 6:
		{
			theApp.m_nBaudRate = CBR_9600;
			break;
		}
		case 7:
		{
			theApp.m_nBaudRate = CBR_14400;
			break;
		}
		case 8:
		{
			theApp.m_nBaudRate = CBR_19200;
			break;
		}
		case 9:
		{
			theApp.m_nBaudRate = CBR_38400;
			break;
		}
		case 10:
		{
			theApp.m_nBaudRate = CBR_57600;
			break;
		}
		case 11:
		{
			theApp.m_nBaudRate = CBR_115200;
			break;
		}
		case 12:
		{
			theApp.m_nBaudRate = CBR_128000;
			break;
		}
		case 13:
		{
			theApp.m_nBaudRate = CBR_256000;
			break;
		}
	}

	switch (m_pDataBits.GetCurSel())
	{
		case 0:
		{
			theApp.m_nDataBits = 7;
			break;
		}
		case 1:
		{
			theApp.m_nDataBits = 8;
			break;
		}
	}

	switch (m_pParity.GetCurSel())
	{
		case 0:
		{
			theApp.m_nParity = (int)CSerialPort::Parity::NoParity;
			break;
		}
		case 1:
		{
			theApp.m_nParity = (int)CSerialPort::Parity::OddParity;
			break;
		}
		case 2:
		{
			theApp.m_nParity = (int)CSerialPort::Parity::EvenParity;
			break;
		}
		case 3:
		{
			theApp.m_nParity = (int)CSerialPort::Parity::MarkParity;
			break;
		}
		case 4:
		{
			theApp.m_nParity = (int)CSerialPort::Parity::SpaceParity;
			break;
		}
	}

	switch (m_pStopBits.GetCurSel())
	{
		case 0:
		{
			theApp.m_nStopBits = (int)CSerialPort::StopBits::OneStopBit;
			break;
		}
		case 1:
		{
			theApp.m_nStopBits = (int)CSerialPort::StopBits::OnePointFiveStopBits;
			break;
		}
		case 2:
		{
			theApp.m_nStopBits = (int)CSerialPort::StopBits::TwoStopBits;
			break;
		}
	}

	switch (m_pFlowControl.GetCurSel())
	{
		case 0:
		{
			theApp.m_nFlowControl = (int)CSerialPort::FlowControl::NoFlowControl;
			break;
		}
		case 1:
		{
			theApp.m_nFlowControl = (int)CSerialPort::FlowControl::CtsRtsFlowControl;
			break;
		}
		case 2:
		{
			theApp.m_nFlowControl = (int)CSerialPort::FlowControl::CtsDtrFlowControl;
			break;
		}
		case 3:
		{
			theApp.m_nFlowControl = (int)CSerialPort::FlowControl::DsrRtsFlowControl;
			break;
		}
		case 4:
		{
			theApp.m_nFlowControl = (int)CSerialPort::FlowControl::DsrDtrFlowControl;
			break;
		}
		case 5:
		{
			theApp.m_nFlowControl = (int)CSerialPort::FlowControl::XonXoffFlowControl;
			break;
		}
	}

	theApp.m_nSocketType = m_pSocketType.GetCurSel();

	CString strServerIP;
	BYTE nServer1, nServer2, nServer3, nServer4;
	m_pServerIP.GetAddress(nServer1, nServer2, nServer3, nServer4);
	strServerIP.Format(_T("%d.%d.%d.%d"), nServer1, nServer2, nServer3, nServer4);
	theApp.m_strServerIP = strServerIP;

	CString strServerPort;
	m_pServerPort.GetWindowText(strServerPort);
	theApp.m_nServerPort = _tstoi(strServerPort);

	CString strClientIP;
	BYTE nClient1, nClient2, nClient3, nClient4;
	m_pClientIP.GetAddress(nClient1, nClient2, nClient3, nClient4);
	strClientIP.Format(_T("%d.%d.%d.%d"), nClient1, nClient2, nClient3, nClient4);
	theApp.m_strClientIP = strClientIP;

	CString strClientPort;
	m_pClientPort.GetWindowText(strClientPort);
	theApp.m_nClientPort = _tstoi(strClientPort);

	CDialogEx::OnOK();
}


void CConfigureDlg::OnSelchangeConnection()
{
	m_pSerialPortNames.EnableWindow(m_pConnection.GetCurSel() == 0);
	m_pBaudRate.EnableWindow(m_pConnection.GetCurSel() == 0);
	m_pDataBits.EnableWindow(m_pConnection.GetCurSel() == 0);
	m_pParity.EnableWindow(m_pConnection.GetCurSel() == 0);
	m_pStopBits.EnableWindow(m_pConnection.GetCurSel() == 0);
	m_pFlowControl.EnableWindow(m_pConnection.GetCurSel() == 0);

	m_pSocketType.EnableWindow(m_pConnection.GetCurSel() != 0);
	m_pServerIP.EnableWindow(((m_pConnection.GetCurSel() != 0) && (m_pSocketType.GetCurSel() != 0)) || (m_pConnection.GetCurSel() == 2));
	m_pServerPort.EnableWindow(((m_pConnection.GetCurSel() != 0) && (m_pSocketType.GetCurSel() != 0)) || (m_pConnection.GetCurSel() == 2));
	m_pClientIP.EnableWindow(((m_pConnection.GetCurSel() != 0) && (m_pSocketType.GetCurSel() == 0)) || (m_pConnection.GetCurSel() == 2));
	m_pClientPort.EnableWindow(((m_pConnection.GetCurSel() != 0) && (m_pSocketType.GetCurSel() == 0)) || (m_pConnection.GetCurSel() == 2));
}

void CConfigureDlg::OnSelchangeSocketType()
{
	m_pServerIP.EnableWindow(((m_pConnection.GetCurSel() != 0) && (m_pSocketType.GetCurSel() != 0)) || (m_pConnection.GetCurSel() == 2));
	m_pServerPort.EnableWindow(((m_pConnection.GetCurSel() != 0) && (m_pSocketType.GetCurSel() != 0)) || (m_pConnection.GetCurSel() == 2));
	m_pClientIP.EnableWindow(((m_pConnection.GetCurSel() != 0) && (m_pSocketType.GetCurSel() == 0)) || (m_pConnection.GetCurSel() == 2));
	m_pClientPort.EnableWindow(((m_pConnection.GetCurSel() != 0) && (m_pSocketType.GetCurSel() == 0)) || (m_pConnection.GetCurSel() == 2));
}
