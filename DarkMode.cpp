#include "stdafx.h"
#include "wordpad.h"
#include "DarkMode.h"
#include "AppLookDlg.h"
#include <commdlg.h>
#include <shellapi.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "uxtheme.lib")
#pragma comment(lib, "comctl32.lib")

#define DWMWA_USE_IMMERSIVE_DARK_MODE_NEW 20
#define DWMWA_USE_IMMERSIVE_DARK_MODE_OLD 19

IMPLEMENT_DYNCREATE(CDarkVisualManager, CMFCVisualManagerOffice2003)

CDarkVisualManager::CDarkVisualManager()
{
}

CDarkVisualManager::~CDarkVisualManager()
{
}

void CDarkVisualManager::OnUpdateSystemColors()
{
	CMFCVisualManagerOffice2003::OnUpdateSystemColors();

	if (DarkMode::IsSystemDark())
	{
		m_clrBarGradientDark = DarkMode::BarFace();
		m_clrBarGradientLight = DarkMode::BarFace();
		m_clrToolBarGradientDark = DarkMode::BarFace();
		m_clrToolBarGradientLight = DarkMode::BarFace();
		m_clrToolBarGradientVertDark = DarkMode::BarFace();
		m_clrToolBarGradientVertLight = DarkMode::BarFace();
		m_clrCustomizeButtonGradientDark = DarkMode::BarFace();
		m_clrCustomizeButtonGradientLight = DarkMode::BarFace();
		m_clrToolBarBottomLine = DarkMode::EdgeDark();
		m_colorToolBarCornerTop = DarkMode::BarFace();
		m_colorToolBarCornerBottom = DarkMode::BarFace();
		m_clrToolbarDisabled = DarkMode::TextDisabled();
		m_clrHighlightMenuItem = DarkMode::Highlight();
		m_clrHighlightGradientLight = DarkMode::Highlight();
		m_clrHighlightGradientDark = DarkMode::Highlight();
		m_clrHighlightDnGradientLight = DarkMode::HighlightPressed();
		m_clrHighlightDnGradientDark = DarkMode::HighlightPressed();
		m_clrHighlightCheckedGradientLight = DarkMode::Highlight();
		m_clrHighlightCheckedGradientDark = DarkMode::Highlight();
		m_clrGripper = DarkMode::EdgeDark();
		m_clrCaptionBarGradientLight = DarkMode::BarFace();
		m_clrCaptionBarGradientDark = DarkMode::BarFace();
		m_clrTaskPaneGradientDark = DarkMode::BarFace();
		m_clrTaskPaneGradientLight = DarkMode::BarFace();
		m_clrTaskPaneGroupCaptionDark = DarkMode::BarFace();
		m_clrTaskPaneGroupCaptionLight = DarkMode::BarFace();
		m_clrTaskPaneGroupCaptionSpecDark = DarkMode::BarFace();
		m_clrTaskPaneGroupCaptionSpecLight = DarkMode::BarFace();
		m_clrTaskPaneGroupAreaLight = DarkMode::CanvasBg();
		m_clrTaskPaneGroupAreaDark = DarkMode::CanvasBg();
		m_clrTaskPaneGroupAreaSpecLight = DarkMode::CanvasBg();
		m_clrTaskPaneGroupAreaSpecDark = DarkMode::CanvasBg();
		m_clrTaskPaneGroupBorder = DarkMode::EdgeDark();
	}

	m_penBottomLine.DeleteObject();
	m_penBottomLine.CreatePen(PS_SOLID, 0, DarkMode::EdgeDark());
	m_penSeparatorLight.DeleteObject();
	m_penSeparatorLight.CreatePen(PS_SOLID, 0, DarkMode::EdgeLight());
	m_penTaskPaneGroupBorder.DeleteObject();
	m_penTaskPaneGroupBorder.CreatePen(PS_SOLID, 1, DarkMode::EdgeDark());
	m_brTearOffCaption.DeleteObject();
	m_brTearOffCaption.CreateSolidBrush(DarkMode::BarFace());
	m_brFace.DeleteObject();
	m_brFace.CreateSolidBrush(DarkMode::BarFace());

	m_clrMenuLight = DarkMode::BarFace();
	m_brMenuLight.DeleteObject();
	m_brMenuLight.CreateSolidBrush(m_clrMenuLight);
	m_clrMenuBorder = DarkMode::EdgeDark();
	m_clrMenuItemBorder = DarkMode::EdgeLight();
	m_clrMenuShadowBase = DarkMode::BarShadow();
	m_clrMenuRarelyUsed = DarkMode::BarFace();
	m_clrSeparator = DarkMode::EdgeLight();
	m_penSeparator.DeleteObject();
	m_penSeparator.CreatePen(PS_SOLID, 1, m_clrSeparator);
	m_penMenuItemBorder.DeleteObject();
	m_penMenuItemBorder.CreatePen(PS_SOLID, 1, m_clrMenuItemBorder);

	m_clrPaneBorder = DarkMode::EdgeDark();

	DarkMode::ApplyGlobalColors();
}

