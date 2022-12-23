/*
Module : SerialPort.h
Purpose: Interface for a set of C++ wrapper classes for serial ports
Created: PJN / 31-05-1999
History: PJN / 03-06-1999 1. Fixed problem with code using CancelIo which does not exist on 95.
                          2. Fixed leaks which can occur in sample app when an exception is thrown
         PJN / 16-06-1999 1. Fixed a bug whereby CString::ReleaseBuffer was not being called in 
                             CSerialException::GetErrorMessage
         PJN / 29-09-1999 1. Fixed a simple copy and paste bug in CSerialPort::SetDTR
         PJN / 08-05-2000 1. Fixed an unreferrenced variable in CSerialPort::GetOverlappedResult in VC 6
         PJN / 10-12-2000 1. Made class destructor virtual
         PJN / 15-01-2001 1. Attach method now also allows you to specify whether the serial port
                          is being attached to in overlapped mode
                          2. Removed some ASSERTs which were unnecessary in some of the functions
                          3. Updated the Read method which uses OVERLAPPED IO to also return the bytes
                          read. This allows calls to WriteFile with a zeroed overlapped structure (This
                          is required when dealing with TAPI and serial communications)
                          4. Now includes copyright message in the source code and documentation.
         PJN / 24-03-2001 1. Added a BytesWaiting method
         PJN / 04-04-2001 1. Provided an overriden version of BytesWaiting which specifies a timeout
         PJN / 23-04-2001 1. Fixed a memory leak in DataWaiting method
         PJN / 01-05-2002 1. Fixed a problem in Open method which was failing to initialize the DCB 
                          structure incorrectly, when calling GetState. Thanks to Ben Newson for this fix.
         PJN / 29-05-2002 1. Fixed an problem where the GetProcAddress for CancelIO was using the
                          wrong calling convention
         PJN / 07-08-2002 1. Changed the declaration of CSerialPort::WaitEvent to be consistent with the
                          rest of the methods in CSerialPort which can operate in "OVERLAPPED" mode. A note
                          about the usage of this: If the method succeeds then the overlapped operation
                          has completed synchronously and there is no need to do a WaitForSingle/MultipleObjects.
                          If any other unexpected error occurs then a CSerialException will be thrown. See
                          the implementation of the CSerialPort::DataWaiting which has been rewritten to use
                          this new design pattern. Thanks to Serhiy Pavlov for spotting this inconsistency.
         PJN / 20-09-2002 1. Addition of an additional ASSERT in the internal _OnCompletion function.
                          2. Addition of an optional out parameter to the Write method which operates in 
                          overlapped mode. Thanks to Kevin Pinkerton for this addition.
         PJN / 10-04-2006 1. Updated copyright details.
                          2. Addition of a CSERIALPORT_EXT_CLASS and CSERIALPORT_EXT_API macros which makes 
                          the class easier to use in an extension dll.
                          3. Removed derivation of CSerialPort from CObject as it was not really needed.
                          4. Fixed a number of level 4 warnings in the sample app.
                          5. Reworked the overlapped IO methods to expose the LPOVERLAPPED structure to client 
                          code.
                          6. Updated the documentation to use the same style as the web site.
                          7. Did a spell check of the HTML documentation.
                          8. Updated the documentation on possible blocking in Read/Ex function. Thanks to 
                          D Kerrison for reporting this issue.
                          9. Fixed a minor issue in the sample app when the code is compiled using /Wp64
         PJN / 02-06-2006 1. Removed the bOverlapped as a member variable from the class. There was no 
                          real need for this setting, since the SDK functions will perform their own 
                          checking of how overlapped operations should
                          2. Fixed a bug in GetOverlappedResult where the code incorrectly checking against
                          the error ERROR_IO_PENDING instead of ERROR_IO_INCOMPLETE. Thanks to Sasho Darmonski
                          for reporting this bug.
                          3. Reviewed all TRACE statements for correctness.
         PJN / 05-06-2006 1. Fixed an issue with the creation of the internal event object. It was incorrectly
                          being created as an auto-reset event object instead of a manual reset event object.
                          Thanks to Sasho Darmonski for reporting this issue.
         PJN / 24-06-2006 1. Fixed some typos in the history list. Thanks to Simon Wong for reporting this.
                          2. Made the class which handles the construction of function pointers at runtime a
                          member variable of CSerialPort
                          3. Made AfxThrowSerialPortException part of the CSerialPort class. Thanks to Simon Wong
                          for reporting this.
                          4. Removed the unnecessary CSerialException destructor. Thanks to Simon Wong for 
                          reporting this.
                          5. Fixed a minor error in the TRACE text in CSerialPort::SetDefaultConfig. Again thanks
                          to Simon Wong for reporting this. 
                          6. Code now uses new C++ style casts rather than old style C casts where necessary. 
                          Again thanks to Simon Wong for reporting this.
                          7. CSerialException::GetErrorMessage now uses the strsafe functions. This does mean 
                          that the code now requires the Platform SDK if compiled using VC 6.
         PJN / 25-06-2006 1. Combined the functionality of the CSerialPortData class into the main CSerialPort class.
                          2. Renamed AfxThrowSerialPortException to ThrowSerialPortException and made the method
                          public.
         PJN / 05-11-2006 1. Minor update to stdafx.h of sample app to avoid compiler warnings in VC 2005.
                          2. Reverted the use of the strsafe.h header file. Instead now the code uses the VC 2005
                          Safe CRT and if this is not available, then we fail back to the standard CRT.
         PJN / 25-01-2007 1. Minor update to remove strsafe.h from stdafx.h of the sample app.
                          2. Updated copyright details.
         PJN / 24-12-2007 1. CSerialException::GetErrorMessage now uses the FORMAT_MESSAGE_IGNORE_INSERTS flag. For more information please see Raymond
                          Chen's blog at http://blogs.msdn.com/oldnewthing/archive/2007/11/28/6564257.aspx. Thanks to Alexey Kuznetsov for reporting this
                          issue.
                          2. Simplified the code in CSerialException::GetErrorMessage somewhat.
                          3. Optimized the CSerialException constructor code.
                          4. Code now uses newer C++ style casts instead of C style casts.
                          5. Reviewed and updated all the TRACE logging in the module
                          6. Replaced all calls to ZeroMemory with memset
         PJN / 30-12-2007 1. Updated the sample app to clean compile on VC 2008
                          2. CSerialException::GetErrorMessage now uses Checked::tcsncpy_s if compiled using VC 2005 or later.
         PJN / 18-05-2008 1. Updated copyright details.
                          2. Changed the actual values for Parity enum so that they are consistent with the Parity define values in the Windows SDK 
                          header file WinBase.h. This avoids the potential issue where you use the CSerialPort enum parity values in a call to the
                          raw Win32 API calls. Thanks to Robert Krueger for reporting this issue.
         PJN / 21-06-2008 1. Code now compiles cleanly using Code Analysis (/analyze)
                          2. Updated code to compile correctly using _ATL_CSTRING_EXPLICIT_CONSTRUCTORS define
                          3. The code now only supports VC 2005 or later. 
                          4. CSerialPort::Read, Write, GetOverlappedResult & WaitEvent now throw an exception irrespective of whether the last error
                          is ERROR_IO_PENDING or not
                          5. Replaced all calls to ZeroMemory with memset
         PJN / 04-07-2008 1. Provided a version of the Open method which takes a string instead of a numeric port number value. This allows the code
                          to support some virtual serial port packages which do not use device names of the form "COM%d". Thanks to David Balazic for 
                          suggesting this addition.
         PJN / 25-01-2012 1. Updated copyright details.
                          2. Updated sample app and class to compile cleanly on VC 2010 and later.
         PJN / 28-02-2015 1. Updated sample project settings to more modern default values.
                          2. Updated copyright details.
                          3. Reworked the CSerialPort and CSerialPortException classes to optionally compile without MFC. By default 
                          the classes now use STL classes and idioms but if you define CSERIALPORT_MFC_EXTENSIONS the classes will 
                          revert back to the MFC behaviour.
                          4. Remove logic to use GetProcAddress to access CancelIO functionality.
                          5. Updated the code to clean compile on VC 2013
                          6. Added SAL annotations to all the code
                          7. Addition of a GetDefaultConfig method which takes a string
                          8. Addition of a SetDefaultConfig method which takes a string
         PJN / 26-04-2015 1. Removed unnecessary inclusion of WinError.h
                          2. Removed the CSerialPort::DataWaiting method as it depends on the port being open in overlapped mode. Instead client code
                          can simply call CSerialPort::WaitEvent directly themselves. Removing this method also means that the CSerialPort::m_hEvent
                          handle has not also been removed.
                          3. The CSerialPort::WriteEx method has been reworked to expose all the parameters of the underlying WriteFileEx API. This
                          rework also fixes a memory leak in WriteEx which can sometimes occur. This reworks also means that the 
                          CSerialPort::_OnCompletion and CSerialPort::_OnCompletion methods have been removed. Thanks to Yufeng Huang for reporting 
                          this issue.
                          4. The CSerialPort::ReadEx method has been reworked to expose all the parameters of the underlying ReadFileEx API. This
                          rework also fixes a memory leak in ReadEx which can sometimes occur. This reworks also means that the 
                          CSerialPort::_OnCompletion and CSerialPort::_OnCompletion methods have been removed. Thanks to Yufeng Huang for reporting 
                          this issue.
         PJN / 17-12-2015 1. Verified the code compiles cleanly on VC 2015.
         PJN / 22-05-2016 1. Updated copyright details.
                          2. Fixed some typos in SerialPort.h, SerialPort.cpp and SerialPort.htm where CSERIALPORT_MFC_EXTENSTIONS was being used 
                          instead of CSERIALPORT_MFC_EXTENSIONS. Thanks to Nicholas Buse for reporting this issue.
         PJN / 16-08-2017 1. Updated copyright details.
                          2. Updated declarations of CSerialException::GetErrorMessage to be consistent with CException base class implementations
                          3. Fixed up the SAL annotations on CSerialPort::GetConfig. Thanks to Wang Jinhai for reporting this issue.
         PJN / 17-12-2017 1. Replaced NULL throughout the codebase with nullptr. This means that the minimum requirement for the framework is now 
                          VC 2010.
                          2. Updated the code to compile cleanly when _ATL_NO_AUTOMATIC_NAMESPACE is defined.
                          3. Provided a CSerialPort::CancelIoEx method which encapsulates the CancelIoEx API. Thanks to Victor Derks for this 
                          suggestion.
                          4. Fixed code in CSerialPort::GetDefaultConfig as it does not have a return value. Thanks to Victor Derks for reporting
                          this bug.
                          5. Fixed code in CSerialPort::SetDefaultConfig as it does not have a return value. Thanks to Victor Derks for reporting
                          this bug.
                          6. The sal header file is now included before the fallback SAL macros are defined in SerialPort.h. Thanks to Victor Derks 
                          for reporting this issue.
                          7. Reworked the code to now be a header only implementation.
                          8. Provided a new CSerialPort2 class which is a non exception based version of CSerialPort
                          9. Provided methods in CSerialPort & CSerialPort2 which encapsulate the GetOverlappedResultEx API.
                          10. Provided a new Open method in CSerialPort & CSerialPort2 which just opens the port without explicit configuration.
                          11. Provided a new Open method in CSerialPort & CSerialPort2 which encapsulates the OpenCommPort API.
         PJN / 01-07-2018 1. Updated copyright details.
                          2. Fixed a number of C++ core guidelines compiler warnings. These changes mean that the code
                          will now only compile on VC 2017 or later.
         PJN / 16-09-2018 1. Fixed a number of compiler warnings when using VS 2017 15.8.4
         PJN / 21-12-2018 1. Fixed issues in CSerialPort2::Open where sometimes the function would return FALSE, but would leave the serial port in 
                          an open state. Thanks to "Guan" for reporting this issue.
         PJN / 29-01-2019 1. Updated copyright details.
                          2. Fixed a bug in the two CSerialPort::Open methods where they were incorrectly defined as having a return value of
                          BOOL instead of the correct return value of void. Thanks to Boris Usievich for reporting this bug.
         PJN / 21-04-2019 1. Updated the sample app to clean compile on VC 2019
                          2. Removed the code path supported by the non defunct CSERIALPORT_MFC_EXTENSIONS enum
         PJN / 10-09-2019 1. Replaced enum with enum class throughout the code
                          2. Fixed a number of compiler warnings when the code is compiled with VS 2019 Preview
         PJN / 07-11-2019 1. Updated initialization of various structs to use C++ 11 list initialization
         PJN / 12-04-2020 1. Updated copyright details.
                          2. Fixed more Clang-Tidy static code analysis warnings in the code.

Copyright (c) 1999 - 2020 by PJ Naughter. (Web: www.naughter.com, Email: pjna@naughter.com)

All rights reserved.

Copyright / Usage Details:

You are allowed to include the source code in any product (commercial, shareware, freeware or otherwise) 
when your product is released in binary form. You are allowed to modify the source code in any way you want 
except you cannot modify the copyright details at the top of each module. If you want to distribute source 
code with your application, then you are only allowed to distribute versions released by the author. This is 
to maintain a single distribution point for the source code. 

*/


