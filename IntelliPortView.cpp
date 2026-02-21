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

// IntelliPortView.cpp : implementation of the CIntelliPortView class
//

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "IntelliPort.h"
#endif

#include "IntelliPortDoc.h"
#include "IntelliPortView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/**
 * @class CIntelliPortView
 * @brief View class for displaying terminal output in IntelliPort.
 * 
 * This class extends CEditView to provide a text display area for:
 * - Displaying received data from serial port or network socket
 * - Showing formatted text with proper line endings
 * - Providing context menu for text operations (copy, select all)
 * - Supporting printing and print preview functionality
 * - Word-wrapping text without horizontal scrolling
 * 
 * The view uses an edit control as the underlying text display mechanism,
 * allowing users to select and copy text but not edit it during active connections.
 */

// Enable dynamic creation of this view class
IMPLEMENT_DYNCREATE(CIntelliPortView, CEditView)

// Message map - connects Windows messages and commands to handler functions
BEGIN_MESSAGE_MAP(CIntelliPortView, CEditView)
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, &CEditView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CEditView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CIntelliPortView::OnFilePrintPreview)
	// Context menu handling
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
END_MESSAGE_MAP()

/**
 * @brief Constructor for CIntelliPortView.
 * 
 * Initializes the view component for displaying terminal output.
 * The base class CEditView handles the creation and initialization
 * of the underlying edit control.
 */
CIntelliPortView::CIntelliPortView()
{
	// Constructor body is empty - initialization is handled by base class
}

/**
 * @brief Destructor for CIntelliPortView.
 * 
 * Cleans up any resources allocated by the view.
 * The base class CEditView handles cleanup of the edit control.
 */
CIntelliPortView::~CIntelliPortView()
{
	// Destructor body is empty - cleanup is handled by base class
}

/**
 * @brief Called before the window is created to modify window styles.
 * 
 * Customizes the edit control behavior:
 * - Disables horizontal scrolling (ES_AUTOHSCROLL flag removed)
 * - Removes horizontal scroll bar (WS_HSCROLL style removed)
 * - Enables automatic word-wrapping to fit text within window width
 * 
 * This ensures all terminal output is visible without requiring
 * horizontal scrolling, making it easier to read long lines of text.
 * 
 * @param cs Reference to CREATESTRUCT containing window creation parameters.
 * @return TRUE if the window should be created, FALSE to prevent creation.
 */
BOOL CIntelliPortView::PreCreateWindow(CREATESTRUCT& cs)
{
	// Call base class to perform standard initialization
	BOOL bPreCreated = CEditView::PreCreateWindow(cs);

	// Remove horizontal scroll styles to enable word-wrapping
	// ES_AUTOHSCROLL = automatically scroll horizontally when typing
	// WS_HSCROLL = horizontal scroll bar
	cs.style &= ~(ES_AUTOHSCROLL|WS_HSCROLL);

	return bPreCreated;
}

/**
 * @brief Displays the print preview dialog for the current document.
 * 
 * Launches the MFC print preview framework to show a visual preview
 * of how the terminal output will appear when printed to paper.
 * Allows users to see page breaks, margins, and overall layout
 * before committing to a physical print job.
 */
void CIntelliPortView::OnFilePrintPreview()
{
#ifndef SHARED_HANDLERS
	// Launch the MFC print preview dialog with this view
	AFXPrintPreview(this);
#endif
}

/**
 * @brief Prepares the view for printing by initializing print settings.
 * 
 * Called by the MFC framework before printing or print preview begins.
 * Allows customization of print settings such as:
 * - Page range selection
 * - Number of copies
 * - Printer selection
 * - Page orientation
 * 
 * @param pInfo Pointer to CPrintInfo structure containing print job information.
 * @return TRUE if printing should proceed, FALSE to cancel the print job.
 */
BOOL CIntelliPortView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// Use default CEditView preparation (shows print dialog)
	return CEditView::OnPreparePrinting(pInfo);
}

