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

/**
 * @class CConfigureDlg
 * @brief Configuration dialog for serial port and network socket settings.
 * 
 * This dialog allows users to configure connection parameters including:
 * - Serial port settings (port name, baud rate, data bits, parity, stop bits, flow control)
 * - TCP/UDP socket settings (server/client mode, IP addresses, ports)
 * The dialog dynamically enables/disables controls based on the selected connection type.
 */

// Enable runtime type information for this dialog class
IMPLEMENT_DYNAMIC(CConfigureDlg, CDialogEx)

/**
 * @brief Constructor for CConfigureDlg.
 * 
 * Initializes the configuration dialog.
 * 
 * @param pParent Pointer to the parent window, or NULL for no parent.
 */
CConfigureDlg::CConfigureDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CConfigureDlg::IDD, pParent)
{
}

/**
 * @brief Destructor for CConfigureDlg.
 * 
 * Cleans up resources used by the configuration dialog.
 */
CConfigureDlg::~CConfigureDlg()
{
}

/**
 * @brief Exchanges data between dialog controls and member variables.
 * 
 * Maps all dialog controls (connection type, serial port settings, socket settings)
 * to their corresponding member variables for easy access.
 * 
 * @param pDX Pointer to the data exchange context.
 */
void CConfigureDlg::DoDataExchange(CDataExchange* pDX)
{
	// Call base class data exchange
	CDialogEx::DoDataExchange(pDX);

	// Map connection type controls
	DDX_Control(pDX, IDC_CONNECTION, m_pConnection);

	// Map serial port configuration controls
	DDX_Control(pDX, IDC_NAMES, m_pSerialPortNames);
	DDX_Control(pDX, IDC_BAUD_RATE, m_pBaudRate);
	DDX_Control(pDX, IDC_DATA_BITS, m_pDataBits);
	DDX_Control(pDX, IDC_PARITY, m_pParity);
	DDX_Control(pDX, IDC_STOP_BITS, m_pStopBits);
	DDX_Control(pDX, IDC_FLOW_CONTROL, m_pFlowControl);

	// Map network socket configuration controls
	DDX_Control(pDX, IDC_SOCKET_TYPE, m_pSocketType);
	DDX_Control(pDX, IDC_SERVER_IP, m_pServerIP);
	DDX_Control(pDX, IDC_SERVER_PORT, m_pServerPort);
	DDX_Control(pDX, IDC_CLIENT_IP, m_pClientIP);
	DDX_Control(pDX, IDC_CLIENT_PORT, m_pClientPort);
}

// Message map - connects Windows messages to handler functions
BEGIN_MESSAGE_MAP(CConfigureDlg, CDialogEx)
	ON_WM_DESTROY()  // Handle WM_DESTROY message for cleanup
	ON_CBN_SELCHANGE(IDC_CONNECTION, &CConfigureDlg::OnSelchangeConnection)  // Handle connection type change
	ON_CBN_SELCHANGE(IDC_SOCKET_TYPE, &CConfigureDlg::OnSelchangeSocketType)  // Handle socket type change
END_MESSAGE_MAP()

/**
 * @brief Initializes the configuration dialog.
 * 
 * Populates all combo boxes with available options and sets their values
 * based on the current application settings:
 * - Enumerates available serial ports
 * - Populates baud rates (110 to 256000)
 * - Sets data bits, parity, stop bits, and flow control options
 * - Configures socket type and network addresses
 * - Enables/disables controls based on connection type
 * 
 * @return TRUE to set focus to the first control, FALSE if focus was set manually.
 */