///////////////////// Macros / Structs etc ////////////////////////////////////

#pragma once

#ifndef __SERIALPORT_H__
#define __SERIALPORT_H__

#ifndef CSERIALPORT_EXT_CLASS
#define CSERIALPORT_EXT_CLASS
#endif //#ifndef CSERIALPORT_EXT_CLASS


////////////////////////// Includes ///////////////////////////////////////////

#include <exception>

#ifndef __ATLBASE_H__
#pragma message("To avoid this message, please put atlbase.h in your pre compiled header (normally stdafx.h)")
#include <atlbase.h>
#endif //#ifndef __ATLBASE_H__

#ifndef __ATLSTR_H__
#pragma message("To avoid this message, please put atlstr.h in your pre compiled header (normally stdafx.h)")
#include <atlstr.h>
#endif //#ifndef __ATLSTR_H__



/////////////////////////// Classes ///////////////////////////////////////////

class CSERIALPORT_EXT_CLASS CSerialException : public std::exception
{
public:
	//Constructors / Destructors
	CSerialException(_In_ DWORD dwError) noexcept : m_dwError(dwError)
	{
	}

	//Methods
#pragma warning(suppress: 26429)
	virtual BOOL GetErrorMessage2(_Out_writes_z_(nMaxError) LPTSTR lpszError, _In_ UINT nMaxError, _Out_opt_ PUINT pnHelpContext = nullptr) const
	{
		//Validate our parameters
#pragma warning(suppress: 26477)
		ATLASSERT(lpszError != nullptr);

		if (pnHelpContext != nullptr)
			*pnHelpContext = 0;

		//What will be the return value from this function (assume the worst)
		BOOL bSuccess = FALSE;
		LPTSTR lpBuffer = nullptr;
		const DWORD dwReturn = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			nullptr, m_dwError, MAKELANGID(LANG_NEUTRAL, SUBLANG_SYS_DEFAULT),
#pragma warning(suppress: 26490)
			reinterpret_cast<LPTSTR>(&lpBuffer), 0, nullptr);
		if (dwReturn == 0)
			*lpszError = _T('\0');
		else
		{
			bSuccess = TRUE;
			ATL::Checked::tcsncpy_s(lpszError, nMaxError, lpBuffer, _TRUNCATE);
			LocalFree(lpBuffer);
		}

