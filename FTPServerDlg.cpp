
// FTPServerDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "FTPServer.h"
#include "FTPServerDlg.h"
#include"Server.h"
#include"UserDlg.h"


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框
extern Server cServer;
bool state;
int anon_index = 0;
class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CFTPServerDlg 对话框



CFTPServerDlg::CFTPServerDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_FTPSERVER_DIALOG, pParent)
	, m_isanonymous(FALSE)
{
	m_port = 21;
	m_serverip = _T("");
	m_time = 10;
	m_maxlink = 10;
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CFTPServerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_SERVERPORT, m_port);
	DDX_Text(pDX, IDC_EDIT_TIMEOUT, m_time);
	DDX_Text(pDX, IDC_EDIT_MAXLINK, m_maxlink);
	DDX_Text(pDX, IDC_EDIT_SERVERIP, m_serverip);
	DDX_Control(pDX, IDC_LIST_USERS, m_cUserListCtrl);
	DDX_Control(pDX, IDC_LOGLIST, m_TraceList);
	DDX_Check(pDX, IDC_CHECK1, m_isanonymous);
}

BEGIN_MESSAGE_MAP(CFTPServerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_SERVER_START, &CFTPServerDlg::OnBnServerStart)
	ON_BN_CLICKED(IDC_BUTTON_SERVER_STOP, &CFTPServerDlg::OnBnServerStop)
	ON_BN_CLICKED(IDC_BUTTON_USER_ADD, &CFTPServerDlg::OnBnUserAdd)
	ON_BN_CLICKED(IDC_BUTTON_USER_EDIT, &CFTPServerDlg::OnBnUserEdit)
	ON_BN_CLICKED(IDC_BUTTON_USER_DELETE, &CFTPServerDlg::OnBnUserDelete)
	ON_MESSAGE(WM_ADDTRACELINE, OnAddTraceLine)//绑定消息到该消息的响应函数
	ON_BN_CLICKED(IDC_CHECK1, &CFTPServerDlg::OnClickedCheck1)
END_MESSAGE_MAP()


// CFTPServerDlg 消息处理程序

BOOL CFTPServerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	c_serverip= (CEdit*)GetDlgItem(IDC_EDIT_SERVERIP);
	c_serverport = (CEdit*)GetDlgItem(IDC_EDIT_SERVERPORT);
	c_timeout = (CEdit*)GetDlgItem(IDC_EDIT_TIMEOUT);
	c_maxlink = (CEdit*)GetDlgItem(IDC_EDIT_MAXLINK);
	c_startbutton = (CButton*)GetDlgItem(IDC_BUTTON_SERVER_START);
	c_stopbutton = (CButton*)GetDlgItem(IDC_BUTTON_SERVER_STOP);
	c_serverport->SetWindowText(_T("21"));//默认端口号21
	c_timeout->SetWindowText(_T("10"));//默认超时10
	c_maxlink->SetWindowText(_T("10"));//默认最大连接数10
	GetIP();
	UpdateData(TRUE);//true刷新控件的值到对应的变量。false拷贝变量值到控件显示。

	m_cUserListCtrl.InsertColumn(0, _T("用户名"), LVCFMT_CENTER, 120);//增加列属性
	m_cUserListCtrl.InsertColumn(1, _T("目录"), LVCFMT_CENTER, 150);
	m_cUserListCtrl.InsertColumn(2, _T("权限"), LVCFMT_CENTER, 240);
	cServer.Initialize(this);
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CFTPServerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CFTPServerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CFTPServerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}
void CFTPServerDlg::GetIP()
{
	WSADATA wsData;
	::WSAStartup(MAKEWORD(2, 2), &wsData); //如果获取失败，请添加以上两行代码；如果获取成功，则可以不添加
	char szhostname[128];
	CString str;
	if (gethostname(szhostname, 128) == 0) //获得主机名
	{
		//获得主机IP地址
		struct hostent* phost;
		int occurred;
		phost = gethostbyname(szhostname);//根据主机名获得IP地址
		m_hostname = szhostname;
		occurred = 0;
		int j;
		int h_length = 4;
		for (j = 0; j < h_length; j++)
		{
			CString addr;

			if (j > 0)
				str += ".";
			addr.Format(_T("%u"), (unsigned int)((unsigned char*)phost->h_addr_list[occurred])[j]);
			str += addr;
		}
		
	}
	m_serverip = str;
	c_serverip->SetWindowText(str);
}