BOOL CConfigureDlg::OnInitDialog()
{
	CString nItemText;
	CString strSerialPortName;

	// Call base class initialization
	CDialogEx::OnInitDialog();

	// Initialize connection type combo box (Serial, TCP, UDP)
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
		m_pConnection.SetCurSel(0);  // Default to Serial Port
	}

	// Enumerate available serial ports and populate the list
	if (CEnumerateSerial::UsingGetCommPorts(m_arrSerialPortNames))
	{
		m_pSerialPortNames.ResetContent();
		// Add each discovered serial port to the combo box
		for (int nIndex = 0; nIndex < (int)m_arrSerialPortNames.size(); nIndex++)
		{
			strSerialPortName.Format(_T("COM%u"), m_arrSerialPortNames[nIndex]);
			m_pSerialPortNames.AddString(strSerialPortName);
		}

		// Try to select the previously used serial port
		bool bFound = false;
		if (!theApp.m_strSerialName.IsEmpty())
		{
			for (int nIndex = 0; nIndex < (int)m_arrSerialPortNames.size(); nIndex++)
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
				m_pSerialPortNames.SetCurSel(0);  // Default to first port if not found

		}
		else
		{
			m_pSerialPortNames.SetCurSel(0);  // Default to first port
		}
	}

	// Initialize baud rate combo box with standard baud rates
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

	// Select current baud rate based on application settings
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
			m_pBaudRate.SetCurSel(11);  // Default to 115200 baud
	}

	// Initialize data bits combo box (7 or 8 bits)
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
			m_pDataBits.SetCurSel(1);  // Default to 8 data bits
	}

	// Initialize parity combo box (None, Odd, Even, Mark, Space)
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
			m_pParity.SetCurSel(0);  // Default to no parity
	}

	// Initialize stop bits combo box (1, 1.5, or 2 stop bits)
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
			m_pStopBits.SetCurSel(0);  // Default to 1 stop bit
	}

	// Initialize flow control combo box (None, CTS/RTS, CTS/DTR, DSR/RTS, DSR/DTR, Xon/Xoff)
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
			m_pFlowControl.SetCurSel(0);  // Default to no flow control
		}

	// Initialize socket type combo box (Server or Client)
	m_pSocketType.ResetContent();
	m_pSocketType.AddString(_T("Server"));
	m_pSocketType.AddString(_T("Client"));
	if (theApp.m_nSocketType != -1)
	{
		m_pSocketType.SetCurSel(theApp.m_nSocketType);
	}
	else
	{
		m_pSocketType.SetCurSel(0);  // Default to Server mode
	}

	// Parse and set server IP address (format: "xxx.xxx.xxx.xxx")
	CString strServerIP = theApp.m_strServerIP;
	const int nServer1 = strServerIP.Find(_T('.'), 0);
	const int nServer2 = strServerIP.Find(_T('.'), nServer1 + 1);
	const int nServer3 = strServerIP.Find(_T('.'), nServer2 + 1);
	// Extract each octet and set in IP address control
	m_pServerIP.SetAddress((BYTE)_tstoi(strServerIP.Mid(0, nServer1)),
		(BYTE)_tstoi(strServerIP.Mid(nServer1 + 1, nServer2 - nServer1)),
		(BYTE)_tstoi(strServerIP.Mid(nServer2 + 1, nServer3 - nServer2)),
		(BYTE)_tstoi(strServerIP.Mid(nServer3 + 1, strServerIP.GetLength() - nServer3)));

	// Set server port number with 5-character limit
	CString strServerPort;
	strServerPort.Format(_T("%d"), theApp.m_nServerPort);
	m_pServerPort.SetWindowText(strServerPort);
	m_pServerPort.SetLimitText(5);

	// Parse and set client IP address (format: "xxx.xxx.xxx.xxx")
	CString strClientIP = theApp.m_strClientIP;
	const int nClient1 = strClientIP.Find(_T('.'), 0);
	const int nClient2 = strClientIP.Find(_T('.'), nClient1 + 1);
	const int nClient3 = strClientIP.Find(_T('.'), nClient2 + 1);
	// Extract each octet and set in IP address control
	m_pClientIP.SetAddress((BYTE)_tstoi(strClientIP.Mid(0, nClient1)),
		(BYTE)_tstoi(strClientIP.Mid(nClient1 + 1, nClient2 - nClient1)),
		(BYTE)_tstoi(strClientIP.Mid(nClient2 + 1, nClient3 - nClient2)),
		(BYTE)_tstoi(strClientIP.Mid(nClient3 + 1, strClientIP.GetLength() - nClient3)));

	// Set client port number with 5-character limit
	CString strClientPort;
	strClientPort.Format(_T("%d"), theApp.m_nClientPort);
	m_pClientPort.SetWindowText(strClientPort);
	m_pClientPort.SetLimitText(5);

	// Update control enable/disable states based on selected connection type
	OnSelchangeConnection();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

/**
 * @brief Handles cleanup when the dialog is destroyed.
 * 
 * Performs any necessary cleanup before the dialog window is destroyed.
 */
void CConfigureDlg::OnDestroy()
{
	// Call base class destroy handler
	CDialogEx::OnDestroy();
}

/**
 * @brief Handles the OK button click event.
 * 
 * Saves all configuration settings from the dialog controls back to the application:
 * - Connection type (Serial/TCP/UDP)
 * - Serial port settings (name, baud rate, data bits, parity, stop bits, flow control)
 * - Socket settings (type, server/client IP addresses and ports)
 * Then closes the dialog with IDOK result.
 */
