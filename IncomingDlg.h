#pragma once

// CIncomingDlg dialog

class CIncomingDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CIncomingDlg)

public:
	CIncomingDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CIncomingDlg();

// Dialog Data
	enum { IDD = IDD_INCOMINGDLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
