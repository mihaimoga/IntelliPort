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

/**
 * Converts a wide character (Unicode) string to UTF-8 encoded multi-byte string.
 * @param pszText Pointer to the wide character string to convert
 * @param nLength Length of the input string in characters, or -1 for null-terminated strings
 * @return UTF-8 encoded string as CStringA
 */
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

/**
 * Converts a UTF-8 encoded multi-byte string to wide character (Unicode) string.
 * @param pszText Pointer to the UTF-8 string to convert
 * @param nLength Length of the input string in bytes, or -1 for null-terminated strings
 * @return Wide character string as CStringW
 */
CStringW UTF82W(_In_NLS_string_(nLength) const char* pszText, _In_ int nLength)
{
	// First call the function to determine how much space we need to allocate
	int nWideLength{ MultiByteToWideChar(CP_UTF8, 0, pszText, nLength, nullptr, 0) };

	// If the calculated length is zero, then ensure we have at least room for a null terminator
	if (nWideLength == 0)
		nWideLength = 1;

	// Now recall with the buffer to get the converted text
	CStringW sWideString;
#pragma warning(suppress: 26429)
	wchar_t* pszWText{ sWideString.GetBuffer(nWideLength + 1) }; // include an extra byte because we may be null terminating the string ourselves
	int nCharsWritten{ MultiByteToWideChar(CP_UTF8, 0, pszText, nLength, pszWText, nWideLength) };

	// Ensure we null terminate the text if MultiByteToWideChar doesn't do it for us
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

/**
 * Constructor: Initializes the document and creates the terminal font.
 * Sets up a Consolas font for the terminal display with anti-aliasing.
 */
CIntelliPortDoc::CIntelliPortDoc()
{
	// Initialize BOM (Byte Order Mark) to unknown state
	m_BOM = BOM::Unknown;
	
	// Create the terminal font with Consolas typeface for better readability
	VERIFY(m_fontTerminal.CreateFont(
		-MulDiv(10, GetDeviceCaps(::GetDC(nullptr), LOGPIXELSY), 72), // nHeight - 10pt font
		0,                         // nWidth - use default
		0,                         // nEscapement - no rotation
		0,                         // nOrientation - no rotation
		FW_NORMAL,                 // nWeight - normal weight
		FALSE,                     // bItalic - not italic
		FALSE,                     // bUnderline - not underlined
		0,                         // cStrikeOut - no strikeout
		ANSI_CHARSET,              // nCharSet - ANSI character set
		OUT_DEFAULT_PRECIS,        // nOutPrecision - default precision
		CLIP_DEFAULT_PRECIS,       // nClipPrecision - default clipping
		DEFAULT_QUALITY,           // nQuality - antialiased for smooth rendering
		DEFAULT_PITCH | FF_MODERN, // nPitchAndFamily - modern fixed-pitch font
		_T("Consolas")));          // lpszFacename - Consolas font
}

/**
 * Destructor: Cleans up resources, specifically the terminal font.
 */
CIntelliPortDoc::~CIntelliPortDoc()
{
	// Clean up the terminal font object
	VERIFY(m_fontTerminal.DeleteObject());
}

/**
 * Called when a new document is created. Sets up the edit control
 * with appropriate properties for terminal display.
 * @return TRUE if successful, FALSE otherwise
 */
BOOL CIntelliPortDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;
		
	// Clear any existing text in the view
	if (!m_viewList.IsEmpty())
	{
		reinterpret_cast<CEditView*>(m_viewList.GetHead())->SetWindowText(nullptr);
	}

	// Configure the edit control for terminal display
	// (SDI documents will reuse this document)
	reinterpret_cast<CEditView*>(m_viewList.GetHead())->GetEditCtrl().SetFont(&m_fontTerminal);
	reinterpret_cast<CEditView*>(m_viewList.GetHead())->GetEditCtrl().SetReadOnly(TRUE); // Terminal output is read-only
	reinterpret_cast<CEditView*>(m_viewList.GetHead())->GetEditCtrl().LimitText(-1); // No text limit

	return TRUE;
}

// CIntelliPortDoc serialization

/**
 * Adds text to the terminal view. Normalizes line endings to CRLF format
 * and appends the text to the end of the current content.
 * @param strText The text to add to the terminal display
 * @return true if text was successfully added, false otherwise
 */