void CConfigureDlg::OnOK()
{
	// Save connection type (Serial Port, TCP, or UDP)
	theApp.m_nConnection = m_pConnection.GetCurSel();

	// Save selected serial port name
	CString strSerialPort;
	if (m_pSerialPortNames.GetCount() > 0)
		m_pSerialPortNames.GetLBText(m_pSerialPortNames.GetCurSel(), strSerialPort);
	theApp.m_strSerialName = strSerialPort;

	// Convert selected baud rate index to Windows baud rate constant
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

	// Save data bits setting (7 or 8)
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

	// Save parity setting (None, Odd, Even, Mark, Space)
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

	// Save stop bits setting (1, 1.5, or 2)
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

	// Save flow control setting
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

	// Save socket type (Server or Client)
	theApp.m_nSocketType = m_pSocketType.GetCurSel();

	// Retrieve and save server IP address
	CString strServerIP;
	BYTE nServer1, nServer2, nServer3, nServer4;
	m_pServerIP.GetAddress(nServer1, nServer2, nServer3, nServer4);
	strServerIP.Format(_T("%d.%d.%d.%d"), nServer1, nServer2, nServer3, nServer4);
	theApp.m_strServerIP = strServerIP;

	// Retrieve and save server port number
	CString strServerPort;
	m_pServerPort.GetWindowText(strServerPort);
	theApp.m_nServerPort = _tstoi(strServerPort);

	// Retrieve and save client IP address
	CString strClientIP;
	BYTE nClient1, nClient2, nClient3, nClient4;
	m_pClientIP.GetAddress(nClient1, nClient2, nClient3, nClient4);
	strClientIP.Format(_T("%d.%d.%d.%d"), nClient1, nClient2, nClient3, nClient4);
	theApp.m_strClientIP = strClientIP;

	// Retrieve and save client port number
	CString strClientPort;
	m_pClientPort.GetWindowText(strClientPort);
	theApp.m_nClientPort = _tstoi(strClientPort);

	// Close dialog with OK result
	CDialogEx::OnOK();
}


/**
 * @brief Handles the connection type selection change event.
 * 
 * Enables or disables controls based on the selected connection type:
 * - Serial Port (index 0): Enables serial port controls, disables socket controls
 * - TCP Socket (index 1): Enables socket controls, disables serial controls
 * - UDP Socket (index 2): Enables all socket controls (both server and client)
 */
void CConfigureDlg::OnSelchangeConnection()
{
	// Enable serial port controls only when Serial Port is selected (index 0)
	m_pSerialPortNames.EnableWindow(m_pConnection.GetCurSel() == 0);
	m_pBaudRate.EnableWindow(m_pConnection.GetCurSel() == 0);
	m_pDataBits.EnableWindow(m_pConnection.GetCurSel() == 0);
	m_pParity.EnableWindow(m_pConnection.GetCurSel() == 0);
	m_pStopBits.EnableWindow(m_pConnection.GetCurSel() == 0);
	m_pFlowControl.EnableWindow(m_pConnection.GetCurSel() == 0);

	// Enable socket type only for TCP/UDP connections (index != 0)
	m_pSocketType.EnableWindow(m_pConnection.GetCurSel() != 0);

	// Enable server IP/port for: TCP Client or UDP (both server and client)
	m_pServerIP.EnableWindow(((m_pConnection.GetCurSel() != 0) && (m_pSocketType.GetCurSel() != 0)) || (m_pConnection.GetCurSel() == 2));
	m_pServerPort.EnableWindow(((m_pConnection.GetCurSel() != 0) && (m_pSocketType.GetCurSel() != 0)) || (m_pConnection.GetCurSel() == 2));

	// Enable client IP/port for: TCP Server or UDP (both server and client)
	m_pClientIP.EnableWindow(((m_pConnection.GetCurSel() != 0) && (m_pSocketType.GetCurSel() == 0)) || (m_pConnection.GetCurSel() == 2));
	m_pClientPort.EnableWindow(((m_pConnection.GetCurSel() != 0) && (m_pSocketType.GetCurSel() == 0)) || (m_pConnection.GetCurSel() == 2));
}

/**
 * @brief Handles the socket type selection change event.
 * 
 * Enables or disables IP and port controls based on the selected socket type:
 * - Server mode: Enables client IP/port controls (for incoming connections)
 * - Client mode: Enables server IP/port controls (for outgoing connections)
 * - UDP mode: Enables all IP/port controls (bidirectional communication)
 */
void CConfigureDlg::OnSelchangeSocketType()
{
	// Enable server IP/port for: TCP Client or UDP mode
	m_pServerIP.EnableWindow(((m_pConnection.GetCurSel() != 0) && (m_pSocketType.GetCurSel() != 0)) || (m_pConnection.GetCurSel() == 2));
	m_pServerPort.EnableWindow(((m_pConnection.GetCurSel() != 0) && (m_pSocketType.GetCurSel() != 0)) || (m_pConnection.GetCurSel() == 2));

	// Enable client IP/port for: TCP Server or UDP mode
	m_pClientIP.EnableWindow(((m_pConnection.GetCurSel() != 0) && (m_pSocketType.GetCurSel() == 0)) || (m_pConnection.GetCurSel() == 2));
	m_pClientPort.EnableWindow(((m_pConnection.GetCurSel() != 0) && (m_pSocketType.GetCurSel() == 0)) || (m_pConnection.GetCurSel() == 2));
}
