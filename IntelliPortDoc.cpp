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
 * @brief Converts a wide character (Unicode) string to UTF-8 encoding.
 * 
 * Performs a two-phase conversion:
 * 1. Calculates the required buffer size for the UTF-8 representation
 * 2. Converts the wide character string to UTF-8
 * 
 * The function handles both null-terminated strings (nLength = -1) and
 * strings with explicit length. Ensures proper null termination in all cases.
 * 
 * @param pszText Pointer to the wide character string to convert.
 * @param nLength Length of the input string in characters, or -1 for null-terminated strings.
 * @return UTF-8 encoded string as CStringA.
 */
CStringA W2UTF8(_In_NLS_string_(nLength) const wchar_t* pszText, _In_ int nLength)
{
	// Phase 1: Calculate required buffer size
	// Pass NULL buffer to get size needed for conversion
	int nUTF8Length{ WideCharToMultiByte(CP_UTF8, 0, pszText, nLength, nullptr, 0, nullptr, nullptr) };

	// Ensure minimum size for null terminator
	if (nUTF8Length == 0)
		nUTF8Length = 1;

	// Phase 2: Perform actual conversion
	CStringA sUTF;
#pragma warning(suppress: 26429)
	// Allocate buffer with extra byte for manual null termination if needed
	char* const pszUTF8Text{ sUTF.GetBuffer(nUTF8Length + 1) };
	// Convert wide string to UTF-8
	int nCharsWritten{ WideCharToMultiByte(CP_UTF8, 0, pszText, nLength, pszUTF8Text, nUTF8Length, nullptr, nullptr) };

	// Add null terminator if not automatically added (when nLength != -1)
	if (nLength != -1)
	{
#pragma warning(suppress: 26477 26496)
		ATLASSUME(nCharsWritten <= nUTF8Length);
#pragma warning(suppress: 26481)
		pszUTF8Text[nCharsWritten] = '\0';
	}
	// Release buffer and return converted string
	sUTF.ReleaseBuffer();

	return sUTF;
}

/**
 * @brief Converts a UTF-8 encoded string to wide character (Unicode) format.
 * 
 * Performs a two-phase conversion:
 * 1. Calculates the required buffer size for the wide character representation
 * 2. Converts the UTF-8 string to wide characters
 * 
 * The function handles both null-terminated strings (nLength = -1) and
 * strings with explicit length. Ensures proper null termination in all cases.
 * 
 * @param pszText Pointer to the UTF-8 encoded string to convert.
 * @param nLength Length of the input string in bytes, or -1 for null-terminated strings.
 * @return Wide character string as CStringW.
 */
CStringW UTF82W(_In_NLS_string_(nLength) const char* pszText, _In_ int nLength)
{
	// Phase 1: Calculate required buffer size
	// Pass NULL buffer to get size needed for conversion
	int nWideLength{ MultiByteToWideChar(CP_UTF8, 0, pszText, nLength, nullptr, 0) };

	// Ensure minimum size for null terminator
	if (nWideLength == 0)
		nWideLength = 1;

	// Phase 2: Perform actual conversion
	CStringW sWideString;
#pragma warning(suppress: 26429)
	// Allocate buffer with extra character for manual null termination if needed
	wchar_t* pszWText{ sWideString.GetBuffer(nWideLength + 1) };
	// Convert UTF-8 to wide string
	int nCharsWritten{ MultiByteToWideChar(CP_UTF8, 0, pszText, nLength, pszWText, nWideLength) };

	// Add null terminator if not automatically added (when nLength != -1)
	if (nLength != -1)
	{
#pragma warning(suppress: 26477 26496)
		ATLASSUME(nCharsWritten <= nWideLength);
#pragma warning(suppress: 26481)
		pszWText[nCharsWritten] = '\0';
	}
	// Release buffer and return converted string
	sWideString.ReleaseBuffer();

	return sWideString;
}