		return bSuccess;
	}

	//Data members
	DWORD m_dwError;
#pragma warning(suppress: 26495)
};


//Non exception based version of the class
class CSERIALPORT_EXT_CLASS CSerialPort2
{
public:
	//Enums
	enum class FlowControl
	{
		NoFlowControl,
		CtsRtsFlowControl,
		CtsDtrFlowControl,
		DsrRtsFlowControl,
		DsrDtrFlowControl,
		XonXoffFlowControl
	};

	enum class Parity
	{
		NoParity = 0,
		OddParity = 1,
		EvenParity = 2,
		MarkParity = 3,
		SpaceParity = 4
	};

	enum class StopBits
	{
		OneStopBit,
		OnePointFiveStopBits,
		TwoStopBits
	};

	//Constructors / Destructors
	CSerialPort2() noexcept : m_hComm(INVALID_HANDLE_VALUE)
	{
	}

	CSerialPort2(_In_ const CSerialPort2&) = delete;

	CSerialPort2(CSerialPort2&& port) noexcept : m_hComm(INVALID_HANDLE_VALUE)
	{
		Attach(port.Detach());
	}

	virtual ~CSerialPort2()
	{
		Close();
	}

	//General Methods
	CSerialPort2& operator=(_In_ const CSerialPort2&) = delete;

	CSerialPort2& operator=(CSerialPort2&& port) noexcept
	{
		Attach(port.Detach());
		return *this;
	}

	__if_exists(OpenCommPort)
	{
		BOOL Open(_In_ ULONG uPortNumber, _In_ DWORD dwDesiredAccess, _In_ DWORD dwFlagsAndAttributes) noexcept
		{
			Close(); //In case we are already open

			//Call CreateFile to open the comms port
			m_hComm = OpenCommPort(uPortNumber, dwDesiredAccess, dwFlagsAndAttributes);
			return (m_hComm != INVALID_HANDLE_VALUE);
		}
	}

	BOOL Open(_In_z_ LPCTSTR pszPort, _In_ BOOL bOverlapped = FALSE) noexcept
	{
		Close(); //In case we are already open

		//Call CreateFile to open the comms port
		m_hComm = CreateFile(pszPort, GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, bOverlapped ? FILE_FLAG_OVERLAPPED : 0, nullptr);
		return (m_hComm != INVALID_HANDLE_VALUE);
	}

	BOOL Open(_In_ int nPort, _In_ DWORD dwBaud = 9600, _In_ Parity parity = Parity::NoParity, _In_ BYTE DataBits = 8,
		_In_ StopBits stopBits = StopBits::OneStopBit, _In_ FlowControl fc = FlowControl::NoFlowControl, _In_ BOOL bOverlapped = FALSE)
	{
		//Form the string version of the port number
		ATL::CAtlString sPort;
		sPort.Format(_T("\\\\.\\COM%d"), nPort);

		//Delegate the work to the other version of Open
		return Open(sPort, dwBaud, parity, DataBits, stopBits, fc, bOverlapped);
	}

