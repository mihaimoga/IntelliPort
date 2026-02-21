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

// CIntelliPortView

IMPLEMENT_DYNCREATE(CIntelliPortView, CEditView)

BEGIN_MESSAGE_MAP(CIntelliPortView, CEditView)
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, &CEditView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CEditView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CIntelliPortView::OnFilePrintPreview)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
END_MESSAGE_MAP()

// CIntelliPortView construction/destruction

/**
 * Default constructor for CIntelliPortView.
 * Initializes the view component for displaying terminal output.
 */
CIntelliPortView::CIntelliPortView()
{
	// Constructor body is empty - initialization is handled by base class
}

/**
 * Destructor for CIntelliPortView.
 * Cleans up any resources allocated by the view.
 */
CIntelliPortView::~CIntelliPortView()
{
	// Destructor body is empty - cleanup is handled by base class
}

/**
 * Called before the window is created. Allows modification of window styles.
 * Disables horizontal scrolling and enables word-wrapping for the edit control.
 * @param cs Reference to the CREATESTRUCT structure containing window creation parameters
 * @return TRUE if the window should be created, FALSE otherwise
 */
BOOL CIntelliPortView::PreCreateWindow(CREATESTRUCT& cs)
{
	// Call base class implementation first
	BOOL bPreCreated = CEditView::PreCreateWindow(cs);
	
	// Remove horizontal scroll bar and auto-horizontal scroll to enable word-wrapping
	cs.style &= ~(ES_AUTOHSCROLL|WS_HSCROLL);

	return bPreCreated;
}

// CIntelliPortView printing

/**
 * Displays the print preview dialog for the current document.
 * Uses the MFC print preview framework to show a preview of how the document will look when printed.
 */
void CIntelliPortView::OnFilePrintPreview()
{
#ifndef SHARED_HANDLERS
	// Launch the print preview dialog
	AFXPrintPreview(this);
#endif
}

/**
 * Prepares the view for printing by initializing print settings.
 * Called by the framework before printing or print preview begins.
 * @param pInfo Pointer to CPrintInfo structure containing print job information
 * @return TRUE if printing should proceed, FALSE to cancel
 */
BOOL CIntelliPortView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// Use default CEditView preparation logic
	return CEditView::OnPreparePrinting(pInfo);
}

/**
 * Called by the framework when printing begins.
 * Performs initialization tasks required before the first page is printed.
 * @param pDC Pointer to the printer device context
 * @param pInfo Pointer to CPrintInfo structure containing print job information
 */
void CIntelliPortView::OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo)
{
	// Delegate to base class implementation for default begin printing behavior
	CEditView::OnBeginPrinting(pDC, pInfo);
}

/**
 * Called by the framework when printing ends.
 * Performs cleanup tasks after the last page is printed.
 * @param pDC Pointer to the printer device context
 * @param pInfo Pointer to CPrintInfo structure containing print job information
 */
void CIntelliPortView::OnEndPrinting(CDC* pDC, CPrintInfo* pInfo)
{
	// Delegate to base class implementation for default end printing behavior
	CEditView::OnEndPrinting(pDC, pInfo);
}

/**
 * Handles right mouse button release events.
 * Converts the click point to screen coordinates and displays the context menu.
 * @param nFlags Indicates which virtual keys are down (not used in this implementation)
 * @param point The cursor position in client coordinates
 */
void CIntelliPortView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	// Convert client coordinates to screen coordinates for context menu positioning
	ClientToScreen(&point);
	// Display the context menu at the click location
	OnContextMenu(this, point);
}

/**
 * Displays a context menu at the specified location.
 * Shows the edit popup menu with standard editing commands (copy, select all, etc.).
 * @param pWnd Pointer to the window that received the context menu request (not used)
 * @param point The position where the context menu should be displayed in screen coordinates
 */
void CIntelliPortView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
	// Show the edit context menu (IDR_POPUP_EDIT) at the specified screen coordinates
	// The TRUE parameter indicates that the menu should be right-aligned
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}

// CIntelliPortView diagnostics

#ifdef _DEBUG
/**
 * Validates the view object in debug builds.
 * Performs consistency checks to ensure the object is in a valid state.
 */
void CIntelliPortView::AssertValid() const
{
	// Validate base class state
	CEditView::AssertValid();
}

/**
 * Dumps diagnostic information about the view to the dump context.
 * Used for debugging purposes to output object state information.
 * @param dc Reference to the CDumpContext for outputting diagnostic information
 */
void CIntelliPortView::Dump(CDumpContext& dc) const
{
	// Dump base class information
	CEditView::Dump(dc);
}

/**
 * Retrieves a pointer to the document associated with this view.
 * This is the debug version that performs type checking.
 * @return Pointer to the CIntelliPortDoc document object
 */
CIntelliPortDoc* CIntelliPortView::GetDocument() const // non-debug version is inline
{
	// Verify the document is of the correct type
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CIntelliPortDoc)));
	// Cast and return the document pointer
	return (CIntelliPortDoc*)m_pDocument;
}
#endif //_DEBUG

// CIntelliPortView message handlers
