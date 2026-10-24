#pragma once

class CDarkVisualManager : public CMFCVisualManagerOffice2003
{
	DECLARE_DYNCREATE(CDarkVisualManager)

public:
	CDarkVisualManager();
	virtual ~CDarkVisualManager();

	virtual void OnUpdateSystemColors();
	virtual void OnFillBarBackground(CDC* pDC, CBasePane* pBar, CRect rectClient, CRect rectClip, BOOL bNCArea = FALSE);
	virtual void OnDrawStatusBarPaneBorder(CDC* pDC, CMFCStatusBar* pBar, CRect rectPane, UINT uiID, UINT nStyle);
	virtual void OnDrawMenuBorder(CDC* pDC, CMFCPopupMenu* pMenu, CRect rect);
	virtual COLORREF GetMenuItemTextColor(CMFCToolBarMenuButton* pButton, BOOL bHighlighted, BOOL bDisabled);
	virtual COLORREF GetStatusBarPaneTextColor(CMFCStatusBar* pStatusBar, CMFCStatusBarPaneInfo* pPane);
	virtual void OnFillTasksPaneBackground(CDC* pDC, CRect rectWorkArea);
};

namespace DarkMode
{
	BOOL IsSystemDark();
	void InitProcessDarkMode();
	void InstallDialogThemeHook();
	void ApplyManager();
	void ApplyGlobalColors();
	void ApplyTitleBar(CWnd* pWnd);
	void ApplyControlTheme(HWND hWnd);
	void ApplyDialogTheme(CWnd* pDlg);
	HBRUSH DlgCtlColor(CDC* pDC, UINT nCtlColor);
	BOOL DlgEraseBkgnd(CWnd* pDlg, CDC* pDC);
	void ThemePageSetupHook(PAGESETUPDLG& psd);
	void ShowDarkAbout(HWND hOwner, LPCTSTR lpszApp, LPCTSTR lpszOther, HICON hIcon);

	COLORREF CanvasBg();
	COLORREF CanvasText();
	COLORREF BarFace();
	COLORREF BarHilite();
	COLORREF BarShadow();
	COLORREF BarText();
	COLORREF EdgeLight();
	COLORREF EdgeDark();
	COLORREF Highlight();
	COLORREF HighlightPressed();
	COLORREF TextDisabled();
	COLORREF Accent();
}

class CDarkFontDialog : public CFontDialog
{
	DECLARE_DYNAMIC(CDarkFontDialog)
public:
	CDarkFontDialog(LPLOGFONT lplfInitial = NULL, DWORD dwFlags = CF_EFFECTS | CF_SCREENFONTS, CDC* pdcPrinter = NULL, CWnd* pParentWnd = NULL);
	CDarkFontDialog(const CHARFORMAT& charformat, DWORD dwFlags = CF_SCREENFONTS, CDC* pdcPrinter = NULL, CWnd* pParentWnd = NULL);
protected:
	virtual BOOL OnInitDialog();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	DECLARE_MESSAGE_MAP()
};