	BOOL Open(_In_z_ LPCTSTR pszPort, _In_ DWORD dwBaud = 9600, _In_ Parity parity = Parity::NoParity, _In_ BYTE DataBits = 8,
		_In_ StopBits stopBits = StopBits::OneStopBit, _In_ FlowControl fc = FlowControl::NoFlowControl, _In_ BOOL bOverlapped = FALSE) noexcept
	{
		//Delegate to the other version of this method
		if (!Open(pszPort, bOverlapped))
			return FALSE;

		//Get the current state prior to changing it
		DCB dcb{};
		dcb.DCBlength = sizeof(DCB);
		if (!GetState(dcb))
		{
			const DWORD dwLastError = GetLastError();
			Close();
			SetLastError(dwLastError);
			return FALSE;
		}

		//Setup the baud rate
		dcb.BaudRate = dwBaud;

		//Setup the Parity
		switch (parity)
		{
		case Parity::EvenParity:
		{
			dcb.Parity = EVENPARITY;
			break;
		}
		case Parity::MarkParity:
		{
			dcb.Parity = MARKPARITY;
			break;
		}
		case Parity::NoParity:
		{
			dcb.Parity = NOPARITY;
			break;
		}
		case Parity::OddParity:
		{
			dcb.Parity = ODDPARITY;
			break;
		}
		case Parity::SpaceParity:
		{
			dcb.Parity = SPACEPARITY;
			break;
		}
		default:
		{
#pragma warning(suppress: 26477)
			ATLASSERT(FALSE);
			break;
		}
		}

		//Setup the data bits
		dcb.ByteSize = DataBits;

		//Setup the stop bits
		switch (stopBits)
		{
		case StopBits::OneStopBit:
		{
			dcb.StopBits = ONESTOPBIT;
			break;
		}
		case StopBits::OnePointFiveStopBits:
		{
			dcb.StopBits = ONE5STOPBITS;
			break;
		}
		case StopBits::TwoStopBits:
		{
			dcb.StopBits = TWOSTOPBITS;
			break;
		}
		default:
		{
#pragma warning(suppress: 26477)
			ATLASSERT(FALSE);
			break;
		}
		}

		//Setup the flow control
		dcb.fDsrSensitivity = FALSE;
		switch (fc)
		{
		case FlowControl::NoFlowControl:
		{
			dcb.fOutxCtsFlow = FALSE;
			dcb.fOutxDsrFlow = FALSE;
			dcb.fOutX = FALSE;
			dcb.fInX = FALSE;
			break;
		}
		case FlowControl::CtsRtsFlowControl:
		{
			dcb.fOutxCtsFlow = TRUE;
			dcb.fOutxDsrFlow = FALSE;
			dcb.fRtsControl = RTS_CONTROL_HANDSHAKE;
			dcb.fOutX = FALSE;
			dcb.fInX = FALSE;
			break;
		}
		case FlowControl::CtsDtrFlowControl:
		{
			dcb.fOutxCtsFlow = TRUE;
			dcb.fOutxDsrFlow = FALSE;
			dcb.fDtrControl = DTR_CONTROL_HANDSHAKE;
			dcb.fOutX = FALSE;
			dcb.fInX = FALSE;
			break;
		}
		case FlowControl::DsrRtsFlowControl:
		{
			dcb.fOutxCtsFlow = FALSE;
			dcb.fOutxDsrFlow = TRUE;
			dcb.fRtsControl = RTS_CONTROL_HANDSHAKE;
			dcb.fOutX = FALSE;
			dcb.fInX = FALSE;
			break;
		}
		case FlowControl::DsrDtrFlowControl:
		{
			dcb.fOutxCtsFlow = FALSE;
			dcb.fOutxDsrFlow = TRUE;
			dcb.fDtrControl = DTR_CONTROL_HANDSHAKE;
			dcb.fOutX = FALSE;
			dcb.fInX = FALSE;
			break;
		}
		case FlowControl::XonXoffFlowControl:
		{
			dcb.fOutxCtsFlow = FALSE;
			dcb.fOutxDsrFlow = FALSE;
			dcb.fOutX = TRUE;
			dcb.fInX = TRUE;
			dcb.XonChar = 0x11;
			dcb.XoffChar = 0x13;
			dcb.XoffLim = 100;
			dcb.XonLim = 100;
			break;
		}
		default:
		{
#pragma warning(suppress: 26477)
			ATLASSERT(FALSE);
			break;
		}
		}

		//Now that we have all the settings in place, make the changes
		if (!SetState(dcb))
		{
			const DWORD dwLastError = GetLastError();
			Close();
			SetLastError(dwLastError);
			return FALSE;
		}

		return TRUE;
	}

	void Close() noexcept
	{
		if (IsOpen())
		{
			//Close down the comms port
			CloseHandle(m_hComm);
			m_hComm = INVALID_HANDLE_VALUE;
		}
	}

	void Attach(_In_ HANDLE hComm) noexcept
	{
		Close();

		//Validate our parameters, now that the port has been closed
#pragma warning(suppress: 26477)
		ATLASSERT(m_hComm == INVALID_HANDLE_VALUE);

		m_hComm = hComm;
	}

	HANDLE Detach() noexcept
	{
		HANDLE hComm = m_hComm; //What will be the return value from this function
		m_hComm = INVALID_HANDLE_VALUE;
		return hComm;
	}

	operator HANDLE() const noexcept
	{
		return m_hComm;
	}

	_NODISCARD BOOL IsOpen() const noexcept
	{
		return m_hComm != INVALID_HANDLE_VALUE;
	}

	//Reading / Writing Methods
	_Must_inspect_result_ BOOL Read(_Out_writes_bytes_(dwNumberOfBytesToRead) __out_data_source(FILE) void* lpBuffer, _In_ DWORD dwNumberOfBytesToRead, _Out_ DWORD& dwBytesRead) noexcept
	{
		//Validate our parameters
#pragma warning(suppress: 26477)
		ATLASSERT(IsOpen());

		dwBytesRead = 0;
		return ReadFile(m_hComm, lpBuffer, dwNumberOfBytesToRead, &dwBytesRead, nullptr);
	}

	_Must_inspect_result_ BOOL Read(_Out_writes_bytes_to_opt_(dwNumberOfBytesToRead, *lpNumberOfBytesRead) __out_data_source(FILE) void* lpBuffer, _In_ DWORD dwNumberOfBytesToRead, _In_ OVERLAPPED& overlapped, _Inout_opt_ DWORD* lpNumberOfBytesRead = nullptr) noexcept
	{
		//Validate our parameters
#pragma warning(suppress: 26477)
		ATLASSERT(IsOpen());

		return ReadFile(m_hComm, lpBuffer, dwNumberOfBytesToRead, lpNumberOfBytesRead, &overlapped);
	}

	_Must_inspect_result_ BOOL ReadEx(_Out_writes_bytes_opt_(dwNumberOfBytesToRead) __out_data_source(FILE) LPVOID lpBuffer, _In_ DWORD dwNumberOfBytesToRead, _Inout_ LPOVERLAPPED lpOverlapped, _In_ LPOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine) noexcept
	{
		//Validate our parameters
#pragma warning(suppress: 26477)
		ATLASSERT(IsOpen());

		return ReadFileEx(m_hComm, lpBuffer, dwNumberOfBytesToRead, lpOverlapped, lpCompletionRoutine);
	}

	BOOL Write(_In_reads_bytes_opt_(dwNumberOfBytesToWrite) const void* lpBuffer, _In_ DWORD dwNumberOfBytesToWrite, _Out_ DWORD& dwBytesWritten) noexcept
	{
		//Validate our parameters
#pragma warning(suppress: 26477)
		ATLASSERT(IsOpen());

		dwBytesWritten = 0;
		return WriteFile(m_hComm, lpBuffer, dwNumberOfBytesToWrite, &dwBytesWritten, nullptr);
	}

