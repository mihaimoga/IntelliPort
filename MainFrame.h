/* This file is part of IntelliPort application developed by Stefan-Mihai MOGA.

IntelliPort is free software: you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the Open
Source Initiative, either version 3 of the License, or any later version.

IntelliPort is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
IntelliPort. If not, see <http://www.opensource.org/licenses/gpl-3.0.html>*/

// MainFrame.h : interface of the CMainFrame class
//

#pragma once

#include "SerialPort.h"
#include "SocMFC.h"
#include "RingBuffer.h"
#include "IncomingDlg.h"
#include <mutex>

class CMainFrame : public CFrameWndEx
{
	
protected: // create from serialization only
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

// Attributes
public:

// Operations
public:

// Overrides
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

// Implementation
public:
	bool CreateCaptionBar();
	virtual ~CMainFrame();
	bool SetStatusBarText(const CString& strMessage);
	bool SetCaptionBarText(const CString& strMessage);
	bool HideMessageBar();
	bool AddText(CString strText);

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // control bar embedded members
	CMFCRibbonBar m_wndRibbonBar;
	CMFCRibbonApplicationButton m_MainButton;
	CMFCToolBarImages m_PanelImages;
	CMFCRibbonStatusBar m_wndStatusBar;
	CMFCCaptionBar m_wndCaptionBar;
	CIncomingDlg m_dlgIncoming;
public:
	std::mutex m_pMutualAccess;
	CRingBuffer m_pRingBuffer;
	CSerialPort m_pSerialPort;
	CWSocket m_pSocket;
	CWSocket m_pIncomming;
	CTime m_pCurrentDateTime;
	UINT_PTR m_nTimerID;
	bool m_nThreadRunning;
	HANDLE m_hSerialPortThread;
	HANDLE m_hSocketThread;
	DWORD m_nSerialPortThreadID;
	DWORD m_nSocketTreadID;

// Generated message map functions
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnViewCaptionBar();
	afx_msg void OnUpdateViewCaptionBar(CCmdUI* pCmdUI);
	afx_msg void OnOptions();
	afx_msg void OnFilePrint();
	afx_msg void OnFilePrintPreview();
	afx_msg void OnUpdateFilePrintPreview(CCmdUI* pCmdUI);
	// Custom implementation
	afx_msg void OnConfigureSerialPort();
	afx_msg void OnOpenSerialPort();
	afx_msg void OnCloseSerialPort();
	afx_msg void OnSendReceive();
	afx_msg void OnUpdateConfigureSerialPort(CCmdUI* pCmdUI);
	afx_msg void OnUpdateOpenSerialPort(CCmdUI* pCmdUI);
	afx_msg void OnUpdateCloseSerialPort(CCmdUI* pCmdUI);
	afx_msg void OnUpdateSendReceive(CCmdUI* pCmdUI);
	afx_msg void OnTwitter();
	afx_msg void OnLinkedin();
	afx_msg void OnFacebook();
	afx_msg void OnInstagram();
	afx_msg void OnIssues();
	afx_msg void OnDiscussions();
	afx_msg void OnWiki();

	DECLARE_MESSAGE_MAP()
};

static DWORD WINAPI SerialPortThreadFunc(LPVOID pParam);

static DWORD WINAPI SocketThreadFunc(LPVOID pParam);
