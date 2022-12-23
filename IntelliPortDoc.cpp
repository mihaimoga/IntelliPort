/* This file is part of IntelliPort application developed by Mihai MOGA.

IntelliPort is free software: you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the Open
Source Initiative, either version 3 of the License, or any later version.

IntelliPort is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
IntelliPort. If not, see <http://www.opensource.org/licenses/gpl-3.0.html>*/

// IntelliPortDoc.cpp : implementation of the CIntelliPortDoc class
//

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "IntelliPort.h"
#endif

#include "IntelliPortDoc.h"
#include "MainFrame.h"
#include <propkey.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CIntelliPortDoc

IMPLEMENT_DYNCREATE(CIntelliPortDoc, CDocument)

BEGIN_MESSAGE_MAP(CIntelliPortDoc, CDocument)
	ON_COMMAND(ID_FILE_SEND_MAIL, &CIntelliPortDoc::OnFileSendMail)
	ON_UPDATE_COMMAND_UI(ID_FILE_SEND_MAIL, &CIntelliPortDoc::OnUpdateFileSendMail)
END_MESSAGE_MAP()

// CIntelliPortDoc construction/destruction

CIntelliPortDoc::CIntelliPortDoc()
{
	VERIFY(m_fontTerminal.CreateFont(
		-MulDiv(10, GetDeviceCaps(::GetDC(NULL), LOGPIXELSY), 72), // nHeight
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

CIntelliPortDoc::~CIntelliPortDoc()
{
	VERIFY(m_fontTerminal.DeleteObject());
}

BOOL CIntelliPortDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;
	if (!m_viewList.IsEmpty())
	{
		reinterpret_cast<CEditView*>(m_viewList.GetHead())->SetWindowText(NULL);
	}

	// (SDI documents will reuse this document)
	reinterpret_cast<CEditView*>(m_viewList.GetHead())->GetEditCtrl().SetFont(&m_fontTerminal);
	reinterpret_cast<CEditView*>(m_viewList.GetHead())->GetEditCtrl().SetReadOnly(TRUE);
	//const UINT Before = reinterpret_cast<CEditView*>(m_viewList.GetHead())->GetEditCtrl().GetLimitText();
	reinterpret_cast<CEditView*>(m_viewList.GetHead())->GetEditCtrl().SetLimitText(0);
	//const UINT After = reinterpret_cast<CEditView*>(m_viewList.GetHead())->GetEditCtrl().GetLimitText();
	//TCHAR buffer[0x100] = { 0, };
	//_stprintf(buffer, _T("Before = %u, After = %u"), Before, After);
	//MessageBox(NULL, buffer, _T("IntelliPort"), MB_OK);

	return TRUE;
}

// CIntelliPortDoc serialization

void CIntelliPortDoc::Serialize(CArchive& ar)
{
	// CEditView contains an edit control which handles all serialization
	if (!m_viewList.IsEmpty())
	{
		// reinterpret_cast<CEditView*>(m_viewList.GetHead())->SerializeRaw(ar);
		CEditView* pEditView = reinterpret_cast<CEditView*>(m_viewList.GetHead());
		if (ar.IsLoading())
		{
			CStringA pInput;
			UINT nLength = (UINT) ar.GetFile()->GetLength();
			ar.GetFile()->Read(pInput.GetBufferSetLength(nLength), nLength);
			pInput.ReleaseBuffer();
			CStringW pBuffer(pInput);
			pEditView->GetEditCtrl().SetWindowText(pBuffer);
		}
		else
		{
			CStringW pBuffer;
			pEditView->GetEditCtrl().GetWindowText(pBuffer);
			CStringA pOutput(pBuffer);
			UINT nLength = pOutput.GetLength();
			ar.GetFile()->Write(pOutput.GetBuffer(nLength), nLength);
			pOutput.ReleaseBuffer();
		}
		pEditView->GetEditCtrl().SetModify(FALSE);
	}

	CString strFormat, strMessage;
	CMainFrame* pMainFrame = (CMainFrame*) AfxGetMainWnd();
	ASSERT_VALID(pMainFrame);
	strFormat.LoadString(ar.IsLoading() ? IDS_FILE_HAS_BEEN_LOADED : IDS_FILE_HAS_BEEN_SAVED);
	strMessage.Format(strFormat, ar.m_strFileName);
	pMainFrame->SetStatusBarText(strMessage);

#ifdef SHARED_HANDLERS
	if (m_viewList.IsEmpty() && ar.IsLoading())
	{
		CFile* pFile = ar.GetFile();
		pFile->Seek(0, FILE_BEGIN);
		ULONGLONG nFileSizeBytes = pFile->GetLength();
		ULONGLONG nFileSizeChars = nFileSizeBytes/sizeof(TCHAR);
		LPTSTR lpszText = (LPTSTR)malloc(((size_t)nFileSizeChars + 1) * sizeof(TCHAR));
		if (lpszText != NULL)
		{
			ar.Read(lpszText, (UINT)nFileSizeBytes);
			lpszText[nFileSizeChars] = '\0';
			m_strThumbnailContent = lpszText;
			m_strSearchContent = lpszText;
		}
	}
#endif
}

#ifdef SHARED_HANDLERS
// Support for thumbnails
void CIntelliPortDoc::OnDrawThumbnail(CDC& dc, LPRECT lprcBounds)
{
	// Modify this code to draw the document's data
	dc.FillSolidRect(lprcBounds, RGB(255, 255, 255));

	LOGFONT lf;

	CFont* pDefaultGUIFont = CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT));
	pDefaultGUIFont->GetLogFont(&lf);
	lf.lfHeight = 36;

	CFont fontDraw;
	fontDraw.CreateFontIndirect(&lf);

	CFont* pOldFont = dc.SelectObject(&fontDraw);
	dc.DrawText(m_strThumbnailContent, lprcBounds, DT_CENTER | DT_WORDBREAK);
	dc.SelectObject(pOldFont);
}

// Support for Search Handlers
void CIntelliPortDoc::InitializeSearchContent()
{
	// Set search contents from document's data. 
	// The content parts should be separated by ";"

	// Use the entire text file content as the search content.
	SetSearchContent(m_strSearchContent);
}

void CIntelliPortDoc::SetSearchContent(const CString& value)
{
	if (value.IsEmpty())
	{
		RemoveChunk(PKEY_Search_Contents.fmtid, PKEY_Search_Contents.pid);
	}
	else
	{
		CMFCFilterChunkValueImpl *pChunk = NULL;
		ATLTRY(pChunk = new CMFCFilterChunkValueImpl);
		if (pChunk != NULL)
		{
			pChunk->SetTextValue(PKEY_Search_Contents, value, CHUNK_TEXT);
			SetChunkValue(pChunk);
		}
	}
}
#endif // SHARED_HANDLERS

// CIntelliPortDoc diagnostics

#ifdef _DEBUG
void CIntelliPortDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CIntelliPortDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

// CIntelliPortDoc commands