	BOOL Write(_In_reads_bytes_opt_(dwNumberOfBytesToWrite) const void* lpBuffer, _In_ DWORD dwNumberOfBytesToWrite, _In_ OVERLAPPED& overlapped, _Out_opt_ DWORD* lpNumberOfBytesWritten = nullptr) noexcept
	{
		//Validate our parameters
#pragma warning(suppress: 26477)
		ATLASSERT(IsOpen());

		return WriteFile(m_hComm, lpBuffer, dwNumberOfBytesToWrite, lpNumberOfBytesWritten, &overlapped);
	}

	BOOL WriteEx(_In_reads_bytes_opt_(dwNumberOfBytesToWrite) LPCVOID lpBuffer, _In_ DWORD dwNumberOfBytesToWrite, _Inout_ LPOVERLAPPED lpOverlapped, _In_ LPOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine) noexcept
	{
		//Validate our parameters
#pragma warning(suppress: 26477)
		ATLASSERT(IsOpen());

		return WriteFileEx(m_hComm, lpBuffer, dwNumberOfBytesToWrite, lpOverlapped, lpCompletionRoutine);
	}

	BOOL TransmitChar(_In_ char cChar) noexcept
	{
		//Validate our parameters
#pragma warning(suppress: 26477)
		ATLASSERT(IsOpen());

		return TransmitCommChar(m_hComm, cChar);
	}

	BOOL GetOverlappedResult(_In_ OVERLAPPED& overlapped, _Out_ DWORD& dwBytesTransferred, _In_ BOOL bWait) noexcept
	{
		//Validate our parameters
#pragma warning(suppress: 26477)
		ATLASSERT(IsOpen());

		return ::GetOverlappedResult(m_hComm, &overlapped, &dwBytesTransferred, bWait);
	}

	__if_exists(::GetOverlappedResultEx)
	{
		BOOL GetOverlappedResultEx(_In_ OVERLAPPED & overlapped, _Out_ DWORD & dwBytesTransferred, _In_ DWORD dwMilliseconds, _In_ BOOL bAlertable) noexcept
		{
			//Validate our parameters
#pragma warning(suppress: 26477)
			ATLASSERT(IsOpen());

			return ::GetOverlappedResultEx(m_hComm, &overlapped, &dwBytesTransferred, dwMilliseconds, bAlertable);
		}
	}

	BOOL CancelIo() noexcept
	{
		//Validate our parameters
#pragma warning(suppress: 26477)
		ATLASSERT(IsOpen());

		return ::CancelIo(m_hComm);
	}

	__if_exists(::CancelIoEx)
	{
		BOOL CancelIoEx(_In_opt_ LPOVERLAPPED lpOverlapped = nullptr) noexcept
		{
			//Validate our parameters
#pragma warning(suppress: 26477)
			ATLASSERT(IsOpen());

			return ::CancelIoEx(m_hComm, lpOverlapped);
		}
	}

	BOOL BytesWaiting(_Out_ DWORD& dwBytesWaiting) noexcept
	{
		//Validate our parameters
#pragma warning(suppress: 26477)
		ATLASSERT(IsOpen());

		//Check to see how many characters are unread
		dwBytesWaiting = 0;
		COMSTAT stat;
		if (!GetStatus(stat))
			return FALSE;
		dwBytesWaiting = stat.cbInQue;
		return TRUE;
	}

	//Configuration Methods
	_Return_type_success_(return != 0) BOOL GetConfig(_Out_ COMMCONFIG& config) noexcept
	{
		//Validate our parameters
#pragma warning(suppress: 26477)
		ATLASSERT(IsOpen());

		DWORD dwSize = sizeof(COMMCONFIG);
		return GetCommConfig(m_hComm, &config, &dwSize);
	}

	static BOOL GetDefaultConfig(_In_ int nPort, _Out_ COMMCONFIG& config)
	{
		//Create the device name as a string
		ATL::CAtlString sPort;
		sPort.Format(_T("COM%d"), nPort);

		//Delegate to the other version of the method
		return GetDefaultConfig(sPort, config);
	}

	static BOOL GetDefaultConfig(_In_z_ LPCTSTR pszPort, _Out_ COMMCONFIG& config) noexcept
	{
		DWORD dwSize = sizeof(COMMCONFIG);
		return GetDefaultCommConfig(pszPort, &config, &dwSize);
	}

	BOOL SetConfig(_In_ COMMCONFIG& config) noexcept
	{
		//Validate our parameters
#pragma warning(suppress: 26477)
		ATLASSERT(IsOpen());

		constexpr const DWORD dwSize = sizeof(COMMCONFIG);
		return SetCommConfig(m_hComm, &config, dwSize);
	}

	static BOOL SetDefaultConfig(_In_ int nPort, _In_ COMMCONFIG& config)
	{
		//Create the device name as a string
		ATL::CAtlString sPort;
		sPort.Format(_T("COM%d"), nPort);

		//Delegate to the other version of the method
		return SetDefaultConfig(sPort, config);
	}

	static BOOL SetDefaultConfig(_In_z_ LPCTSTR pszPort, _In_ COMMCONFIG& config) noexcept
	{
		constexpr const DWORD dwSize = sizeof(COMMCONFIG);
		return SetDefaultCommConfig(pszPort, &config, dwSize);
	}

	//Misc RS232 Methods
	BOOL ClearBreak() noexcept
	{
		//Validate our parameters
#pragma warning(suppress: 26477)
		ATLASSERT(IsOpen());

		return ClearCommBreak(m_hComm);
	}

	BOOL SetBreak() noexcept
	{
		//Validate our parameters
#pragma warning(suppress: 26477)
		ATLASSERT(IsOpen());

		return SetCommBreak(m_hComm);
	}

	BOOL ClearError(_Out_ DWORD& dwErrors) noexcept
	{
		//Validate our parameters
#pragma warning(suppress: 26477)
		ATLASSERT(IsOpen());

		return ClearCommError(m_hComm, &dwErrors, nullptr);
	}

	BOOL GetStatus(_Out_ COMSTAT& stat) noexcept
	{
		//Validate our parameters
#pragma warning(suppress: 26477)
		ATLASSERT(IsOpen());

		DWORD dwErrors = 0;
		return ClearCommError(m_hComm, &dwErrors, &stat);
	}

