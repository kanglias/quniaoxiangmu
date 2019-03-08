
// DriveBirdsApplicationDlg.h : header file
//

#pragma once
#include "mscomm1.h"
#include "drivebirdscenter.h"


// CDriveBirdsApplicationDlg dialog
class CDriveBirdsApplicationDlg : public CDialogEx
{
// Construction
public:
	CDriveBirdsApplicationDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_DRIVEBIRDSAPPLICATION_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CMscomm1 m_mscomm;
	CDriveBirdsCenter m_drivebirds;
	DECLARE_EVENTSINK_MAP()
	void OnCommMscomm1();
};