/**
 * @class CIntelliPortDoc
 * @brief Document class for managing terminal content in IntelliPort.
 * 
 * This class handles:
 * - Document creation and initialization
 * - Terminal text display with custom font (Consolas)
 * - File loading and saving with multiple text encoding support:
 *   * UTF-8 with BOM (Byte Order Mark)
 *   * UTF-16LE (Little Endian) with BOM
 *   * UTF-16BE (Big Endian) with BOM
 *   * UTF-16LE without BOM (auto-detected)
 *   * ASCII/ANSI encoding
 * - Text serialization and encoding conversion
 * - Automatic BOM detection when loading files
 * - Adding text to terminal view with line ending normalization
 * - Shell integration (thumbnails, search content)
 */

// Enable dynamic creation of this document class
IMPLEMENT_DYNCREATE(CIntelliPortDoc, CDocument)

// Message map - connects commands to handler functions
BEGIN_MESSAGE_MAP(CIntelliPortDoc, CDocument)
	ON_COMMAND(ID_FILE_SEND_MAIL, &CIntelliPortDoc::OnFileSendMail)  // Handle "Send as Email" command
	ON_UPDATE_COMMAND_UI(ID_FILE_SEND_MAIL, &CIntelliPortDoc::OnUpdateFileSendMail)  // Update UI state
END_MESSAGE_MAP()

/**
 * @brief Constructor for CIntelliPortDoc.
 * 
 * Initializes the document and creates a terminal font:
 * - Sets BOM (Byte Order Mark) to unknown state
 * - Creates Consolas font (10pt, monospace) for terminal display
 * - Configures font with antialiasing for better readability
 * 
 * The Consolas font is ideal for terminal/console applications due to
 * its clear character differentiation (0 vs O, 1 vs l vs I, etc.).
 */
CIntelliPortDoc::CIntelliPortDoc()
{
	// Initialize BOM (Byte Order Mark) to unknown state
	// Will be detected when loading files
	m_BOM = BOM::Unknown;

	// Create terminal font with Consolas typeface (monospace font)
	VERIFY(m_fontTerminal.CreateFont(
		-MulDiv(10, GetDeviceCaps(::GetDC(nullptr), LOGPIXELSY), 72), // nHeight - 10pt font size
		0,                         // nWidth - use default width (maintain aspect ratio)
		0,                         // nEscapement - no text rotation
		0,                         // nOrientation - no baseline rotation
		FW_NORMAL,                 // nWeight - normal weight (not bold)
		FALSE,                     // bItalic - not italic
		FALSE,                     // bUnderline - not underlined
		0,                         // cStrikeOut - no strikethrough
		ANSI_CHARSET,              // nCharSet - ANSI character set
		OUT_DEFAULT_PRECIS,        // nOutPrecision - default output precision
		CLIP_DEFAULT_PRECIS,       // nClipPrecision - default clipping precision
		DEFAULT_QUALITY,           // nQuality - default quality (antialiased)
		DEFAULT_PITCH | FF_MODERN, // nPitchAndFamily - fixed-pitch modern font
		_T("Consolas")));          // lpszFacename - Consolas font family
}

/**
 * @brief Destructor for CIntelliPortDoc.
 * 
 * Cleans up resources allocated by the document:
 * - Deletes the terminal font object (m_fontTerminal)
 * - Releases GDI resources
 */
CIntelliPortDoc::~CIntelliPortDoc()
{
	// Clean up the terminal font object
	VERIFY(m_fontTerminal.DeleteObject());
}

/**
 * @brief Called when a new document is created.
 * 
 * Initializes the document for terminal display:
 * - Calls base class to perform standard initialization
 * - Clears any existing text in the view
 * - Sets the terminal font (Consolas) for the edit control
 * - Makes the edit control read-only (terminal output)
 * - Removes text length limit for unlimited terminal output
 * 
 * This method is called when:
 * - The application starts (initial document in SDI)
 * - User selects File->New
 * - A new SDI document reuses the existing document object
 * 
 * @return TRUE if document was successfully initialized, FALSE otherwise.
 */