void CDarkVisualManager::OnFillBarBackground(CDC* pDC, CBasePane* pBar, CRect rectClient, CRect rectClip, BOOL bNCArea)
{
	if (DarkMode::IsSystemDark() && pBar != NULL && pBar->IsKindOf(RUNTIME_CLASS(CMFCStatusBar)))
	{
		pDC->FillSolidRect(rectClient, DarkMode::BarFace());
		return;
	}
	CMFCVisualManagerOffice2003::OnFillBarBackground(pDC, pBar, rectClient, rectClip, bNCArea);
}

void CDarkVisualManager::OnDrawStatusBarPaneBorder(CDC* pDC, CMFCStatusBar* pBar, CRect rectPane, UINT uiID, UINT nStyle)
{
	if (DarkMode::IsSystemDark())
	{
		if (!(nStyle & SBPS_NOBORDERS))
			pDC->Draw3dRect(rectPane, DarkMode::EdgeLight(), DarkMode::EdgeDark());
		return;
	}
	CMFCVisualManagerOffice2003::OnDrawStatusBarPaneBorder(pDC, pBar, rectPane, uiID, nStyle);
}

void CDarkVisualManager::OnDrawMenuBorder(CDC* pDC, CMFCPopupMenu* pMenu, CRect rect)
{
	if (DarkMode::IsSystemDark())
	{
		pDC->FillSolidRect(rect, DarkMode::BarFace());
		pDC->Draw3dRect(rect, DarkMode::EdgeDark(), DarkMode::EdgeDark());
		CRect rInner = rect;
		rInner.DeflateRect(1, 1);
		pDC->Draw3dRect(rInner, DarkMode::EdgeLight(), DarkMode::EdgeLight());
		return;
	}
	CMFCVisualManagerOffice2003::OnDrawMenuBorder(pDC, pMenu, rect);
}

COLORREF CDarkVisualManager::GetMenuItemTextColor(CMFCToolBarMenuButton* pButton, BOOL bHighlighted, BOOL bDisabled)
{
	if (!DarkMode::IsSystemDark())
		return CMFCVisualManagerOffice2003::GetMenuItemTextColor(pButton, bHighlighted, bDisabled);
	if (bDisabled)
		return DarkMode::TextDisabled();
	return DarkMode::BarText();
}

COLORREF CDarkVisualManager::GetStatusBarPaneTextColor(CMFCStatusBar* pStatusBar, CMFCStatusBarPaneInfo* pPane)
{
	if (!DarkMode::IsSystemDark())
		return CMFCVisualManagerOffice2003::GetStatusBarPaneTextColor(pStatusBar, pPane);
	if (pPane != NULL && (pPane->nStyle & SBPS_DISABLED))
		return DarkMode::TextDisabled();
	return DarkMode::BarText();
}

void CDarkVisualManager::OnFillTasksPaneBackground(CDC* pDC, CRect rectWorkArea)
{
	if (DarkMode::IsSystemDark())
	{
		pDC->FillSolidRect(rectWorkArea, DarkMode::CanvasBg());
		return;
	}
	CMFCVisualManagerOffice2003::OnFillTasksPaneBackground(pDC, rectWorkArea);
}

namespace DarkMode
{
	static BOOL ReadAppsUseLightTheme()
	{
		BOOL bLight = TRUE;
		HKEY hKey = NULL;
		if (::RegOpenKeyEx(HKEY_CURRENT_USER, _T("Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize"), 0, KEY_READ, &hKey) == ERROR_SUCCESS)
		{
			DWORD dwValue = 1;
			DWORD dwSize = sizeof(dwValue);
			DWORD dwType = 0;
			if (::RegQueryValueEx(hKey, _T("AppsUseLightTheme"), NULL, &dwType, (LPBYTE)&dwValue, &dwSize) == ERROR_SUCCESS && dwType == REG_DWORD)
			{
				bLight = dwValue != 0;
			}
			::RegCloseKey(hKey);
		}
		return bLight;
	}

