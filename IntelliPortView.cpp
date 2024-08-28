/* Copyright (C) 2014-2024 Stefan-Mihai MOGA
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
CIntelliPortView::CIntelliPortView()
{
}

CIntelliPortView::~CIntelliPortView()
{
}

BOOL CIntelliPortView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	BOOL bPreCreated = CEditView::PreCreateWindow(cs);
	cs.style &= ~(ES_AUTOHSCROLL|WS_HSCROLL);	// Enable word-wrapping

	return bPreCreated;
}

// CIntelliPortView printing

void CIntelliPortView::OnFilePrintPreview()
{
#ifndef SHARED_HANDLERS
	AFXPrintPreview(this);
#endif
}

BOOL CIntelliPortView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default CEditView preparation
	return CEditView::OnPreparePrinting(pInfo);
}

void CIntelliPortView::OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo)
{
	// Default CEditView begin printing
	CEditView::OnBeginPrinting(pDC, pInfo);
}

void CIntelliPortView::OnEndPrinting(CDC* pDC, CPrintInfo* pInfo)
{
	// Default CEditView end printing
	CEditView::OnEndPrinting(pDC, pInfo);
}

void CIntelliPortView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CIntelliPortView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}

// CIntelliPortView diagnostics

#ifdef _DEBUG
void CIntelliPortView::AssertValid() const
{
	CEditView::AssertValid();
}

void CIntelliPortView::Dump(CDumpContext& dc) const
{
	CEditView::Dump(dc);
}

CIntelliPortDoc* CIntelliPortView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CIntelliPortDoc)));
	return (CIntelliPortDoc*)m_pDocument;
}
#endif //_DEBUG

// CIntelliPortView message handlers