bool CIntelliPortDoc::AddText(CString strText)
{
	if (!m_viewList.IsEmpty())
	{
		CEditView* pEditView = reinterpret_cast<CEditView*>(m_viewList.GetHead());
		if (pEditView != nullptr)
		{
			// Normalize line endings: Convert all line ending formats to CRLF
			strText.Replace(CRLF, LF);   // First convert CRLF to LF
			strText.Replace(CR, LF);     // Then convert standalone CR to LF
			strText.Replace(LF, CRLF);   // Finally convert all LF to CRLF
			
			// Append text to the end of the edit control
			CEdit& pEdit = pEditView->GetEditCtrl();
			int outLength = pEdit.GetWindowTextLength();
			pEdit.SetSel(outLength, outLength); // Move cursor to end
			pEdit.ReplaceSel(strText, TRUE);    // Insert new text
			pEdit.SetSel(-1, 0);                // Reset selection
			return true;
		}
	}
	return false;
}

/**
 * Handles document serialization for loading and saving files.
 * Supports multiple text encodings including UTF-8, UTF-16LE, and UTF-16BE with and without BOM.
 * @param ar Archive object for reading/writing file data
 */
void CIntelliPortDoc::Serialize(CArchive& ar)
{
	if (ar.IsLoading())
	{
		// Loading: Read file content and detect encoding
#pragma warning(suppress: 26429)
		CFile* pFile{ ar.GetFile() };
		ASSERT(pFile != nullptr);
		const auto nFileSize = pFile->GetLength();

		// Read the data in from the file in blocks
		std::vector<BYTE> byBuffer{ static_cast<UINT>(nFileSize), std::allocator<BYTE>{} };
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
				
				// Detect BOM (Byte Order Mark) on first read
				if (!bDetectedBOM)
				{
					bDetectedBOM = true;
					int nUniTest = IS_TEXT_UNICODE_STATISTICS;
					
					// Detect UTF-16BE with BOM (0xFE 0xFF)
#pragma warning(suppress: 26446)
					if ((nBytesRead > 1) && ((nFileSize % 2) == 0) && ((nBytesRead % 2) == 0) && (byBuffer[0] == 0xFE) && (byBuffer[1] == 0xFF))
					{
						m_BOM = BOM::UTF16BE;
						nSkip = 2; // Skip BOM bytes
					}
					// Detect UTF-16LE with BOM (0xFF 0xFE)
#pragma warning(suppress: 26446)
					else if ((nBytesRead > 1) && ((nFileSize % 2) == 0) && ((nBytesRead % 2) == 0) && (byBuffer[0] == 0xFF) && (byBuffer[1] == 0xFE))
					{
						m_BOM = BOM::UTF16LE;
						nSkip = 2; // Skip BOM bytes
					}
					// Detect UTF-8 with BOM (0xEF 0xBB 0xBF)
#pragma warning(suppress: 26446)
					else if ((nBytesRead > 2) && (byBuffer[0] == 0xEF) && (byBuffer[1] == 0xBB) && (byBuffer[2] == 0xBF))
					{
						m_BOM = BOM::UTF8;
						nSkip = 3; // Skip BOM bytes
					}
					// Detect UTF-16LE without BOM using heuristics
					// (Note: IS_TEXT_UNICODE_STATISTICS implies Little Endian on all supported Windows platforms)
#pragma warning(suppress: 26446)
					else if ((nBytesRead > 1) && ((nFileSize % 2) == 0) && ((nBytesRead % 2) == 0) && (byBuffer[0] != 0) && (byBuffer[1] == 0) && IsTextUnicode(byBuffer.data(), nBytesRead, &nUniTest))
					{
						m_BOM = BOM::UTF16LE_NOBOM;
						nSkip = 0; // No BOM to skip
					}
				}

				// Convert data to UTF-8 based on detected encoding
				const char* pLoadData{ nullptr };
				int nBytesToLoad{ 0 };
				CStringA sUTF8Data;
				
				if ((m_BOM == BOM::UTF16LE_NOBOM) || (m_BOM == BOM::UTF16LE))
				{
					// Handle conversion from UTF-16LE to UTF-8
#pragma warning(suppress: 26481 26490)
					sUTF8Data = W2UTF8(reinterpret_cast<const wchar_t*>(byBuffer.data() + nSkip), (nBytesRead - nSkip) / 2);
					pLoadData = sUTF8Data;
					nBytesToLoad = sUTF8Data.GetLength();
				}
				else if (m_BOM == BOM::UTF16BE)
				{
					// Handle conversion from UTF-16BE to UTF-8
					// First swap byte order from big-endian to little-endian
#pragma warning(suppress: 26429 26481)
					BYTE* p = byBuffer.data() + nSkip;
					const int nUTF16CharsRead = (nBytesRead - nSkip) / 2;
					for (int i = 0; i < nUTF16CharsRead; i++)
					{
						const BYTE t{ *p };
#pragma warning(suppress: 26481)
						*p = p[1];
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
					// Assume UTF-8 or ASCII encoding - use data as-is
#pragma warning(suppress: 26481 26490)
					pLoadData = reinterpret_cast<const char*>(byBuffer.data()) + nSkip;
					nBytesToLoad = nBytesRead - nSkip;
				}
				
				ASSERT(pLoadData != nullptr);
				// Convert UTF-8 to wide string and add to the terminal view
				AddText(utf8_to_wstring(pLoadData).c_str());
			}
		} while (nBytesRead);
	}
	else
	{
		// Saving: Write file content with UTF-8 BOM
#pragma warning(suppress: 26429)
		CFile* pFile{ ar.GetFile() };
		ASSERT(pFile != nullptr);

		ASSERT(!m_viewList.IsEmpty());
		CEditView* pEditView = reinterpret_cast<CEditView*>(m_viewList.GetHead());
		if (pEditView != nullptr)
		{
			// Write UTF-8 BOM (0xEF 0xBB 0xBF) at the beginning of the file
			pFile->Write("\xEF\xBB\xBF", 3);
			
			// Get the window content
			CStringW strBuffer;
			pEditView->GetEditCtrl().GetWindowText(strBuffer);
			const std::wstring strRawText(strBuffer);
			
			// Convert Unicode characters to UTF-8
			CStringA strOutput(wstring_to_utf8(strRawText).c_str());
			UINT nLength = strOutput.GetLength();
			
			// Write the UTF-8 encoded content to file
			pFile->Write(strOutput.GetBuffer(nLength), nLength);
			strOutput.ReleaseBuffer();
		}
	}

	// Update status bar with operation result
	CString strFormat, strMessage;
	CMainFrame* pMainFrame = (CMainFrame*) AfxGetMainWnd();
	ASSERT_VALID(pMainFrame);
	VERIFY(strFormat.LoadString(ar.IsLoading() ? IDS_FILE_HAS_BEEN_LOADED : IDS_FILE_HAS_BEEN_SAVED));
	strMessage.Format(strFormat, static_cast<LPCWSTR>(ar.m_strFileName));
	pMainFrame->SetStatusBarText(strMessage);