	BOOL IsSystemDark()
	{
		return !ReadAppsUseLightTheme();
	}

	typedef enum _PreferredAppMode { Default, AllowDark, ForceDark, ForceLight, Max } PreferredAppMode;
	typedef PreferredAppMode(WINAPI* fnSetPreferredAppMode)(PreferredAppMode);

	static void* GetProcByOrdinal(LPCWSTR lpszDll, WORD wOrdinal)
	{
		HMODULE hMod = ::GetModuleHandleW(lpszDll);
		if (hMod == NULL)
			hMod = ::LoadLibraryW(lpszDll);
		if (hMod == NULL)
			return NULL;
		return (void*)::GetProcAddress(hMod, (LPCSTR)(ULONG_PTR)wOrdinal);
	}

	void InitProcessDarkMode()
	{
		static BOOL bDone = FALSE;
		if (bDone)
			return;
		bDone = TRUE;

		auto pSetPreferredAppMode = (fnSetPreferredAppMode)GetProcByOrdinal(L"uxtheme.dll", 135);
		if (pSetPreferredAppMode != NULL)
			pSetPreferredAppMode(IsSystemDark() ? ForceDark : ForceLight);

		ApplyManager();
		InstallDialogThemeHook();
	}

	COLORREF CanvasBg() { return RGB(0x20, 0x20, 0x20); }
	COLORREF CanvasText() { return RGB(0xEE, 0xEE, 0xEE); }
	COLORREF BarFace() { return RGB(0x2B, 0x2B, 0x2B); }
	COLORREF BarHilite() { return RGB(0x45, 0x45, 0x45); }
	COLORREF BarShadow() { return RGB(0x1A, 0x1A, 0x1A); }
	COLORREF BarText() { return RGB(0xFF, 0xFF, 0xFF); }
	COLORREF EdgeLight() { return RGB(0x3F, 0x3F, 0x3F); }
	COLORREF EdgeDark() { return RGB(0x14, 0x14, 0x14); }
	COLORREF Highlight() { return RGB(0x3F, 0x3F, 0x3F); }
	COLORREF HighlightPressed() { return RGB(0x33, 0x33, 0x33); }
	COLORREF TextDisabled() { return RGB(0x7A, 0x7A, 0x7A); }
	COLORREF Accent() { return RGB(0x00, 0x78, 0xD4); }

	static BOOL g_bSavedGlobals = FALSE;
	static COLORREF g_clrBarFace;
	static COLORREF g_clrBarShadow;
	static COLORREF g_clrBarHilite;
	static COLORREF g_clrBarDkShadow;
	static COLORREF g_clrBarLight;
	static COLORREF g_clrBarText;
	static COLORREF g_clrBtnFace;
	static COLORREF g_clrBtnShadow;
	static COLORREF g_clrBtnHilite;
	static COLORREF g_clrWindow;
	static COLORREF g_clrWindowText;

	static void SaveGlobals()
	{
		if (g_bSavedGlobals)
			return;
		AFX_GLOBAL_DATA* pGlobalData = GetGlobalData();
		g_clrBarFace = pGlobalData->clrBarFace;
		g_clrBarShadow = pGlobalData->clrBarShadow;
		g_clrBarHilite = pGlobalData->clrBarHilite;
		g_clrBarDkShadow = pGlobalData->clrBarDkShadow;
		g_clrBarLight = pGlobalData->clrBarLight;
		g_clrBarText = pGlobalData->clrBarText;
		g_clrBtnFace = pGlobalData->clrBtnFace;
		g_clrBtnShadow = pGlobalData->clrBtnShadow;
		g_clrBtnHilite = pGlobalData->clrBtnHilite;
		g_clrWindow = pGlobalData->clrWindow;
		g_clrWindowText = pGlobalData->clrWindowText;
		g_bSavedGlobals = TRUE;
	}

