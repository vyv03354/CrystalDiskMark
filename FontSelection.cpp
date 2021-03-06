﻿/*---------------------------------------------------------------------------*/
//       Author : hiyohiyo
//         Mail : hiyohiyo@crystalmark.info
//          Web : https://crystalmark.info/
//      License : The MIT License
/*---------------------------------------------------------------------------*/

#include "stdafx.h"
#include "DiskMark.h"
#include "DiskMarkDlg.h"
#include "FontSelection.h"

struct EnumFontFamExProcData {
	HDC hDC;
	CFontComboBox* pFontComboBox;
};

int CALLBACK EnumFontFamExProc(ENUMLOGFONTEX *lpelfe, NEWTEXTMETRICEX *lpntme, int FontType, LPARAM lParam);

IMPLEMENT_DYNAMIC(CFontSelection, CDialog)

static CDiskMarkDlg *p;

CFontSelection::CFontSelection(CWnd* pParent)
	: CDialogCx(CFontSelection::IDD, pParent)
{
	p = (CDiskMarkDlg*)pParent;
	_tcscpy_s(m_Ini, MAX_PATH, ((CDiskMarkApp*)AfxGetApp())->m_Ini);

	m_CurrentLangPath = ((CMainDialog*)pParent)->m_CurrentLangPath;
	m_DefaultLangPath = ((CMainDialog*)pParent)->m_DefaultLangPath;
	m_ZoomType = ((CMainDialog*)pParent)->GetZoomType();
	m_FontFace = ((CMainDialog*)pParent)->m_FontFace;
	m_FontType = ((CMainDialog*)pParent)->m_FontType;

	m_CxThemeDir = ((CDiskMarkApp*)AfxGetApp())->m_ThemeDir;
	m_CxCurrentTheme = ((CMainDialog*)pParent)->m_CurrentTheme;
	m_CxDefaultTheme = ((CMainDialog*)pParent)->m_DefaultTheme;

	m_FontScale = ((CDiskMarkDlg*)pParent)->m_FontScale;

	m_BackgroundName = L"";
}

CFontSelection::~CFontSelection()
{
}

void CFontSelection::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, ID_OK, m_CtrlOk);
	DDX_Control(pDX, IDC_FONT_FACE, m_LabelFontFace);
	DDX_Control(pDX, IDC_FONT_SCALE, m_LabelFontScale);
	DDX_Control(pDX, IDC_FONT_TYPE, m_LabelFontType);
	DDX_Control(pDX, IDC_FONT_COMBO, m_FontComboBox);
	DDX_Control(pDX, IDC_FONT_SCALE_COMBO, m_FontScaleComboBox);
	DDX_Control(pDX, IDC_FONT_TYPE_COMBO, m_FontTypeComboBox);
	DDX_Control(pDX, IDC_SET_DEFAULT, m_ButtonSetDefault);
}

BEGIN_MESSAGE_MAP(CFontSelection, CDialogCx)
	ON_BN_CLICKED(ID_OK, &CFontSelection::OnBnClickedOk)
	ON_BN_CLICKED(IDC_SET_DEFAULT, &CFontSelection::OnSetDefault)
END_MESSAGE_MAP()

BOOL CFontSelection::OnInitDialog()
{
	CDialogCx::OnInitDialog();

	SetWindowText(i18n(_T("WindowTitle"), _T("FONT_SETTING")));

    CClientDC dc(this);
    LOGFONT logfont; 
    ZeroMemory(&logfont, sizeof(LOGFONT)); 
    logfont.lfCharSet = DEFAULT_CHARSET;
    EnumFontFamExProcData data = { dc.m_hDC, &m_FontComboBox };

    ::EnumFontFamiliesExW(dc.m_hDC, &logfont, (FONTENUMPROC)EnumFontFamExProc, (LPARAM)&data, 0);

	int no = m_FontComboBox.FindStringExact(0, m_FontFace);
	if(no >= 0)
	{
		m_FontComboBox.SetCurSel(no);
	}
	else
	{
		no = m_FontComboBox.FindStringExact(0, _T("Segoe UI"));
		if(no >= 0)
		{
			m_FontComboBox.SetCurSel(no);
		}
		else
		{
			no = m_FontComboBox.FindStringExact(0, _T("Tahoma"));
			if(no >= 0)
			{
				m_FontComboBox.SetCurSel(no);
			}
			else
			{
				m_FontComboBox.SetCurSel(0);
			}
		}
	}

	m_FontTypeComboBox.AddString(i18n(_T("Dialog"), _T("Auto")));
	m_FontTypeComboBox.AddString(L"GDI");
	m_FontTypeComboBox.AddString(L"GDI+ Type1");
	m_FontTypeComboBox.AddString(L"GDI+ Type2");
	m_FontTypeComboBox.AddString(L"GDI+ Type3");

//	m_FontTypeComboBox.AddString(L"DirectWrite");
	if (FT_AUTO <= m_FontType && m_FontType <= FT_GDI_PLUS_3)
	{
		m_FontTypeComboBox.SetCurSel(m_FontType);
	}
	else
	{
		m_FontTypeComboBox.SetCurSel(0);
	}

	CString cstr;

	for (int i = 50; i <= 150; i += 10)
	{
		cstr.Format(L"%d", i);
		m_FontScaleComboBox.AddString(cstr);
		if (m_FontScale == i) { m_FontScaleComboBox.SetCurSel(m_FontScaleComboBox.GetCount() - 1);  }
	}

	m_LabelFontFace.SetWindowTextW(i18n(L"Dialog", L"FONT_FACE"));
	m_LabelFontScale.SetWindowTextW(i18n(L"Dialog", L"FONT_SCALE"));
	m_LabelFontType.SetWindowTextW(i18n(L"Dialog", L"FONT_RENDER_METHOD"));

	m_ButtonSetDefault.SetWindowTextW(i18n(L"Dialog", L"DEFAULT"));

	UpdateDialogSize();

	m_FontComboBox.SetBgColor(RGB(255, 255, 255), RGB(192, 192, 255));

	return TRUE;
}