#ifdef SHARED_HANDLERS
	// Handle thumbnail and search content for shell integration
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
/**
 * Draws a thumbnail representation of the document for Windows Explorer preview.
 * @param dc Device context for drawing
 * @param lprcBounds Bounding rectangle for the thumbnail
 */
void CIntelliPortDoc::OnDrawThumbnail(CDC& dc, LPRECT lprcBounds)
{
	// Draw white background
	dc.FillSolidRect(lprcBounds, RGB(255, 255, 255));

	LOGFONT lf;

	// Use system default font as base
	CFont* pDefaultGUIFont = CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT));
	pDefaultGUIFont->GetLogFont(&lf);
	lf.lfHeight = 36; // Increase font size for thumbnail visibility

	CFont fontDraw;
	fontDraw.CreateFontIndirect(&lf);

	// Draw document content in the thumbnail
	CFont* pOldFont = dc.SelectObject(&fontDraw);
	dc.DrawText(m_strThumbnailContent, lprcBounds, DT_CENTER | DT_WORDBREAK);
	dc.SelectObject(pOldFont);
}

/**
 * Initializes search content for Windows Search integration.
 */
void CIntelliPortDoc::InitializeSearchContent()
{
	// Set search contents from document's data
	// The content parts should be separated by ";"

	// Use the entire text file content as the search content
	SetSearchContent(m_strSearchContent);
}

/**
 * Sets the search content for the document to enable Windows Search indexing.
 * @param value The content to be indexed for search
 */
void CIntelliPortDoc::SetSearchContent(const CString& value)
{
	if (value.IsEmpty())
	{
		// Remove search content if empty
		RemoveChunk(PKEY_Search_Contents.fmtid, PKEY_Search_Contents.pid);
	}
	else
	{
		// Create and set search chunk
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
/**
 * Validates the document object in debug builds.
 */
void CIntelliPortDoc::AssertValid() const
{
	CDocument::AssertValid();
}

/**
 * Dumps diagnostic information about the document to the dump context.
 * @param dc Dump context for output
 */
void CIntelliPortDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

// CIntelliPortDoc commands