	BOOL GetState(_Out_ DCB& dcb) noexcept
	{
		//Validate our parameters
#pragma warning(suppress: 26477)
		ATLASSERT(IsOpen());

		return GetCommState(m_hComm, &dcb);
	}

	BOOL SetState(_In_ DCB& dcb) noexcept
	{
		//Validate our parameters
#pragma warning(suppress: 26477)
		ATLASSERT(IsOpen());

		return SetCommState(m_hComm, &dcb);
	}

	BOOL Escape(_In_ DWORD dwFunc) noexcept
	{
		//Validate our parameters
#pragma warning(suppress: 26477)
		ATLASSERT(IsOpen());

		return EscapeCommFunction(m_hComm, dwFunc);
	}

	BOOL ClearDTR() noexcept
	{
		return Escape(CLRDTR);
	}

	BOOL ClearRTS() noexcept
	{
		return Escape(CLRRTS);
	}

	BOOL SetDTR() noexcept
	{
		return Escape(SETDTR);
	}

	BOOL SetRTS() noexcept
	{
		return Escape(SETRTS);
	}

	BOOL SetXOFF() noexcept
	{
		return Escape(SETXOFF);
	}

	BOOL SetXON() noexcept
	{
		return Escape(SETXON);
	}

	BOOL GetProperties(_Inout_ COMMPROP& properties) noexcept
	{
		//Validate our parameters
#pragma warning(suppress: 26477)
		ATLASSERT(IsOpen());

		return GetCommProperties(m_hComm, &properties);
	}

	BOOL GetModemStatus(_Out_ DWORD& dwModemStatus) noexcept
	{
		//Validate our parameters
#pragma warning(suppress: 26477)
		ATLASSERT(IsOpen());

		return GetCommModemStatus(m_hComm, &dwModemStatus);
	}

	//Timeouts
	BOOL SetTimeouts(_In_ COMMTIMEOUTS& timeouts) noexcept
	{
		//Validate our parameters
#pragma warning(suppress: 26477)
		ATLASSERT(IsOpen());

		return SetCommTimeouts(m_hComm, &timeouts);
	}

	BOOL GetTimeouts(_Out_ COMMTIMEOUTS& timeouts) noexcept
	{
		//Validate our parameters
#pragma warning(suppress: 26477)
		ATLASSERT(IsOpen());

		return GetCommTimeouts(m_hComm, &timeouts);
	}

	BOOL Set0Timeout() noexcept
	{
		COMMTIMEOUTS Timeouts;
		memset(&Timeouts, 0, sizeof(Timeouts));
		Timeouts.ReadIntervalTimeout = MAXDWORD;
		return SetTimeouts(Timeouts);
	}

	BOOL Set0WriteTimeout() noexcept
	{
		COMMTIMEOUTS Timeouts;
		GetTimeouts(Timeouts);
		Timeouts.WriteTotalTimeoutMultiplier = 0;
		Timeouts.WriteTotalTimeoutConstant = 0;
		return SetTimeouts(Timeouts);
	}

	BOOL Set0ReadTimeout() noexcept
	{
		COMMTIMEOUTS Timeouts;
		GetTimeouts(Timeouts);
		Timeouts.ReadIntervalTimeout = MAXDWORD;
		Timeouts.ReadTotalTimeoutMultiplier = 0;
		Timeouts.ReadTotalTimeoutConstant = 0;
		return SetTimeouts(Timeouts);
	}

	//Event Methods
	BOOL SetMask(_In_ DWORD dwMask) noexcept
	{
		//Validate our parameters
#pragma warning(suppress: 26477)
		ATLASSERT(IsOpen());

		return SetCommMask(m_hComm, dwMask);
	}

	BOOL GetMask(_Out_ DWORD& dwMask) noexcept
	{
		//Validate our parameters
#pragma warning(suppress: 26477)
		ATLASSERT(IsOpen());

		return GetCommMask(m_hComm, &dwMask);
	}

	BOOL WaitEvent(_Inout_ DWORD& dwMask) noexcept
	{
		//Validate our parameters
#pragma warning(suppress: 26477)
		ATLASSERT(IsOpen());

		return WaitCommEvent(m_hComm, &dwMask, nullptr);
	}

	BOOL WaitEvent(_Inout_ DWORD& dwMask, _Inout_ OVERLAPPED& overlapped) noexcept
	{
		//Validate our parameters
#pragma warning(suppress: 26477)
		ATLASSERT(IsOpen());
#pragma warning(suppress: 26477)
		ATLASSERT(overlapped.hEvent != nullptr);

		return WaitCommEvent(m_hComm, &dwMask, &overlapped);
	}

	//Queue Methods
	BOOL Flush() noexcept
	{
		//Validate our parameters
#pragma warning(suppress: 26477)
		ATLASSERT(IsOpen());

		return FlushFileBuffers(m_hComm);
	}

	BOOL Purge(_In_ DWORD dwFlags) noexcept
	{
		//Validate our parameters
#pragma warning(suppress: 26477)
		ATLASSERT(IsOpen());

		return PurgeComm(m_hComm, dwFlags);
	}

	BOOL TerminateOutstandingWrites() noexcept
	{
		return Purge(PURGE_TXABORT);
	}

	BOOL TerminateOutstandingReads() noexcept
	{
		return Purge(PURGE_RXABORT);
	}

	BOOL ClearWriteBuffer() noexcept
	{
		return Purge(PURGE_TXCLEAR);
	}

	BOOL ClearReadBuffer() noexcept
	{
		return Purge(PURGE_RXCLEAR);
	}

	BOOL Setup(_In_ DWORD dwInQueue, _In_ DWORD dwOutQueue) noexcept
	{
		//Validate our parameters
#pragma warning(suppress: 26477)
		ATLASSERT(IsOpen());

		return SetupComm(m_hComm, dwInQueue, dwOutQueue);
	}

protected:
	//Member variables
	HANDLE m_hComm; //Handle to the comms port
};


