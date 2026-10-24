//
// chicdial.cpp : implementation file
//
// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) 1992-1998 Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

#include "stdafx.h"
#include "DarkMode.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCSDialog dialog

CCSDialog::CCSDialog(UINT nIDTemplate, CWnd* pParentWnd)
	: CDialog(nIDTemplate, pParentWnd)
{
}

CCSDialog::CCSDialog(LPCTSTR lpszTemplateName, CWnd* pParentWnd)
	: CDialog(lpszTemplateName, pParentWnd)
{
}

CCSDialog::CCSDialog() : CDialog()
{
}

BEGIN_MESSAGE_MAP(CCSDialog, CDialog)
	//{{AFX_MSG_MAP(CCSDialog)
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_HELP, OnHelp)
	ON_MESSAGE(WM_CONTEXTMENU, OnHelpContextMenu)
	ON_WM_CTLCOLOR()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CCSDialog message handlers

HBRUSH CCSDialog::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	if (DarkMode::IsSystemDark() &&
		(nCtlColor == CTLCOLOR_DLG || nCtlColor == CTLCOLOR_STATIC ||
		 nCtlColor == CTLCOLOR_EDIT || nCtlColor == CTLCOLOR_LISTBOX ||
		 nCtlColor == CTLCOLOR_BTN))
	{
		return DarkMode::DlgCtlColor(pDC, nCtlColor);
	}
	return hbr;
}

BOOL CCSDialog::OnEraseBkgnd(CDC* pDC)
{
	if (DarkMode::DlgEraseBkgnd(this, pDC))
		return TRUE;
	return CDialog::OnEraseBkgnd(pDC);
}

LRESULT CCSDialog::OnHelp(WPARAM, LPARAM lParam)
{
	::WinHelp( (HWND)((LPHELPINFO)lParam)->hItemHandle, AfxGetApp()->m_pszHelpFilePath,
		HELP_WM_HELP, (ULONG_PTR)(LPVOID)GetHelpIDs());
	return 0;
}

LRESULT CCSDialog::OnHelpContextMenu(WPARAM wParam, LPARAM)
{
	::WinHelp((HWND)wParam, AfxGetApp()->m_pszHelpFilePath,
		HELP_CONTEXTMENU, (ULONG_PTR)(LPVOID)GetHelpIDs());
	return 0;
}

BOOL CCSDialog::OnInitDialog()
{
	CDialog::OnInitDialog();
	ModifyStyleEx(0, WS_EX_CONTEXTHELP);
	DarkMode::ApplyDialogTheme(this);
	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

/////////////////////////////////////////////////////////////////////////////
// CCSPropertyPage

CCSPropertyPage::CCSPropertyPage(UINT nIDTemplate, UINT nIDCaption)
	: CPropertyPage(nIDTemplate, nIDCaption)
{
}

CCSPropertyPage::CCSPropertyPage(LPCTSTR lpszTemplateName,
	UINT nIDCaption) : CPropertyPage(lpszTemplateName, nIDCaption)
{
}

BEGIN_MESSAGE_MAP(CCSPropertyPage, CPropertyPage)
	//{{AFX_MSG_MAP(CCSPropertyPage)
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_HELP, OnHelp)
	ON_MESSAGE(WM_CONTEXTMENU, OnHelpContextMenu)
	ON_WM_CTLCOLOR()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CCSPropertyPage message handlers

BOOL CCSPropertyPage::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	DarkMode::ApplyDialogTheme(this);
	return TRUE;
}

HBRUSH CCSPropertyPage::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CPropertyPage::OnCtlColor(pDC, pWnd, nCtlColor);
	if (DarkMode::IsSystemDark() &&
		(nCtlColor == CTLCOLOR_DLG || nCtlColor == CTLCOLOR_STATIC ||
		 nCtlColor == CTLCOLOR_EDIT || nCtlColor == CTLCOLOR_LISTBOX ||
		 nCtlColor == CTLCOLOR_BTN))
	{
		return DarkMode::DlgCtlColor(pDC, nCtlColor);
	}
	return hbr;
}

BOOL CCSPropertyPage::OnEraseBkgnd(CDC* pDC)
{
	if (DarkMode::DlgEraseBkgnd(this, pDC))
		return TRUE;
	return CPropertyPage::OnEraseBkgnd(pDC);
}

LRESULT CCSPropertyPage::OnHelp(WPARAM, LPARAM lParam)
{
	::WinHelp( (HWND)((LPHELPINFO)lParam)->hItemHandle, AfxGetApp()->m_pszHelpFilePath,
		HELP_WM_HELP, (ULONG_PTR)(LPVOID)GetHelpIDs());
	return 0;
}

LRESULT CCSPropertyPage::OnHelpContextMenu(WPARAM wParam, LPARAM)
{
	::WinHelp((HWND)wParam, AfxGetApp()->m_pszHelpFilePath,
		HELP_CONTEXTMENU, (ULONG_PTR)(LPVOID)GetHelpIDs());
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// CCSPropertySheet

BEGIN_MESSAGE_MAP(CCSPropertySheet, CPropertySheet)
	//{{AFX_MSG_MAP(CCSPropertySheet)
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_HELP, OnHelp)
	ON_MESSAGE(WM_CONTEXTMENU, OnHelpContextMenu)
	ON_WM_CTLCOLOR()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

CCSPropertySheet::CCSPropertySheet(UINT nIDCaption, CWnd *pParentWnd,
	UINT iSelectPage) : CPropertySheet(nIDCaption, pParentWnd, iSelectPage)
{
}

CCSPropertySheet::CCSPropertySheet(LPCTSTR pszCaption, CWnd *pParentWnd,
	UINT iSelectPage) : CPropertySheet(pszCaption, pParentWnd, iSelectPage)
{
}

/////////////////////////////////////////////////////////////////////////////
// CCSPropertySheet message handlers

LRESULT CCSPropertySheet::OnHelp(WPARAM wParam, LPARAM lParam)
{
	GetActivePage()->SendMessage(WM_HELP, wParam, lParam);
	return 0;
}

LRESULT CCSPropertySheet::OnHelpContextMenu(WPARAM wParam, LPARAM lParam)
{
	GetActivePage()->SendMessage(WM_CONTEXTMENU, wParam, lParam);
	return 0;
}

BOOL CCSPropertySheet::PreCreateWindow(CREATESTRUCT& cs)
{
	cs.dwExStyle |= WS_EX_CONTEXTHELP;
	return CPropertySheet::PreCreateWindow(cs);
}

BOOL CCSPropertySheet::OnInitDialog()
{
	BOOL bResult = CPropertySheet::OnInitDialog();
	DarkMode::ApplyDialogTheme(this);
	return bResult;
}

HBRUSH CCSPropertySheet::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CPropertySheet::OnCtlColor(pDC, pWnd, nCtlColor);
	if (DarkMode::IsSystemDark() &&
		(nCtlColor == CTLCOLOR_DLG || nCtlColor == CTLCOLOR_STATIC ||
		 nCtlColor == CTLCOLOR_EDIT || nCtlColor == CTLCOLOR_LISTBOX ||
		 nCtlColor == CTLCOLOR_BTN))
	{
		return DarkMode::DlgCtlColor(pDC, nCtlColor);
	}
	return hbr;
}

BOOL CCSPropertySheet::OnEraseBkgnd(CDC* pDC)
{
	if (DarkMode::DlgEraseBkgnd(this, pDC))
		return TRUE;
	return CPropertySheet::OnEraseBkgnd(pDC);
}