void CFTPServerDlg::OnBnServerStart()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
	v = MAKEWORD(1, 1);
	WSAStartup(v, &wsData); // 加载套接字库    
	cServer.m_serverip = m_serverip;
	cServer.m_serverport = m_port;
	cServer.m_maxlink = m_maxlink;
	cServer.m_timeout = m_time;
	cServer.m_welcomeInfor = "Welcome FTP Server！";
	cServer.m_goodbyeInfor = "BYE!";
	if (cServer.ServerStart()) {
		SStartComponentsState(TRUE);
	}
	else {
		MessageBox(_T("服务器启动失败"));
	}
}



void CFTPServerDlg::OnBnServerStop()
{
	// TODO: 在此添加控件通知处理程序代码
	cServer.ServerStop();
	SStartComponentsState(FALSE);
	WSACleanup();

}
void CFTPServerDlg::SStartComponentsState(BOOL state)
{
	if (state)
	{//两种状态，如果你开启服务，就默认那些窗口为只读
		c_maxlink->SetReadOnly(TRUE);
		c_timeout->SetReadOnly(TRUE);
		c_serverport->SetReadOnly(TRUE);
		c_startbutton->EnableWindow(FALSE);
		c_stopbutton->EnableWindow(TRUE);

	}
	else {
		c_maxlink->SetReadOnly(FALSE);
		c_timeout->SetReadOnly(FALSE);
		c_serverport->SetReadOnly(FALSE);
		c_startbutton->EnableWindow(TRUE);
		c_stopbutton->EnableWindow(TRUE);
	}
}


void CFTPServerDlg::OnBnUserAdd()
{
	// TODO: 在此添加控件通知处理程序代码
	UserDlg dialog;
	if (dialog.DoModal() == IDOK)
	{
		for (int i = 0; i < m_cUserListCtrl.GetItemCount(); i++)
		{
			CString username;
			username = m_cUserListCtrl.GetItemText(i, 0);
			
			if (username.CompareNoCase(dialog.userAccountInfor.m_username) == 0)
			{
				MessageBox(_T("用户名重复"), _T("错误"), MB_ICONERROR);
				return;
			}
		}
		UserInformation userAccountInfor = dialog.userAccountInfor;

		int index = m_userList.Add(userAccountInfor);
		cServer.cSPro.m_UserAccountInforList.Add(userAccountInfor);
		/*for (int i = 0; i < cServer.cSPro.m_UserAccountInforList.GetSize(); i++) {
			MessageBox(cServer.cSPro.m_UserAccountInforList[i].m_username+" "+ cServer.cSPro.m_UserAccountInforList[i].m_password, _T("错误"), MB_ICONERROR);
		}*/
		int nItem = m_cUserListCtrl.InsertItem(0, userAccountInfor.m_username);

		m_cUserListCtrl.SetItemText(nItem, 1, userAccountInfor.m_path);//选择路径
		CString privileges = userAccountInfor.m_allowDownload ? _T("下载 ") : _T("");
		privileges += userAccountInfor.m_allowUpload ? _T("上传 ") : _T("");
		privileges += userAccountInfor.m_allowRename ? _T("重命名 ") : _T("");
		privileges += userAccountInfor.m_allowDelete ? _T("删除 ") : _T("");
		privileges += userAccountInfor.m_allowCreateDirectory ? _T("创建目录 ") : _T("");
		m_cUserListCtrl.SetItemText(nItem, 2, privileges);
		m_cUserListCtrl.SetItemData(nItem, index);//这个数据的设置很重要，以后寻找列表上的数据对应到数据结构中时要用到
		m_cUserListCtrl.SetItemState(nItem, LVIS_SELECTED, LVIS_SELECTED);
	}
	
}


