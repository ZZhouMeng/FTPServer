
// FTPServerDlg.h: 头文件
//
#include"UserInformation.h"
#include"FtpEventSink.h"
#pragma once


// CFTPServerDlg 对话框
class CFTPServerDlg : public CDialogEx, FtpEventSink
{
// 构造
public:
	CFTPServerDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_FTPSERVER_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
	void GetIP();
public:
	CEdit* c_serverip;
	CEdit* c_serverport;
	CEdit* c_timeout;
	CEdit* c_maxlink;
	CString m_hostname;
	CButton * c_startbutton;
	CButton * c_stopbutton;
	WORD v;
	WSADATA wsData;
	CArray<UserInformation, UserInformation&> m_userList;
public:
	CString m_serverip;
	int m_port;
	int m_time;
	int m_maxlink;
	BOOL m_serverState;
	afx_msg void OnBnServerStart();
	afx_msg void OnBnServerStop();
	void SStartComponentsState(BOOL state);

public:
	CListCtrl m_cUserListCtrl;
	afx_msg void OnBnUserAdd();
	afx_msg void OnBnUserEdit();
	afx_msg void OnBnUserDelete();
	

public:
	void AddTraceLine(int nLevel, LPCTSTR pstrFormat, ...);
	virtual void OnFTPStatusChange(int nType, LPCTSTR lpszText);
	CObList m_LogQueue;
	CCriticalSection m_QueueLock;
	CListBox m_TraceList;
	class CLogMsg : public CObject
	{
	public:
		CLogMsg() {};
		virtual ~CLogMsg() {};
		SYSTEMTIME m_sysTime;
		int        m_nLevel;
		CString    m_strText;
	};
	LRESULT OnAddTraceLine(WPARAM, LPARAM);//自定义消息响应
	BOOL m_isanonymous;
	afx_msg void OnClickedCheck1();
	void createAnonymous();
};
