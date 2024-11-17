/* Copyright (C) 2014-2025 Stefan-Mihai MOGA
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

CStringA W2UTF8(_In_NLS_string_(nLength) const wchar_t* pszText, _In_ int nLength)
{
	// First call the function to determine how much space we need to allocate
	int nUTF8Length{ WideCharToMultiByte(CP_UTF8, 0, pszText, nLength, nullptr, 0, nullptr, nullptr) };

	// If the calculated length is zero, then ensure we have at least room for a null terminator
	if (nUTF8Length == 0)
		nUTF8Length = 1;

	// Now recall with the buffer to get the converted text
	CStringA sUTF;
#pragma warning(suppress: 26429)
	char* const pszUTF8Text{ sUTF.GetBuffer(nUTF8Length + 1) }; // include an extra byte because we may be null terminating the string ourselves
	int nCharsWritten{ WideCharToMultiByte(CP_UTF8, 0, pszText, nLength, pszUTF8Text, nUTF8Length, nullptr, nullptr) };

	// Ensure we null terminate the text if WideCharToMultiByte doesn't do it for us
	if (nLength != -1)
	{
#pragma warning(suppress: 26477 26496)
		ATLASSUME(nCharsWritten <= nUTF8Length);
#pragma warning(suppress: 26481)
		pszUTF8Text[nCharsWritten] = '\0';
	}
	sUTF.ReleaseBuffer();

	return sUTF;
}

CStringW UTF82W(_In_NLS_string_(nLength) const char* pszText, _In_ int nLength)
{
	// First call the function to determine how much space we need to allocate
	int nWideLength{ MultiByteToWideChar(CP_UTF8, 0, pszText, nLength, nullptr, 0) };

	// If the calculated length is zero, then ensure we have at least room for a null terminator
	if (nWideLength == 0)
		nWideLength = 1;

	//Now recall with the buffer to get the converted text
	CStringW sWideString;
#pragma warning(suppress: 26429)
	wchar_t* pszWText{ sWideString.GetBuffer(nWideLength + 1) }; //include an extra byte because we may be null terminating the string ourselves
	int nCharsWritten{ MultiByteToWideChar(CP_UTF8, 0, pszText, nLength, pszWText, nWideLength) };

	//Ensure we null terminate the text if MultiByteToWideChar doesn't do it for us
	if (nLength != -1)
	{
#pragma warning(suppress: 26477 26496)
		ATLASSUME(nCharsWritten <= nWideLength);
#pragma warning(suppress: 26481)
		pszWText[nCharsWritten] = '\0';
	}
	sWideString.ReleaseBuffer();

	return sWideString;
}

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
		reinterpret_cast<CEditView*>(m_viewList.GetHead())->SetWindowText(nullptr);
	}

	// (SDI documents will reuse this document)
	reinterpret_cast<CEditView*>(m_viewList.GetHead())->GetEditCtrl().SetFont(&m_fontTerminal);
	reinterpret_cast<CEditView*>(m_viewList.GetHead())->GetEditCtrl().SetReadOnly(TRUE);
	reinterpret_cast<CEditView*>(m_viewList.GetHead())->GetEditCtrl().LimitText(-1);

	return TRUE;
}

// CIntelliPortDoc serialization

bool CIntelliPortDoc::AddText(CString strText)
{
	if (!m_viewList.IsEmpty())
	{
		CEditView* pEditView = reinterpret_cast<CEditView*>(m_viewList.GetHead());
		if (pEditView != nullptr)
		{
			strText.Replace(CRLF, LF);
			strText.Replace(CR, LF);
			strText.Replace(LF, CRLF);
			CEdit& pEdit = pEditView->GetEditCtrl();
			int outLength = pEdit.GetWindowTextLength();
			pEdit.SetSel(outLength, outLength);
			pEdit.ReplaceSel(strText, TRUE);
			pEdit.SetSel(-1, 0);
			return true;
		}
	}
	return false;
}

void CIntelliPortDoc::Serialize(CArchive& ar)
{
	if (ar.IsLoading())
	{
#pragma warning(suppress: 26429)
		CFile* pFile{ ar.GetFile() };
		ASSERT(pFile != nullptr);
		const auto nFileSize = pFile->GetLength();
#pragma warning(suppress: 26472)
		const auto nFileSizeForLoader{ static_cast<intptr_t>(nFileSize) };

		//Check if the size of the file to be loaded can be handled via CreateLoader and fail if not
#pragma warning(suppress: 26472)
		if (nFileSize != static_cast<ULONGLONG>(nFileSizeForLoader))
		{
			AfxMessageBox(IDP_FAIL_SCINTILLA_DOCUMENT_TOO_LARGE, MB_ICONEXCLAMATION);
			AfxThrowUserException();
		}

		// Read the data in from the file in blocks
		std::vector<BYTE> byBuffer{ 0x10000, std::allocator<BYTE>{} };
		int nBytesRead{ 0 };
		m_BOM = BOM::Unknown;
		bool bDetectedBOM{ false };
		do
		{
#pragma warning(suppress: 26472)
			nBytesRead = pFile->Read(byBuffer.data(), static_cast<UINT>(byBuffer.size()));
			if (nBytesRead)
			{
				int nSkip{ 0 };
				if (!bDetectedBOM)
				{
					bDetectedBOM = true;
					int nUniTest = IS_TEXT_UNICODE_STATISTICS;
					// detect UTF-16BE with BOM
#pragma warning(suppress: 26446)
					if ((nBytesRead > 1) && ((nFileSize % 2) == 0) && ((nBytesRead % 2) == 0) && (byBuffer[0] == 0xFE) && (byBuffer[1] == 0xFF))
					{
						m_BOM = BOM::UTF16BE;
						nSkip = 2;
					}
					// detect UTF-16LE with BOM
#pragma warning(suppress: 26446)
					else if ((nBytesRead > 1) && ((nFileSize % 2) == 0) && ((nBytesRead % 2) == 0) && (byBuffer[0] == 0xFF) && (byBuffer[1] == 0xFE))
					{
						m_BOM = BOM::UTF16LE;
						nSkip = 2;
					}
					// detect UTF-8 with BOM
#pragma warning(suppress: 26446)
					else if ((nBytesRead > 2) && (byBuffer[0] == 0xEF) && (byBuffer[1] == 0xBB) && (byBuffer[2] == 0xBF))
					{
						m_BOM = BOM::UTF8;
						nSkip = 3;
					}
					// detect UTF-16LE without BOM (Note IS_TEXT_UNICODE_STATISTICS implies Little Endian for all versions of supported Windows platforms i.e. x86, x64, ARM & ARM64)
#pragma warning(suppress: 26446)
					else if ((nBytesRead > 1) && ((nFileSize % 2) == 0) && ((nBytesRead % 2) == 0) && (byBuffer[0] != 0) && (byBuffer[1] == 0) && IsTextUnicode(byBuffer.data(), nBytesRead, &nUniTest))
					{
						m_BOM = BOM::UTF16LE_NOBOM;
						nSkip = 0;
					}
				}

				// Work out the data to pass to ILoader->AddData
				const char* pLoadData{ nullptr };
				int nBytesToLoad{ 0 };
				CStringA sUTF8Data;
				if ((m_BOM == BOM::UTF16LE_NOBOM) || (m_BOM == BOM::UTF16LE))
				{
					// Handle conversion from UTF16LE to UTF8
#pragma warning(suppress: 26481 26490)
					sUTF8Data = W2UTF8(reinterpret_cast<const wchar_t*>(byBuffer.data() + nSkip), (nBytesRead - nSkip) / 2);
					pLoadData = sUTF8Data;
					nBytesToLoad = sUTF8Data.GetLength();
				}
				else if (m_BOM == BOM::UTF16BE)
				{
					// Handle conversion from UTF16BE to UTF8
#pragma warning(suppress: 26429 26481)
					BYTE* p = byBuffer.data() + nSkip;
					const int nUTF16CharsRead = (nBytesRead - nSkip) / 2;
					for (int i = 0; i < nUTF16CharsRead; i++)
					{
						const BYTE t{ *p };
#pragma warning(suppress: 26481)
						* p = p[1];
#pragma warning(suppress: 26481)
						p[1] = t;
#pragma warning(suppress: 26481)
						p += 2;
					}
#pragma warning(suppress: 26481 26490)
					sUTF8Data = W2UTF8(reinterpret_cast<const wchar_t*>(byBuffer.data() + nSkip), (nBytesRead - nSkip) / 2);
					pLoadData = sUTF8Data;
					nBytesToLoad = sUTF8Data.GetLength();
				}
				else
				{
#pragma warning(suppress: 26481 26490)
					pLoadData = reinterpret_cast<const char*>(byBuffer.data()) + nSkip;
					nBytesToLoad = nBytesRead - nSkip;
				}
				ASSERT(pLoadData != nullptr);
				AddText(utf8_to_wstring(pLoadData).c_str());
			}
		} while (nBytesRead);
	}
	else
	{
#pragma warning(suppress: 26429)
		/* CFile* pFile{ ar.GetFile() };
		ASSERT(pFile != nullptr);
		intptr_t nBytesToWrite{ GetLength() };
		const char* CharacterPointer = GetCharacterPointer();
		bool bHandledBOM{false};
		while (nBytesToWrite > 0)
		{
			intptr_t nDataToWrite{ nBytesToWrite };
			if (nDataToWrite > UINT_MAX)
				nDataToWrite = UINT_MAX;

			// Handle writing the BOM if necessary
			if (!bHandledBOM)
			{
				bHandledBOM = true;
				switch (m_BOM)
				{
					case BOM::UTF8:
					{
						pFile->Write("\xEF\xBB\xBF", 3);
						break;
					}
					case BOM::UTF16BE:
					{
						pFile->Write("\xFE\xFF", 2);
						break;
					}
					case BOM::UTF16LE:
					{
						pFile->Write("\xFF\xFE", 2);
						break;
					}
					default:
					{
						break;
					}
				}
			}

			// Work out the data to save to file
			const void* pSaveData{ nullptr };
			UINT nBytesToSave{ 0 };
			CStringW sUTF16Data;
			if ((m_BOM == BOM::UTF16LE_NOBOM) || ((m_BOM == BOM::UTF16LE)))
			{
				//Handle conversion from UTF8 to UTF16LE
#pragma warning(suppress: 26472)
				sUTF16Data = UTF82W(CharacterPointer, static_cast<int>(nDataToWrite));
				pSaveData = sUTF16Data.GetString();
				nBytesToSave = sUTF16Data.GetLength() * sizeof(wchar_t);
			}
			else if (m_BOM == BOM::UTF16BE)
			{
				//Handle conversion from UTF8 to UTF16BE
#pragma warning(suppress: 26472)
				sUTF16Data = UTF82W(CharacterPointer, static_cast<int>(nDataToWrite));
#pragma warning(suppress: 26429 26490)
				BYTE* p = reinterpret_cast<BYTE*>(sUTF16Data.GetBuffer());
				const int nUTF16CharsWrite = sUTF16Data.GetLength();
				for (int i = 0; i < nUTF16CharsWrite; i++)
				{
					const BYTE t{ *p };
#pragma warning(suppress: 26481)
					* p = p[1];
#pragma warning(suppress: 26481)
					p[1] = t;
#pragma warning(suppress: 26481)
					p += 2;
				}
				sUTF16Data.ReleaseBuffer();
				pSaveData = sUTF16Data.GetString();
				nBytesToSave = sUTF16Data.GetLength() * sizeof(wchar_t);
			}
			else
			{
				pSaveData = CharacterPointer;
#pragma warning(suppress: 26472)
				nBytesToSave = static_cast<UINT>(nDataToWrite);
			}

			// Write the data to file
			ASSERT(pSaveData != nullptr);
			pFile->Write(pSaveData, nBytesToSave);

			// Prepare for the next loop around
#pragma warning(suppress: 26481)
			CharacterPointer += nDataToWrite;
			nBytesToWrite -= nDataToWrite;
		} */
		if (!m_viewList.IsEmpty())
		{
			CEditView* pEditView = reinterpret_cast<CEditView*>(m_viewList.GetHead());
			if (pEditView != nullptr)
			{
				ar.GetFile()->Write("\xEF\xBB\xBF", 3);
				CStringW strBuffer;
				pEditView->GetEditCtrl().GetWindowText(strBuffer);
				const std::wstring strRawText(strBuffer);
				// convert Unicode characters to UTF8
				CStringA strOutput(wstring_to_utf8(strRawText).c_str());
				UINT nLength = strOutput.GetLength();
				// write to file the window's content
				ar.GetFile()->Write(strOutput.GetBuffer(nLength), nLength);
				strOutput.ReleaseBuffer();
			}
		}
	}

	CString strFormat, strMessage;
	CMainFrame* pMainFrame = (CMainFrame*) AfxGetMainWnd();
	ASSERT_VALID(pMainFrame);
	VERIFY(strFormat.LoadString(ar.IsLoading() ? IDS_FILE_HAS_BEEN_LOADED : IDS_FILE_HAS_BEEN_SAVED));
	strMessage.Format(strFormat, static_cast<LPCWSTR>(ar.m_strFileName));
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