void CFontSelection::UpdateDialogSize()
{
	BYTE textAlpha = 255;
	COLORREF textColor = RGB(0, 0, 0);
	COLORREF textSelectedColor = RGB(0, 0, 0);

	ChangeZoomType(m_ZoomType);
	SetClientRect((DWORD)(SIZE_X * m_ZoomRatio), (DWORD)(SIZE_Y * m_ZoomRatio), 0);

	UpdateBackground();

	m_LabelFontFace.InitControl(8, 8, 472, 24, m_ZoomRatio, NULL, 0, SS_LEFT, CStaticCx::OwnerDrawTransparent | m_IsHighContrast);
	m_LabelFontScale.InitControl(8, 96, 236, 24, m_ZoomRatio, NULL, 0, SS_LEFT, CStaticCx::OwnerDrawTransparent | m_IsHighContrast);
	m_LabelFontType.InitControl(248, 96, 236, 24, m_ZoomRatio, NULL, 0, SS_LEFT, CStaticCx::OwnerDrawTransparent | m_IsHighContrast);
	m_FontComboBox.InitControl(20, 36, 440, 360, m_ZoomRatio, CComboBoxCx::OwnerDrawTransparent | m_IsHighContrast);
	m_FontScaleComboBox.InitControl(20, 124, 200, 360, m_ZoomRatio, CComboBoxCx::OwnerDrawTransparent | m_IsHighContrast);
	m_FontTypeComboBox.InitControl(260, 124, 200, 360, m_ZoomRatio, CComboBoxCx::OwnerDrawTransparent | m_IsHighContrast);

	m_ButtonSetDefault.InitControl(40, 180, 160, 32, m_ZoomRatio, NULL, 0, BS_CENTER, CButtonCx::OwnerDrawGlass | m_IsHighContrast);
	m_CtrlOk.InitControl(280, 180, 160, 32, m_ZoomRatio, NULL, 0, BS_CENTER, CButtonCx::OwnerDrawGlass | m_IsHighContrast);
	
	m_LabelFontFace.SetFontEx(m_FontFace, 20, m_ZoomRatio);
	m_LabelFontScale.SetFontEx(m_FontFace, 20, m_ZoomRatio);
	m_LabelFontType.SetFontEx(m_FontFace, 20, m_ZoomRatio);

	m_FontComboBox.SetFontHeight(28, m_ZoomRatio);
	m_FontComboBox.SetFontEx(m_FontFace, 28, m_ZoomRatio, textAlpha, textColor, textSelectedColor, FW_NORMAL, FT_GDI);
	m_FontComboBox.SetItemHeight(-1, (UINT)(44 * m_ZoomRatio));
	for (int i = 0; i < m_FontComboBox.GetCount(); i++)
	{
		m_FontComboBox.SetItemHeight(i, (UINT)(44 * m_ZoomRatio));
	}

	m_FontScaleComboBox.SetFontEx(m_FontFace, 20, m_ZoomRatio, textAlpha, textColor, textSelectedColor, FW_NORMAL, FT_GDI);
	m_FontScaleComboBox.SetItemHeight(-1, (UINT)(28 * m_ZoomRatio));
	for (int i = 0; i < m_FontScaleComboBox.GetCount(); i++)
	{
		m_FontScaleComboBox.SetItemHeight(i, (UINT)(28 * m_ZoomRatio));
	}

	m_FontTypeComboBox.SetFontEx(m_FontFace, 20, m_ZoomRatio, textAlpha, textColor, textSelectedColor, FW_NORMAL, FT_GDI);
	m_FontTypeComboBox.SetItemHeight(-1, (UINT) (28 * m_ZoomRatio));
	for (int i = 0; i < m_FontTypeComboBox.GetCount(); i++)
	{
		m_FontTypeComboBox.SetItemHeight(i, (UINT) (28 * m_ZoomRatio));
	}

	m_ButtonSetDefault.SetFontEx(m_FontFace, 20, m_ZoomRatio);
	m_CtrlOk.SetFontEx(m_FontFace, 20, m_ZoomRatio);
	
	m_ButtonSetDefault.SetHandCursor();
	m_CtrlOk.SetHandCursor();

	Invalidate();
}