//Exception based version of the class
class CSERIALPORT_EXT_CLASS CSerialPort : public CSerialPort2
{
public:
	//General Methods
	__if_exists(OpenCommPort)
	{
#pragma warning(suppress: 26434)
		void Open(_In_ ULONG uPortNumber, _In_ DWORD dwDesiredAccess, _In_ DWORD dwFlagsAndAttributes)
		{
			BOOL bSuccess = __super::Open(uPortNumber, dwDesiredAccess, dwFlagsAndAttributes);
			if (!bSuccess)
				ThrowSerialException();
		}
	}

#pragma warning(suppress: 26434)
	void Open(_In_z_ LPCTSTR pszPort, _In_ BOOL bOverlapped = FALSE)
	{
		const BOOL bSuccess = __super::Open(pszPort, bOverlapped);
		if (!bSuccess)
			ThrowSerialException();
	}

#pragma warning(suppress: 26434)
	void Open(_In_ int nPort, _In_ DWORD dwBaud = 9600, _In_ Parity parity = Parity::NoParity, _In_ BYTE DataBits = 8,
		_In_ StopBits stopBits = StopBits::OneStopBit, _In_ FlowControl fc = FlowControl::NoFlowControl, _In_ BOOL bOverlapped = FALSE)
	{
		const BOOL bSuccess = __super::Open(nPort, dwBaud, parity, DataBits, stopBits, fc, bOverlapped);
		if (!bSuccess)
			ThrowSerialException();
	}

#pragma warning(suppress: 26434)
	void Open(_In_z_ LPCTSTR pszPort, _In_ DWORD dwBaud = 9600, _In_ Parity parity = Parity::NoParity, _In_ BYTE DataBits = 8,
		_In_ StopBits stopBits = StopBits::OneStopBit, _In_ FlowControl fc = FlowControl::NoFlowControl, _In_ BOOL bOverlapped = FALSE)
	{
		const BOOL bSuccess = __super::Open(pszPort, dwBaud, parity, DataBits, stopBits, fc, bOverlapped);
		if (!bSuccess)
			ThrowSerialException();
	}

	//Reading / Writing Methods
#pragma warning(suppress: 26434)
	DWORD Read(_Out_writes_bytes_(dwNumberOfBytesToRead) __out_data_source(FILE) void* lpBuffer, _In_ DWORD dwNumberOfBytesToRead)
	{
		DWORD dwBytesRead = 0;
		if (!__super::Read(lpBuffer, dwNumberOfBytesToRead, dwBytesRead))
			ThrowSerialException();

		return dwBytesRead;
	}

#pragma warning(suppress: 26434)
	void Read(_Out_writes_bytes_to_opt_(dwNumberOfBytesToRead, *lpNumberOfBytesRead) __out_data_source(FILE) void* lpBuffer, _In_ DWORD dwNumberOfBytesToRead, _In_ OVERLAPPED& overlapped, _Inout_opt_ DWORD* lpNumberOfBytesRead = nullptr)
	{
		if (!__super::Read(lpBuffer, dwNumberOfBytesToRead, overlapped, lpNumberOfBytesRead))
			ThrowSerialException();
	}

#pragma warning(suppress: 26434)
	void ReadEx(_Out_writes_bytes_opt_(dwNumberOfBytesToRead) __out_data_source(FILE) LPVOID lpBuffer, _In_ DWORD dwNumberOfBytesToRead, _Inout_ LPOVERLAPPED lpOverlapped, _In_ LPOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine)
	{
		if (!__super::ReadEx(lpBuffer, dwNumberOfBytesToRead, lpOverlapped, lpCompletionRoutine))
			ThrowSerialException();
	}

#pragma warning(suppress: 26434)
	DWORD Write(_In_reads_bytes_opt_(dwNumberOfBytesToWrite) const void* lpBuffer, _In_ DWORD dwNumberOfBytesToWrite)
	{
		DWORD dwBytesWritten = 0;
		if (!__super::Write(lpBuffer, dwNumberOfBytesToWrite, dwBytesWritten))
			ThrowSerialException();

		return dwBytesWritten;
	}

#pragma warning(suppress: 26434)
	void Write(_In_reads_bytes_opt_(dwNumberOfBytesToWrite) const void* lpBuffer, _In_ DWORD dwNumberOfBytesToWrite, _In_ OVERLAPPED& overlapped, _Out_opt_ DWORD* lpNumberOfBytesWritten = nullptr)
	{
		if (!__super::Write(lpBuffer, dwNumberOfBytesToWrite, overlapped, lpNumberOfBytesWritten))
			ThrowSerialException();
	}

#pragma warning(suppress: 26434)
	void WriteEx(_In_reads_bytes_opt_(dwNumberOfBytesToWrite) LPCVOID lpBuffer, _In_ DWORD dwNumberOfBytesToWrite, _Inout_ LPOVERLAPPED lpOverlapped, _In_ LPOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine)
	{
		if (!__super::WriteEx(lpBuffer, dwNumberOfBytesToWrite, lpOverlapped, lpCompletionRoutine))
			ThrowSerialException();
	}

#pragma warning(suppress: 26434)
	void TransmitChar(_In_ char cChar)
	{
		if (!__super::TransmitChar(cChar))
			ThrowSerialException();
	}

#pragma warning(suppress: 26434)
	void GetOverlappedResult(_In_ OVERLAPPED& overlapped, _Out_ DWORD& dwBytesTransferred, _In_ BOOL bWait)
	{
		if (!__super::GetOverlappedResult(overlapped, dwBytesTransferred, bWait))
			ThrowSerialException();
	}

	__if_exists(::GetOverlappedResultEx)
	{
#pragma warning(suppress: 26434)
		void GetOverlappedResultEx(_In_ OVERLAPPED & overlapped, _Out_ DWORD & dwBytesTransferred, _In_ DWORD dwMilliseconds, _In_ BOOL bAlertable)
		{
			if (!__super::GetOverlappedResultEx(overlapped, dwBytesTransferred, dwMilliseconds, bAlertable))
				ThrowSerialException();
		}
	}

#pragma warning(suppress: 26434)
	void CancelIo()
	{
		if (!__super::CancelIo())
			ThrowSerialException();
	}

	__if_exists(::CancelIoEx)
	{
#pragma warning(suppress: 26434)
		void CancelIoEx(_In_opt_ LPOVERLAPPED lpOverlapped = nullptr)
		{
			if (!__super::CancelIoEx(lpOverlapped))
				ThrowSerialException();
		}
	}

#pragma warning(suppress: 26434)
	DWORD BytesWaiting()
	{
		DWORD dwBytesWaiting = 0;
		if (!__super::BytesWaiting(dwBytesWaiting))
			ThrowSerialException();
		return dwBytesWaiting;
	}

	//Configuration Methods
#pragma warning(suppress: 26434)
	void GetConfig(_Out_ COMMCONFIG& config)
	{
		if (!__super::GetConfig(config))
			ThrowSerialException();
	}

	static void GetDefaultConfig(_In_ int nPort, _Out_ COMMCONFIG& config)
	{
		if (!__super::GetDefaultConfig(nPort, config))
			ThrowSerialException();
	}