BOOL CIntelliPortDoc::OnNewDocument()
{
	// Call base class for standard document initialization
	if (!CDocument::OnNewDocument())
		return FALSE;

	// Clear existing text from the view (important for SDI document reuse)
	if (!m_viewList.IsEmpty())
	{
		reinterpret_cast<CEditView*>(m_viewList.GetHead())->SetWindowText(nullptr);
	}

	// Configure edit control for terminal display
	// Set monospace font for better alignment and readability
	reinterpret_cast<CEditView*>(m_viewList.GetHead())->GetEditCtrl().SetFont(&m_fontTerminal);
	// Make read-only to prevent user editing of terminal output
	reinterpret_cast<CEditView*>(m_viewList.GetHead())->GetEditCtrl().SetReadOnly(TRUE);
	// Remove text length limit (default is 32KB) for unlimited terminal output
	reinterpret_cast<CEditView*>(m_viewList.GetHead())->GetEditCtrl().LimitText(-1);

	return TRUE;
}

/**
 * @brief Adds text to the terminal view with line ending normalization.
 * 
 * Performs the following operations:
 * 1. Normalizes all line endings to Windows standard (CRLF)
 * 2. Appends text to the end of the edit control
 * 3. Resets text selection
 * 
 * Line ending normalization process:
 * - First: CRLF → LF (collapse existing Windows line endings)
 * - Then: CR → LF (convert Mac-style line endings)
 * - Finally: LF → CRLF (convert all to Windows standard)
 * 
 * This ensures consistent display regardless of source line ending format
 * (Unix/Linux LF, Classic Mac CR, or Windows CRLF).
 * 
 * @param strText The text to add to the terminal display.
 * @return true if text was successfully added, false if view list is empty.
 */
bool CIntelliPortDoc::AddText(CString strText)
{
	// Check if view exists
	if (!m_viewList.IsEmpty())
	{
		// Get the first (and only in SDI) view
		CEditView* pEditView = reinterpret_cast<CEditView*>(m_viewList.GetHead());
		if (pEditView != nullptr)
		{
			// Normalize line endings to Windows standard (CRLF)
			// Step 1: CRLF → LF (avoid double conversion)
			strText.Replace(CRLF, LF);
			// Step 2: CR → LF (Mac-style line endings)
			strText.Replace(CR, LF);
			// Step 3: LF → CRLF (all line endings now standardized)
			strText.Replace(LF, CRLF);

			// Append text to end of edit control
			CEdit& pEdit = pEditView->GetEditCtrl();
			// Get current text length to find insertion point
			int outLength = pEdit.GetWindowTextLength();
			// Move cursor/selection to end
			pEdit.SetSel(outLength, outLength);
			// Insert text at cursor (TRUE = can undo)
			pEdit.ReplaceSel(strText, TRUE);
			// Clear selection (SetSel(-1, 0) deselects all)
			pEdit.SetSel(-1, 0);
			return true;
		}
	}
	return false;
}

/**
 * @brief Handles document serialization for loading and saving files.
 * 
 * Loading (ar.IsLoading()):
 * - Reads entire file into memory
 * - Detects text encoding by examining Byte Order Mark (BOM):
 *   * UTF-16BE: 0xFE 0xFF
 *   * UTF-16LE: 0xFF 0xFE
 *   * UTF-8: 0xEF 0xBB 0xBF
 *   * UTF-16LE (no BOM): Auto-detected using IsTextUnicode heuristics
 *   * Default: Assumes UTF-8 or ASCII
 * - Converts all text to UTF-8 internally
 * - Displays text in the view using AddText()
 * 
 * Saving (!ar.IsLoading()):
 * - Writes UTF-8 BOM (0xEF 0xBB 0xBF) at file start
 * - Gets current text from edit control
 * - Converts from Unicode to UTF-8
 * - Writes UTF-8 encoded content to file
 * 
 * After load/save, updates status bar with success message.
 * 
 * @param ar Archive object for reading/writing file data.
 */