	static void SetDarkGlobals()
	{
		AFX_GLOBAL_DATA* pGlobalData = GetGlobalData();
		pGlobalData->clrBarFace = BarFace();
		pGlobalData->clrBarShadow = BarShadow();
		pGlobalData->clrBarHilite = BarHilite();
		pGlobalData->clrBarDkShadow = EdgeDark();
		pGlobalData->clrBarLight = EdgeLight();
		pGlobalData->clrBarText = BarText();
		pGlobalData->clrBtnFace = BarFace();
		pGlobalData->clrBtnShadow = BarShadow();
		pGlobalData->clrBtnHilite = BarHilite();
		pGlobalData->clrWindow = CanvasBg();
		pGlobalData->clrWindowText = CanvasText();
	}

	void ApplyGlobalColors()
	{
		if (IsSystemDark())
			SetDarkGlobals();
	}

	static void RestoreGlobals()
	{
		if (!g_bSavedGlobals)
			return;
		AFX_GLOBAL_DATA* pGlobalData = GetGlobalData();
		pGlobalData->clrBarFace = g_clrBarFace;
		pGlobalData->clrBarShadow = g_clrBarShadow;
		pGlobalData->clrBarHilite = g_clrBarHilite;
		pGlobalData->clrBarDkShadow = g_clrBarDkShadow;
		pGlobalData->clrBarLight = g_clrBarLight;
		pGlobalData->clrBarText = g_clrBarText;
		pGlobalData->clrBtnFace = g_clrBtnFace;
		pGlobalData->clrBtnShadow = g_clrBtnShadow;
		pGlobalData->clrBtnHilite = g_clrBtnHilite;
		pGlobalData->clrWindow = g_clrWindow;
		pGlobalData->clrWindowText = g_clrWindowText;
		g_bSavedGlobals = FALSE;
	}