	static void GetDefaultConfig(_In_z_ LPCTSTR pszPort, _Out_ COMMCONFIG& config)
	{
		if (!__super::GetDefaultConfig(pszPort, config))
			ThrowSerialException();
	}

#pragma warning(suppress: 26434)
	void SetConfig(_In_ COMMCONFIG& config)
	{
		if (!__super::SetConfig(config))
			ThrowSerialException();
	}

	static void SetDefaultConfig(_In_ int nPort, _In_ COMMCONFIG& config)
	{
		if (!__super::SetDefaultConfig(nPort, config))
			ThrowSerialException();
	}

	static void SetDefaultConfig(_In_z_ LPCTSTR pszPort, _In_ COMMCONFIG& config)
	{
		if (!__super::SetDefaultConfig(pszPort, config))
			ThrowSerialException();
	}

	//Misc RS232 Methods
#pragma warning(suppress: 26434)
	void ClearBreak()
	{
		if (!__super::ClearBreak())
			ThrowSerialException();
	}

#pragma warning(suppress: 26434)
	void SetBreak()
	{
		if (!__super::SetBreak())
			ThrowSerialException();
	}

#pragma warning(suppress: 26434)
	void ClearError(_Out_ DWORD& dwErrors)
	{
		if (!__super::ClearError(dwErrors))
			ThrowSerialException();
	}

#pragma warning(suppress: 26434)
	void GetStatus(_Out_ COMSTAT& stat)
	{
		if (!__super::GetStatus(stat))
			ThrowSerialException();
	}

#pragma warning(suppress: 26434)
	void GetState(_Out_ DCB& dcb)
	{
		if (!__super::GetState(dcb))
			ThrowSerialException();
	}

#pragma warning(suppress: 26434)
	void SetState(_In_ DCB& dcb)
	{
		if (!__super::SetState(dcb))
			ThrowSerialException();
	}

#pragma warning(suppress: 26434)
	void Escape(_In_ DWORD dwFunc)
	{
		if (!__super::Escape(dwFunc))
			ThrowSerialException();
	}

#pragma warning(suppress: 26434)
	void ClearDTR()
	{
		if (!__super::ClearDTR())
			ThrowSerialException();
	}

#pragma warning(suppress: 26434)
	void ClearRTS()
	{
		if (!__super::ClearRTS())
			ThrowSerialException();
	}

#pragma warning(suppress: 26434)
	void SetDTR()
	{
		if (!__super::SetDTR())
			ThrowSerialException();
	}

#pragma warning(suppress: 26434)
	void SetRTS()
	{
		if (!__super::SetRTS())
			ThrowSerialException();
	}

#pragma warning(suppress: 26434)
	void SetXOFF()
	{
		if (!__super::SetXOFF())
			ThrowSerialException();
	}

#pragma warning(suppress: 26434)
	void SetXON()
	{
		if (!__super::SetXON())
			ThrowSerialException();
	}

#pragma warning(suppress: 26434)
	void GetProperties(_Inout_ COMMPROP& properties)
	{
		if (!__super::GetProperties(properties))
			ThrowSerialException();
	}

#pragma warning(suppress: 26434)
	void GetModemStatus(_Out_ DWORD& dwModemStatus)
	{
		if (!__super::GetModemStatus(dwModemStatus))
			ThrowSerialException();
	}

	//Timeouts
#pragma warning(suppress: 26434)
	void SetTimeouts(_In_ COMMTIMEOUTS& timeouts)
	{
		if (!__super::SetTimeouts(timeouts))
			ThrowSerialException();
	}

#pragma warning(suppress: 26434)
	void GetTimeouts(_Out_ COMMTIMEOUTS& timeouts)
	{
		if (!__super::GetTimeouts(timeouts))
			ThrowSerialException();
	}

#pragma warning(suppress: 26434)
	void Set0Timeout()
	{
		if (!__super::Set0Timeout())
			ThrowSerialException();
	}

#pragma warning(suppress: 26434)
	void Set0WriteTimeout()
	{
		if (!__super::Set0WriteTimeout())
			ThrowSerialException();
	}

#pragma warning(suppress: 26434)
	void Set0ReadTimeout()
	{
		if (!__super::Set0ReadTimeout())
			ThrowSerialException();
	}

	//Event Methods
#pragma warning(suppress: 26434)
	void SetMask(_In_ DWORD dwMask)
	{
		if (!__super::SetMask(dwMask))
			ThrowSerialException();
	}

#pragma warning(suppress: 26434)
	void GetMask(_Out_ DWORD& dwMask)
	{
		if (!__super::GetMask(dwMask))
			ThrowSerialException();
	}

#pragma warning(suppress: 26434)
	void WaitEvent(_Inout_ DWORD& dwMask)
	{
		if (!__super::WaitEvent(dwMask))
			ThrowSerialException();
	}

#pragma warning(suppress: 26434)
	void WaitEvent(_Inout_ DWORD& dwMask, _Inout_ OVERLAPPED& overlapped)
	{
		const BOOL bSuccess = __super::WaitEvent(dwMask, overlapped);
		if (!bSuccess)
			ThrowSerialException();
	}

	//Queue Methods
#pragma warning(suppress: 26434)
	void Flush()
	{
		if (!__super::Flush())
			ThrowSerialException();
	}

#pragma warning(suppress: 26434)
	void Purge(_In_ DWORD dwFlags)
	{
		if (!__super::Purge(dwFlags))
			ThrowSerialException();
	}

#pragma warning(suppress: 26434)
	void TerminateOutstandingWrites()
	{
		if (!__super::TerminateOutstandingWrites())
			ThrowSerialException();
	}

#pragma warning(suppress: 26434)
	void TerminateOutstandingReads()
	{
		if (!__super::TerminateOutstandingReads())
			ThrowSerialException();
	}

#pragma warning(suppress: 26434)
	void ClearWriteBuffer()
	{
		if (!__super::ClearWriteBuffer())
			ThrowSerialException();
	}

#pragma warning(suppress: 26434)
	void ClearReadBuffer()
	{
		if (!__super::ClearReadBuffer())
			ThrowSerialException();
	}

#pragma warning(suppress: 26434)
	void Setup(_In_ DWORD dwInQueue, _In_ DWORD dwOutQueue)
	{
		if (!__super::Setup(dwInQueue, dwOutQueue))
			ThrowSerialException();
	}

	//Static methods
	static void ThrowSerialException(_In_ DWORD dwError = 0)
	{
		if (dwError == 0)
			dwError = ::GetLastError();

		ATLTRACE(_T("Warning: throwing CSerialException for error %d\n"), dwError);
		CSerialException e(dwError);
		throw e;
	}
};

#endif //#ifndef __SERIALPORT_H__