void CFTPServerDlg::OnBnUserEdit()
{
	int selectUserIndex = m_cUserListCtrl.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if (selectUserIndex == -1)
		return;

	int arrayUserListIndex = m_cUserListCtrl.GetItemData(selectUserIndex);

	UserDlg userEditdlg;

	userEditdlg.userAccountInfor = m_userList[arrayUserListIndex];
	if (userEditdlg.DoModal() == IDOK)
	{
		m_userList[arrayUserListIndex] = userEditdlg.userAccountInfor;
		cServer.cSPro.m_UserAccountInforList[arrayUserListIndex] = userEditdlg.userAccountInfor;
		m_cUserListCtrl.SetItemText(selectUserIndex, 0, userEditdlg.userAccountInfor.m_username);

		m_cUserListCtrl.SetItemText(selectUserIndex, 1, userEditdlg.userAccountInfor.m_path);
		CString privileges = userEditdlg.userAccountInfor.m_allowDownload ? _T("下载 ") : _T("");
		privileges += userEditdlg.userAccountInfor.m_allowUpload ? _T("上传 ") : _T("");
		privileges += userEditdlg.userAccountInfor.m_allowRename ? _T("重命名 ") : _T("");
		privileges += userEditdlg.userAccountInfor.m_allowDelete ? _T("删除 ") : _T("");
		privileges += userEditdlg.userAccountInfor.m_allowCreateDirectory ? _T("创建目录 ") : _T("");
		m_cUserListCtrl.SetItemText(selectUserIndex, 2, privileges);
	}
}


void CFTPServerDlg::OnBnUserDelete()
{
	// TODO: 在此添加控件通知处理程序代码
	int selectUserIndex = m_cUserListCtrl.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);//选定用户，m_cUserListCtrl获得的是列表选中的第几行
	if (selectUserIndex == -1)//如果选中的行数是0行，退出
		return;

	// 获取选定用户的索引
	int UserIndex = m_cUserListCtrl.GetItemData(selectUserIndex);
	m_userList.RemoveAt(UserIndex);
	cServer.cSPro.m_UserAccountInforList.RemoveAt(UserIndex);
	// 从列表中删除项目
	m_cUserListCtrl.DeleteItem(UserIndex);

	//更新用户列表
	for (int i = 0; i < m_cUserListCtrl.GetItemCount(); i++)
	{
		int selectUserData = m_cUserListCtrl.GetItemData(i);
		if (selectUserData > UserIndex)
		{
			m_cUserListCtrl.SetItemData(i, selectUserData - 1);
		}
	}
}

void CFTPServerDlg::AddTraceLine(int nLevel, LPCTSTR pstrFormat, ...)
{
	//根据传入的参数格式化我们给出的数据
	CString str;
	va_list args;//申明va_list数据类型变量args，该变量访问变长参数列表中的参数。

	//处理变长参数
	va_start(args, pstrFormat);//在使用arg之前，必须调用va_start使得arg和可变参数进行关联。
	 //pstrFormat是省略号"..."前的一个参数名，va_start根据此参数，判断参数列表的起始位置。
	str.FormatV(pstrFormat, args);

	try
	{
		//在主界面中显示
		CLogMsg *pLogMsg = new CLogMsg; //CLogMsg是自定义的一个类
		GetLocalTime(&pLogMsg->m_sysTime);
		pLogMsg->m_nLevel = nLevel;
		pLogMsg->m_strText = str;
		m_QueueLock.Lock();
		m_LogQueue.AddTail(pLogMsg);
		m_QueueLock.Unlock();
		//schedule log action
		PostMessage(WM_ADDTRACELINE);//	WM_ADDTRACELINE是自定义的消息

	}
	catch (...)
	{
	}
}
void CFTPServerDlg::OnFTPStatusChange(int nType, LPCTSTR lpszText)
{
	AddTraceLine(nType, lpszText);
	
}
LRESULT CFTPServerDlg::OnAddTraceLine(WPARAM, LPARAM)
{

	try
	{
		//get first message in the queue
		CLogMsg *pLogMsg = (CLogMsg*)m_LogQueue.RemoveHead();
		m_TraceList.AddString(pLogMsg->m_strText);
		delete pLogMsg;
	}
	catch (...)
	{
		//something is wrong...
	}
	return TRUE;
}