CString CFontSelection::GetFontFace()
{
	return m_FontFace;
}

INT CFontSelection::GetFontType()
{
	return m_FontType;
}

INT CFontSelection::GetFontScale()
{
	return m_FontScale;
}

struct EnumFontFamExProcBoldData {
	ENUMLOGFONTEX* lpelfeRegular;
	CFontComboBox* pFontComboBox;
};

int CALLBACK EnumFontFamExProcBold(ENUMLOGFONTEX *lpelfe, NEWTEXTMETRICEX *lpntme, int FontType, LPARAM lParam)
{
	EnumFontFamExProcBoldData* pdata = (EnumFontFamExProcBoldData*)lParam;
	CFontComboBox* pFontComboBox = pdata->pFontComboBox;
	if (!lpelfe->elfLogFont.lfItalic
		&& lpelfe->elfLogFont.lfCharSet != SYMBOL_CHARSET
		&& _tcschr((TCHAR*)lpelfe->elfLogFont.lfFaceName, _T('@')) == NULL
		&& _tcscmp(lpelfe->elfFullName, pdata->lpelfeRegular->elfFullName) != 0
		&& pFontComboBox->FindStringExact(0, (TCHAR*)lpelfe->elfFullName) == CB_ERR)
	{
		pFontComboBox->AddString((TCHAR*)lpelfe->elfFullName);
	}
	return TRUE;
}

int CALLBACK EnumFontFamExProc(ENUMLOGFONTEX *lpelfe, NEWTEXTMETRICEX *lpntme, int FontType, LPARAM lParam)
{
	EnumFontFamExProcData* pdata = (EnumFontFamExProcData*)lParam;
	CFontComboBox* pFontComboBox = pdata->pFontComboBox;
//	CComboBox* pFontComboBox = (CComboBox*)lParam;
	if(pFontComboBox->FindStringExact(0, (TCHAR*)lpelfe->elfLogFont.lfFaceName) == CB_ERR
	&& _tcschr((TCHAR*)lpelfe->elfLogFont.lfFaceName, _T('@')) == NULL
	&& lpelfe->elfLogFont.lfCharSet != SYMBOL_CHARSET
	)
	{
		pFontComboBox->AddString((TCHAR*)lpelfe->elfLogFont.lfFaceName);
		LOGFONT logfont = {};
		_tcscpy(logfont.lfFaceName, lpelfe->elfLogFont.lfFaceName);
		logfont.lfCharSet = DEFAULT_CHARSET;
		EnumFontFamExProcBoldData data = { lpelfe, pFontComboBox };
		::EnumFontFamiliesExW(pdata->hDC, &logfont, (FONTENUMPROC)EnumFontFamExProcBold, (LPARAM)&data, 0);
	}
    return TRUE;
}

void CFontSelection::OnBnClickedOk()
{
	CString cstr;

	m_FontComboBox.GetLBText(m_FontComboBox.GetCurSel(), m_FontFace);
	m_FontType = m_FontTypeComboBox.GetCurSel();
	m_FontScaleComboBox.GetLBText(m_FontScaleComboBox.GetCurSel(), cstr);
	m_FontScale = _wtoi(cstr);

	CDialog::OnOK();
}


void CFontSelection::OnSetDefault()
{
	m_FontComboBox.ResetContent();

	CClientDC dc(this);
	LOGFONT logfont;
	ZeroMemory(&logfont, sizeof(LOGFONT));
	logfont.lfCharSet = DEFAULT_CHARSET;
	EnumFontFamExProcData data = { dc.m_hDC, &m_FontComboBox };

	::EnumFontFamiliesExW(dc.m_hDC, &logfont, (FONTENUMPROC)EnumFontFamExProc, (LPARAM)&data, 0);

	int no = m_FontComboBox.FindStringExact(0, _T("Segoe UI"));
	if (no >= 0)
	{
		m_FontComboBox.SetCurSel(no);
	}
	else
	{
		no = m_FontComboBox.FindStringExact(0, _T("Tahoma"));
		if (no >= 0)
		{
			m_FontComboBox.SetCurSel(no);
		}
		else
		{
			m_FontComboBox.SetCurSel(0);
		}
	}

	m_FontTypeComboBox.SetCurSel(0);
	m_FontScaleComboBox.SetCurSel(5);
}