	void ApplyManager()
	{
		if (IsSystemDark())
		{
			SaveGlobals();
			SetDarkGlobals();
			theApp.m_bHiColorIcons = FALSE;
			CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CDarkVisualManager));
		}
		else
		{
			CAppLookDlg::ApplyStoredLook();
			RestoreGlobals();
		}
	}

	void ApplyTitleBar(CWnd* pWnd)
	{
		if (pWnd == NULL || !::IsWindow(pWnd->GetSafeHwnd()))
			return;
		BOOL bDark = IsSystemDark();
		HWND hWnd = pWnd->GetSafeHwnd();
		if (FAILED(::DwmSetWindowAttribute(hWnd, DWMWA_USE_IMMERSIVE_DARK_MODE_NEW, &bDark, sizeof(bDark))))
			::DwmSetWindowAttribute(hWnd, DWMWA_USE_IMMERSIVE_DARK_MODE_OLD, &bDark, sizeof(bDark));
		pWnd->SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
	}

	void ApplyControlTheme(HWND hWnd)
	{
		if (!::IsWindow(hWnd))
			return;
		if (IsSystemDark())
			::SetWindowTheme(hWnd, L"DarkMode_Explorer", NULL);
		else
			::SetWindowTheme(hWnd, NULL, NULL);
	}

	static BOOL CALLBACK ApplyChildThemeProc(HWND hWnd, LPARAM lParam)
	{
		BOOL bDark = (BOOL)lParam;
		WCHAR szClass[64] = { 0 };
		::GetClassNameW(hWnd, szClass, 64);
		if (_wcsicmp(szClass, L"Static") == 0 || _wcsicmp(szClass, L"Button") == 0 || _wcsicmp(szClass, L"Edit") == 0 || _wcsicmp(szClass, L"ComboBox") == 0 || _wcsicmp(szClass, L"ComboLBox") == 0 || _wcsicmp(szClass, L"SysListView32") == 0 || _wcsicmp(szClass, L"SysTreeView32") == 0 || _wcsicmp(szClass, L"ListBox") == 0 || _wcsicmp(szClass, L"ScrollBar") == 0)
			::SetWindowTheme(hWnd, bDark ? L"" : NULL, bDark ? L"" : NULL);
		return TRUE;
	}

	static HBRUSH GetDlgDarkBrush();
	static void DrawDarkButton(const DRAWITEMSTRUCT* pdis);
	static void RefreshDarkChildren(HWND hWnd);

	static LRESULT CALLBACK DarkDialogSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
	{
		switch (uMsg)
		{
		case WM_SHOWWINDOW:
			if (wParam && IsSystemDark())
			{
				LRESULT lr = ::DefSubclassProc(hWnd, uMsg, wParam, lParam);
				RefreshDarkChildren(hWnd);
				return lr;
			}
			break;
		case WM_ACTIVATE:
			if (IsSystemDark())
				RefreshDarkChildren(hWnd);
			break;
		case WM_ERASEBKGND:
			if (IsSystemDark())
			{
				HDC hdc = (HDC)wParam;
				RECT rc;
				::GetClientRect(hWnd, &rc);
				::FillRect(hdc, &rc, GetDlgDarkBrush());
				return TRUE;
			}
			break;
		case WM_PAINT:
			if (dwRefData != 0 && IsSystemDark())
			{
				PAINTSTRUCT ps;
				HDC hdc = ::BeginPaint(hWnd, &ps);
				RECT rc;
				::GetClientRect(hWnd, &rc);
				::FillRect(hdc, &rc, GetDlgDarkBrush());
				::EndPaint(hWnd, &ps);
				return 0;
			}
			break;
		case WM_DRAWITEM:
			{
				DRAWITEMSTRUCT* pdis = (DRAWITEMSTRUCT*)lParam;
				if (pdis != NULL && pdis->CtlType == ODT_BUTTON)
				{
					DrawDarkButton(pdis);
					return TRUE;
				}
			}
			break;
		case WM_CTLCOLORDLG:
		case WM_CTLCOLORSTATIC:
		case WM_CTLCOLOREDIT:
		case WM_CTLCOLORLISTBOX:
		case WM_CTLCOLORBTN:
			if (IsSystemDark())
			{
				HDC hdc = (HDC)wParam;
				if (uMsg == WM_CTLCOLORBTN)
				{
					::SetBkColor(hdc, BarFace());
					::SetTextColor(hdc, BarText());
				}
				else
				{
					::SetBkColor(hdc, CanvasBg());
					::SetTextColor(hdc, CanvasText());
				}
				return (LRESULT)GetDlgDarkBrush();
			}
			break;
		}
		return ::DefSubclassProc(hWnd, uMsg, wParam, lParam);
	}

	static LRESULT CALLBACK DarkComboSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
	{
		switch (uMsg)
		{
		case WM_CTLCOLORLISTBOX:
		case WM_CTLCOLOREDIT:
		case WM_CTLCOLORSTATIC:
			if (IsSystemDark())
			{
				HDC hdc = (HDC)wParam;
				::SetBkColor(hdc, CanvasBg());
				::SetTextColor(hdc, CanvasText());
				return (LRESULT)GetDlgDarkBrush();
			}
			break;
		}
		return ::DefSubclassProc(hWnd, uMsg, wParam, lParam);
	}

	static void DrawDarkButton(const DRAWITEMSTRUCT* pdis)
	{
		HDC hdc = pdis->hDC;
		RECT rc = pdis->rcItem;
		BOOL bDark = IsSystemDark();
		BOOL bDisabled = (pdis->itemState & ODS_DISABLED) != 0;
		BOOL bPressed = (pdis->itemState & ODS_SELECTED) != 0;
		BOOL bFocused = (pdis->itemState & ODS_FOCUS) != 0;

		COLORREF crFace;
		COLORREF crEdge;
		COLORREF crText;
		if (bDark)
		{
			crFace = bPressed ? HighlightPressed() : BarHilite();
			crEdge = bPressed ? EdgeDark() : EdgeLight();
			crText = bDisabled ? TextDisabled() : BarText();
		}
		else
		{
			crFace = bPressed ? ::GetSysColor(COLOR_3DSHADOW) : ::GetSysColor(COLOR_BTNFACE);
			crEdge = ::GetSysColor(COLOR_BTNSHADOW);
			crText = bDisabled ? ::GetSysColor(COLOR_GRAYTEXT) : ::GetSysColor(COLOR_BTNTEXT);
		}

		HBRUSH hbrFace = ::CreateSolidBrush(crFace);
		::FillRect(hdc, &rc, hbrFace);
		::DeleteObject(hbrFace);

		HBRUSH hbrEdge = ::CreateSolidBrush(crEdge);
		::FrameRect(hdc, &rc, hbrEdge);
		::DeleteObject(hbrEdge);

		WCHAR szText[256] = { 0 };
		::GetWindowTextW(pdis->hwndItem, szText, 256);

		::SetBkMode(hdc, TRANSPARENT);
		::SetTextColor(hdc, crText);

		HFONT hFont = (HFONT)::SendMessage(pdis->hwndItem, WM_GETFONT, 0, 0);
		HFONT hOld = hFont ? (HFONT)::SelectObject(hdc, hFont) : NULL;

		UINT uFlags = DT_CENTER | DT_VCENTER | DT_SINGLELINE;
		RECT rt = rc;
		::DrawTextW(hdc, szText, -1, &rt, uFlags);

		if (bFocused)
		{
			RECT rf = rc;
			::InflateRect(&rf, -3, -3);
			::DrawFocusRect(hdc, &rf);
		}

		if (hOld != NULL)
			::SelectObject(hdc, hOld);
	}

	static void PrepDarkChild(HWND hChild)
	{
		if (hChild == NULL || !::IsWindow(hChild))
			return;
		if (!IsSystemDark())
			return;

		WCHAR szClass[64] = { 0 };
		::GetClassNameW(hChild, szClass, 64);

		if (_wcsicmp(szClass, L"Button") == 0)
		{
			LONG_PTR dwStyle = ::GetWindowLongPtr(hChild, GWL_STYLE);
			LONG_PTR dwType = dwStyle & BS_TYPEMASK;
			if (dwType == BS_PUSHBUTTON || dwType == BS_DEFPUSHBUTTON)
			{
				if ((dwStyle & BS_TYPEMASK) != BS_OWNERDRAW)
				{
					::SetWindowLongPtr(hChild, GWL_STYLE, (dwStyle & ~BS_TYPEMASK) | BS_OWNERDRAW);
					::SetWindowPos(hChild, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
					::InvalidateRect(hChild, NULL, TRUE);
				}
			}
			else
			{
				::SetWindowTheme(hChild, L"", L"");
			}
		}
		else if (_wcsicmp(szClass, L"ComboBox") == 0)
		{
			::SetWindowTheme(hChild, L"DarkMode_Explorer", NULL);
			::SetWindowSubclass(hChild, DarkComboSubclassProc, 2, 0);
		}
		else if (_wcsicmp(szClass, L"ScrollBar") == 0)
		{
			::SetWindowTheme(hChild, L"", NULL);
		}
	}

	void SubclassDarkDialog(HWND hWnd)
	{
		if (hWnd == NULL || !::IsWindow(hWnd))
			return;
		WCHAR szClass[64] = { 0 };
		::GetClassNameW(hWnd, szClass, 64);
		if (_wcsicmp(szClass, L"#32770") != 0)
			return;
		::SetWindowSubclass(hWnd, DarkDialogSubclassProc, 1, (CWnd::FromHandlePermanent(hWnd) == NULL) ? 1 : 0);
		for (HWND hChild = ::GetWindow(hWnd, GW_CHILD); hChild != NULL; hChild = ::GetWindow(hChild, GW_HWNDNEXT))
		{
			PrepDarkChild(hChild);
			SubclassDarkDialog(hChild);
		}
	}

	static void RefreshDarkChildren(HWND hWnd)
	{
		if (hWnd == NULL || !::IsWindow(hWnd))
			return;
		for (HWND hChild = ::GetWindow(hWnd, GW_CHILD); hChild != NULL; hChild = ::GetWindow(hChild, GW_HWNDNEXT))
		{
			PrepDarkChild(hChild);
			SubclassDarkDialog(hChild);
		}
		::InvalidateRect(hWnd, NULL, TRUE);
	}

	void ApplyDialogTheme(CWnd* pDlg)
	{
		if (pDlg == NULL || !::IsWindow(pDlg->GetSafeHwnd()))
			return;
		BOOL bDark = IsSystemDark();
		ApplyTitleBar(pDlg);
		::EnumChildWindows(pDlg->GetSafeHwnd(), ApplyChildThemeProc, (LPARAM)bDark);
		SubclassDarkDialog(pDlg->GetSafeHwnd());
	}

	static LPPAGESETUPHOOK g_pfnPrevPageSetupHook = NULL;
	static UINT_PTR CALLBACK DarkPageSetupHook(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		UINT_PTR nRet = 0;
		if (g_pfnPrevPageSetupHook != NULL)
			nRet = g_pfnPrevPageSetupHook(hDlg, uMsg, wParam, lParam);
		if (uMsg == WM_INITDIALOG && IsSystemDark())
		{
			CWnd* pWnd = CWnd::FromHandle(hDlg);
			if (pWnd != NULL)
				ApplyDialogTheme(pWnd);
		}
		return nRet;
	}

	void ThemePageSetupHook(PAGESETUPDLG& psd)
	{
		g_pfnPrevPageSetupHook = (LPPAGESETUPHOOK)psd.lpfnPageSetupHook;
		psd.lpfnPageSetupHook = (LPPAGESETUPHOOK)DarkPageSetupHook;
		psd.Flags |= PSD_ENABLEPAGESETUPHOOK;
	}

	static HHOOK g_hCbtApp = NULL;
	static LRESULT CALLBACK GlobalCbtProc(int nCode, WPARAM wParam, LPARAM lParam)
	{
		if (nCode == HCBT_ACTIVATE && IsSystemDark())
		{
			HWND hWnd = (HWND)wParam;
			WCHAR szClass[32] = { 0 };
			::GetClassNameW(hWnd, szClass, 32);
			if (_wcsicmp(szClass, L"#32770") == 0)
			{
				CWnd* pWnd = CWnd::FromHandle(hWnd);
				if (pWnd != NULL)
					ApplyDialogTheme(pWnd);
				else
					SubclassDarkDialog(hWnd);
			}
		}
		return ::CallNextHookEx(g_hCbtApp, nCode, wParam, lParam);
	}

	void InstallDialogThemeHook()
	{
		if (g_hCbtApp == NULL)
			g_hCbtApp = ::SetWindowsHookEx(WH_CBT, GlobalCbtProc, NULL, ::GetCurrentThreadId());
	}

	void ShowDarkAbout(HWND hOwner, LPCTSTR lpszApp, LPCTSTR lpszOther, HICON hIcon)
	{
		InstallDialogThemeHook();
		ShellAbout(hOwner, lpszApp, lpszOther, hIcon);
	}

	static HBRUSH g_hbrDlgDarkBg = NULL;
	static HBRUSH GetDlgDarkBrush()
	{
		if (g_hbrDlgDarkBg == NULL)
			g_hbrDlgDarkBg = ::CreateSolidBrush(CanvasBg());
		return g_hbrDlgDarkBg;
	}

	HBRUSH DlgCtlColor(CDC* pDC, UINT nCtlColor)
	{
		if (!IsSystemDark())
			return NULL;
		if (nCtlColor == CTLCOLOR_BTN)
		{
			pDC->SetBkColor(BarFace());
			pDC->SetTextColor(BarText());
		}
		else
		{
			pDC->SetBkColor(CanvasBg());
			pDC->SetTextColor(CanvasText());
		}
		return GetDlgDarkBrush();
	}

	BOOL DlgEraseBkgnd(CWnd* pDlg, CDC* pDC)
	{
		if (!IsSystemDark())
			return FALSE;
		CRect rect;
		pDlg->GetClientRect(rect);
		pDC->FillSolidRect(rect, CanvasBg());
		return TRUE;
	}
}