void CFTPServerDlg::OnClickedCheck1()
{
	// TODO: 在此添加控件通知处理程序代码
	if (m_isanonymous == FALSE) {
		m_isanonymous = TRUE;
		state = m_isanonymous;
		createAnonymous();
	}
	else {
		m_isanonymous = FALSE;
		state = m_isanonymous;
		createAnonymous();
	}
}
void  CFTPServerDlg::createAnonymous() {
	// TODO: 在此处添加实现代码.
	if (state == TRUE) {
		//默认添加一个anonymous用户
		UserDlg dialog;
		UserInformation userAccountInfor = dialog.userAccountInfor;
		userAccountInfor.m_username = "anonymous";
		//默认只允许下载和重命名
		userAccountInfor.m_allowRename = TRUE;
		userAccountInfor.m_allowDownload = TRUE;

		int index = m_userList.Add(userAccountInfor);
		anon_index = index;
		cServer.cSPro.m_UserAccountInforList.Add(userAccountInfor);
		int nItem = m_cUserListCtrl.InsertItem(0, userAccountInfor.m_username);

		m_cUserListCtrl.SetItemText(nItem, 1, userAccountInfor.m_path);//选择路径
		CString privileges = userAccountInfor.m_allowDownload ? _T("下载 ") : _T("");//在界面上显示
		privileges += userAccountInfor.m_allowUpload ? _T("上传 ") : _T("");
		privileges += userAccountInfor.m_allowRename ? _T("重命名 ") : _T("");
		privileges += userAccountInfor.m_allowDelete ? _T("删除 ") : _T("");
		privileges += userAccountInfor.m_allowCreateDirectory ? _T("创建目录 ") : _T("");
		m_cUserListCtrl.SetItemText(nItem, 2, privileges);
		m_cUserListCtrl.SetItemData(nItem, index);//这个数据的设置很重要，以后寻找列表上的数据对应到数据结构中时要用到
		m_cUserListCtrl.SetItemState(nItem, LVIS_SELECTED, LVIS_SELECTED);
	}
	else {
		// TODO: Add your control notification handler code here
		// 获取选定用户的索引
		UserDlg dialog;
		int UserIndex;
		for (int i = 0; i < m_cUserListCtrl.GetItemCount(); i++)
		{
			CString username;
			username = m_cUserListCtrl.GetItemText(i, 0);

			if (username.CompareNoCase("anonymous") == 0)
			{
				UserIndex = i;
				break;
			}
		}
		//int UserIndex = m_cUserListCtrl.GetItemData(anon_index);
		m_userList.RemoveAt(UserIndex);
		cServer.cSPro.m_UserAccountInforList.RemoveAt(UserIndex);
		// 从列表中删除项目
		m_cUserListCtrl.DeleteItem(UserIndex);

		//更新用户列表
		for (int i = 0; i < m_cUserListCtrl.GetItemCount(); i++)
		{
			int selectUserData = m_cUserListCtrl.GetItemData(i);
			if (selectUserData > UserIndex)
			{
				m_cUserListCtrl.SetItemData(i, selectUserData - 1);
			}
		}
	}

}
