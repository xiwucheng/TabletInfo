// TabletInfoDlg.h : header file
//

#pragma once


// CTabletInfoDlg dialog
class CTabletInfoDlg : public CDialog
{
// Construction
public:
	CTabletInfoDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_TABLETINFO_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;
	CBitmap m_Bitmap[7];
	CStatic* m_pSignal;
	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedQuery();
	int GetAddress(void);
	int GetIMEI(void);
	int GetProductKeyInfo(void);
public:
	char m_szKeyInfo[32];
	char m_szKeyID[32];
	char m_szIMEI[32];
	char m_szWIFI[32];
	char m_szBT[32];
	HANDLE m_hQueryThread;
	static UINT QueryThread(LPVOID lParam);
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};
