// UserDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "FTPServer.h"
#include "UserDlg.h"
#include "afxdialogex.h"


// UserDlg 对话框

//IMPLEMENT_DYNAMIC(UserDlg, CDialogEx)

UserDlg::UserDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_UserDlg, pParent)
	, m_repass(_T(""))
{
	userAccountInfor.m_username = ("");
	userAccountInfor.m_path = ("F:\\FTP测试");
	userAccountInfor.m_password = ("");
	userAccountInfor.m_allowDownload = FALSE;
	userAccountInfor.m_allowRename = FALSE;
	userAccountInfor.m_allowUpload = FALSE;
	userAccountInfor.m_allowCreateDirectory = FALSE;
	userAccountInfor.m_allowDelete = FALSE;
}

UserDlg::~UserDlg()
{
}

void UserDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_USERNAME, userAccountInfor.m_username);
	DDX_Text(pDX, IDC_EDIT_PASS, userAccountInfor.m_password);
	DDX_Check(pDX, IDC_CHECK_MKDIR, userAccountInfor.m_allowCreateDirectory);
	DDX_Check(pDX, IDC_CHECK_DEL, userAccountInfor.m_allowDelete);
	DDX_Check(pDX, IDC_CHECK_DOWN, userAccountInfor.m_allowDownload);
	DDX_Check(pDX, IDC_CHECK_RENAME, userAccountInfor.m_allowRename);
	DDX_Check(pDX, IDC_CHECK_UP, userAccountInfor.m_allowUpload);
	DDX_Text(pDX, IDC_EDIT_FILEPATH, userAccountInfor.m_path);
	DDX_Text(pDX, IDC_EDIT_REPASS, m_repass);
}


BEGIN_MESSAGE_MAP(UserDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BTN_CHOOSEFILE, &UserDlg::OnBnChoosefile)
	ON_BN_CLICKED(IDOK, &UserDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &UserDlg::OnBnClickedCancel)
END_MESSAGE_MAP()


// UserDlg 消息处理程序


void UserDlg::OnBnChoosefile()
{
	// TODO: 在此添加控件通知处理程序代码
	BROWSEINFO sInfo;
	::ZeroMemory(&sInfo, sizeof(BROWSEINFO));
	sInfo.pidlRoot = 0; //要显示的文件夹的根
	sInfo.lpszTitle = _T("请选择一个文件夹");//选择处理结果的存储路径
	sInfo.ulFlags = BIF_DONTGOBELOWDOMAIN | BIF_RETURNONLYFSDIRS | BIF_EDITBOX;//指定对话框的外观和功能的标志
	//BIF_DONTGOBELOWDOMAIN：在树形视窗中，不包含域名底下的网络目录结构。
	//BIF_RETURNONLYFSDIRS：仅仅返回文件系统的目录。例如：在浏览文件夹对话框中，当选中任意一个目录时，该“OK”按钮可用，而当选中“我的电脑”或“网上邻居”等非有意义的节点时，“OK”按钮为灰色。
	//BIF_EDITBOX：浏览对话框中包含一个编辑框，在该编辑框中用户可以输入选中项的名字。
	sInfo.lpfn = NULL; //处理时间的回调函数
	LPITEMIDLIST lpidlBrowse = ::SHBrowseForFolder(&sInfo);//显示文件夹选择对话框
	TCHAR szFolderPath[MAX_PATH] = { 0 };
	::SHGetPathFromIDList(lpidlBrowse, szFolderPath);//取得文件夹名
	GetDlgItem(IDC_EDIT_FILEPATH)->SetWindowText(szFolderPath);//GetDlgItem()：接收一个控件ID，返回指向该窗口或控件的指针；SetWindowText()：接收一个文本值，设置对应控件的文本值；
}



void UserDlg::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData();
	if (strcmp(userAccountInfor.m_password, m_repass) != 0) {
		MessageBox(_T("密码不一致"), _T("错误"), MB_ICONERROR);;
		return;
	}
	CDialogEx::OnOK();
}


void UserDlg::OnBnClickedCancel()
{
	// TODO: 在此添加控件通知处理程序代码
	CDialogEx::OnCancel();
}