void CIntelliPortDoc::Serialize(CArchive& ar)
{
	if (ar.IsLoading())
	{
		// ===== LOADING: Read file and detect encoding =====
#pragma warning(suppress: 26429)
		CFile* pFile{ ar.GetFile() };
		ASSERT(pFile != nullptr);
		// Get total file size for buffer allocation
		const auto nFileSize = pFile->GetLength();

		// Allocate buffer to hold entire file content
		std::vector<BYTE> byBuffer{ static_cast<UINT>(nFileSize), std::allocator<BYTE>{} };
		int nBytesRead{ 0 };
		// Reset BOM detection state
		m_BOM = BOM::Unknown;
		bool bDetectedBOM{ false };
		
					do
					{
		#pragma warning(suppress: 26472)
						// Read file content into buffer
						nBytesRead = pFile->Read(byBuffer.data(), static_cast<UINT>(byBuffer.size()));
						if (nBytesRead)
						{
							// Number of BOM bytes to skip after detection
							int nSkip{ 0 };

							// ===== BOM Detection (performed once on first read) =====
							if (!bDetectedBOM)
							{
								bDetectedBOM = true;
								// Flag for IsTextUnicode API (statistical analysis)
								int nUniTest = IS_TEXT_UNICODE_STATISTICS;

								// Check for UTF-16 Big Endian BOM (0xFE 0xFF)
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

				// ===== Convert detected encoding to UTF-8 =====
				const char* pLoadData{ nullptr };
				int nBytesToLoad{ 0 };
				CStringA sUTF8Data;

				// Handle UTF-16 Little Endian (with or without BOM)
				if ((m_BOM == BOM::UTF16LE_NOBOM) || (m_BOM == BOM::UTF16LE))
				{
					// Handle conversion from UTF-16LE to UTF-8
#pragma warning(suppress: 26481 26490)
					sUTF8Data = W2UTF8(reinterpret_cast<const wchar_t*>(byBuffer.data() + nSkip), (nBytesRead - nSkip) / 2);
					pLoadData = sUTF8Data;
					nBytesToLoad = sUTF8Data.GetLength();
									}
									// Handle UTF-16 Big Endian
									else if (m_BOM == BOM::UTF16BE)
									{
										// UTF-16BE requires byte swapping to convert to Little Endian
										// Windows APIs expect Little Endian UTF-16
				#pragma warning(suppress: 26429 26481)
										// Point to first character after BOM
										BYTE* p = byBuffer.data() + nSkip;
										// Calculate number of UTF-16 characters (2 bytes each)
										const int nUTF16CharsRead = (nBytesRead - nSkip) / 2;
										// Swap bytes for each character (Big Endian → Little Endian)
										for (int i = 0; i < nUTF16CharsRead; i++)
										{
											// Temporary storage for byte swap
											const BYTE t{ *p };
				#pragma warning(suppress: 26481)
											*p = p[1];      // Move low byte to high position
				#pragma warning(suppress: 26481)
											p[1] = t;       // Move high byte to low position
				#pragma warning(suppress: 26481)
											p += 2;         // Advance to next character
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
		// ===== SAVING: Write UTF-8 encoded file with BOM =====
#pragma warning(suppress: 26429)
		CFile* pFile{ ar.GetFile() };
		ASSERT(pFile != nullptr);

		ASSERT(!m_viewList.IsEmpty());
		// Get the edit view to retrieve content
		CEditView* pEditView = reinterpret_cast<CEditView*>(m_viewList.GetHead());
		if (pEditView != nullptr)
		{
			// Write UTF-8 BOM (0xEF 0xBB 0xBF) to mark file as UTF-8
			// This helps text editors recognize the encoding
			pFile->Write("\xEF\xBB\xBF", 3);

			// Get all text from the edit control
			CStringW strBuffer;
			pEditView->GetEditCtrl().GetWindowText(strBuffer);
			const std::wstring strRawText(strBuffer);

			// Convert from Unicode (UTF-16) to UTF-8
			CStringA strOutput(wstring_to_utf8(strRawText).c_str());
			UINT nLength = strOutput.GetLength();

			// Write UTF-8 encoded bytes to file
			pFile->Write(strOutput.GetBuffer(nLength), nLength);
			strOutput.ReleaseBuffer();
		}
	}

	// ===== Update status bar with success message =====
	CString strFormat, strMessage;
	// Get main frame to access status bar
	CMainFrame* pMainFrame = (CMainFrame*) AfxGetMainWnd();
	ASSERT_VALID(pMainFrame);
	// Load appropriate message string ("File loaded" or "File saved")
	VERIFY(strFormat.LoadString(ar.IsLoading() ? IDS_FILE_HAS_BEEN_LOADED : IDS_FILE_HAS_BEEN_SAVED));
	// Format message with filename
	strMessage.Format(strFormat, static_cast<LPCWSTR>(ar.m_strFileName));
	// Display in status bar
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
 * @brief Draws a thumbnail representation of the document.
 * 
 * Called by Windows Shell to generate a thumbnail preview for:
 * - Windows Explorer thumbnail view
 * - File Explorer preview pane
 * - Task switcher (Alt+Tab) previews
 * 
 * Draws the first portion of the document text on a white background
 * using an enlarged font for visibility in the thumbnail.
 * 
 * @param dc Device context for drawing the thumbnail.
 * @param lprcBounds Bounding rectangle for the thumbnail image.
 */
void CIntelliPortDoc::OnDrawThumbnail(CDC& dc, LPRECT lprcBounds)
{
	// Fill background with white color
	dc.FillSolidRect(lprcBounds, RGB(255, 255, 255));

	LOGFONT lf;

	// Get system default GUI font and modify for thumbnail display
	CFont* pDefaultGUIFont = CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT));
	pDefaultGUIFont->GetLogFont(&lf);
	// Increase font size to 36pt for better visibility in thumbnail
	lf.lfHeight = 36;

	// Create the enlarged font
	CFont fontDraw;
	fontDraw.CreateFontIndirect(&lf);

	// Draw the document content
	CFont* pOldFont = dc.SelectObject(&fontDraw);
	// DT_CENTER = center horizontally, DT_WORDBREAK = wrap words
	dc.DrawText(m_strThumbnailContent, lprcBounds, DT_CENTER | DT_WORDBREAK);
	// Restore original font
	dc.SelectObject(pOldFont);
}

/**
 * @brief Initializes search content for Windows Search integration.
 * 
 * Called by the Windows Search indexer to extract searchable content
 * from the document. The search content allows users to find files
 * using Windows Search based on their text content.
 * 
 * Sets the entire document text as searchable content.
 */
void CIntelliPortDoc::InitializeSearchContent()
{
	// Provide document content to Windows Search indexer
	// Multiple content parts can be separated by semicolons
	SetSearchContent(m_strSearchContent);
}

/**
 * @brief Sets the search content for Windows Search indexing.
 * 
 * Creates a search chunk that Windows Search can index.
 * If the content is empty, removes the existing search chunk.
 * Otherwise, creates a new text chunk with the PKEY_Search_Contents property.
 * 
 * @param value The text content to be indexed for search.
 */
void CIntelliPortDoc::SetSearchContent(const CString& value)
{
	if (value.IsEmpty())
	{
		// No content to index - remove existing search chunk
		RemoveChunk(PKEY_Search_Contents.fmtid, PKEY_Search_Contents.pid);
	}
	else
	{
		// Create a filter chunk for Windows Search indexing
		CMFCFilterChunkValueImpl *pChunk = NULL;
		ATLTRY(pChunk = new CMFCFilterChunkValueImpl);
		if (pChunk != NULL)
		{
			// Set the text content with PKEY_Search_Contents property
			// CHUNK_TEXT indicates this is text content (not binary)
			pChunk->SetTextValue(PKEY_Search_Contents, value, CHUNK_TEXT);
			// Register the chunk with the filter
			SetChunkValue(pChunk);
		}
	}
}
#endif // SHARED_HANDLERS

// CIntelliPortDoc diagnostics

#ifdef _DEBUG
/**
 * @brief Validates the document object in debug builds.
 * 
 * Performs diagnostic validation to ensure the object is in a valid state.
 * Checks object integrity and verifies internal data structures.
 * Called by MFC's debugging facilities to detect memory corruption
 * or invalid object states.
 */
void CIntelliPortDoc::AssertValid() const
{
	CDocument::AssertValid();
}

/**
 * @brief Dumps diagnostic information about the document.
 * 
 * Outputs diagnostic information to the specified dump context for debugging.
 * Includes object state, member variables, and relationships to other objects.
 * Called by MFC's diagnostic facilities when using afxDump or TRACE statements.
 * 
 * @param dc Reference to the CDumpContext for outputting diagnostic information.
 */
void CIntelliPortDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

// CIntelliPortDoc commands