/////////////////////////////////////////////////////////////////////////////
// CDarkFontDialog

IMPLEMENT_DYNAMIC(CDarkFontDialog, CFontDialog)

BEGIN_MESSAGE_MAP(CDarkFontDialog, CFontDialog)
	ON_WM_CTLCOLOR()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

CDarkFontDialog::CDarkFontDialog(LPLOGFONT lplfInitial, DWORD dwFlags, CDC* pdcPrinter, CWnd* pParentWnd)
	: CFontDialog(lplfInitial, dwFlags, pdcPrinter, pParentWnd)
{
}

CDarkFontDialog::CDarkFontDialog(const CHARFORMAT& charformat, DWORD dwFlags, CDC* pdcPrinter, CWnd* pParentWnd)
	: CFontDialog(charformat, dwFlags, pdcPrinter, pParentWnd)
{
}

BOOL CDarkFontDialog::OnInitDialog()
{
	BOOL bResult = CFontDialog::OnInitDialog();
	DarkMode::ApplyDialogTheme(this);
	return bResult;
}

HBRUSH CDarkFontDialog::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CFontDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	if (DarkMode::IsSystemDark() && (nCtlColor == CTLCOLOR_DLG || nCtlColor == CTLCOLOR_STATIC || nCtlColor == CTLCOLOR_EDIT || nCtlColor == CTLCOLOR_LISTBOX || nCtlColor == CTLCOLOR_BTN))
		return DarkMode::DlgCtlColor(pDC, nCtlColor);
	return hbr;
}

BOOL CDarkFontDialog::OnEraseBkgnd(CDC* pDC)
{
	if (DarkMode::DlgEraseBkgnd(this, pDC))
		return TRUE;
	return CFontDialog::OnEraseBkgnd(pDC);
}