/**
 * @brief Called by the framework when printing begins.
 * 
 * Performs initialization tasks required before the first page is printed:
 * - Allocates GDI resources for printing
 * - Calculates page layout and line spacing
 * - Prepares text formatting for the printer device context
 * 
 * @param pDC Pointer to the printer device context.
 * @param pInfo Pointer to CPrintInfo structure containing print job information.
 */
void CIntelliPortView::OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo)
{
	// Delegate to base class for standard initialization
	CEditView::OnBeginPrinting(pDC, pInfo);
}

/**
 * @brief Called by the framework when printing ends.
 * 
 * Performs cleanup tasks after the last page is printed:
 * - Releases GDI resources allocated during printing
 * - Cleans up temporary print-related data structures
 * - Restores view state after printing completes
 * 
 * @param pDC Pointer to the printer device context.
 * @param pInfo Pointer to CPrintInfo structure containing print job information.
 */
void CIntelliPortView::OnEndPrinting(CDC* pDC, CPrintInfo* pInfo)
{
	// Delegate to base class for standard cleanup
	CEditView::OnEndPrinting(pDC, pInfo);
}

/**
 * @brief Handles right mouse button release events.
 * 
 * Called when the user releases the right mouse button in the view.
 * Converts the click location from client coordinates to screen coordinates
 * and delegates to OnContextMenu to display the popup menu.
 * 
 * @param nFlags Indicates which virtual keys are down (not used in this implementation).
 * @param point The cursor position in client coordinates where the click occurred.
 */
void CIntelliPortView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	// Convert client coordinates (relative to window) to screen coordinates (absolute)
	ClientToScreen(&point);
	// Display the context menu at the calculated screen position
	OnContextMenu(this, point);
}

/**
 * @brief Displays a context menu at the specified location.
 * 
 * Shows the edit popup menu (IDR_POPUP_EDIT) with standard editing commands:
 * - Copy: Copy selected text to clipboard
 * - Select All: Select all text in the view
 * - Other standard edit operations
 * 
 * The menu is positioned at the provided screen coordinates and is
 * right-aligned for proper display near screen edges.
 * 
 * @param pWnd Pointer to the window that received the context menu request (not used).
 * @param point The position where the context menu should be displayed in screen coordinates.
 */
void CIntelliPortView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
	// Display the edit context menu at the specified location
	// IDR_POPUP_EDIT = resource ID for edit menu
	// point.x, point.y = screen coordinates for menu position
	// this = window to receive menu commands
	// TRUE = right-align menu if near screen edge
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}

// CIntelliPortView diagnostics

#ifdef _DEBUG
/**
 * @brief Validates the view object in debug builds.
 * 
 * Performs diagnostic validation to ensure the object is in a valid state.
 * Checks object integrity and verifies that internal data structures
 * are consistent. Called by MFC's debugging facilities to detect
 * memory corruption or invalid object states.
 */
void CIntelliPortView::AssertValid() const
{
	// Validate base class state first
	CEditView::AssertValid();
}

/**
 * @brief Dumps diagnostic information about the view.
 * 
 * Outputs diagnostic information to the specified dump context for debugging.
 * Includes object state, member variables, and relationships to other objects.
 * Called by MFC's diagnostic facilities when using afxDump or TRACE statements.
 * 
 * @param dc Reference to the CDumpContext for outputting diagnostic information.
 */
void CIntelliPortView::Dump(CDumpContext& dc) const
{
	// Dump base class information first
	CEditView::Dump(dc);
}

/**
 * @brief Retrieves a pointer to the document associated with this view.
 * 
 * This is the debug version that performs runtime type checking to ensure
 * the document is actually a CIntelliPortDoc object. The non-debug version
 * is defined as an inline function in the header file for better performance.
 * 
 * @return Pointer to the CIntelliPortDoc document object associated with this view.
 */
CIntelliPortDoc* CIntelliPortView::GetDocument() const // non-debug version is inline
{
	// Verify the document pointer is valid and of the correct type
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CIntelliPortDoc)));
	// Safely cast and return the document pointer
	return (CIntelliPortDoc*)m_pDocument;
}
#endif //_DEBUG

// CIntelliPortView message handlers
