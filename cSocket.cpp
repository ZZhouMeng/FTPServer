// cSocket.cpp : implementation file
//

#include "stdafx.h"
#include "FTPServer.h"
#include "cSocket.h"
#include "UserInformation.h"
#include "Server.h"
#include "dataSocket.h"

/////////////////////////////////////////////////////////////////////////////
// cSocket
extern Server cServer;
extern bool state;
typedef struct myname
{
	char name[100];
	int year;
}myname;
cSocket::cSocket()
{
	m_bPassiveMode = FALSE;
	m_strRemoteHost = "";
	m_nRemotePort = -1;
	m_dwRestartOffset = 0;
	m_bLoggedon = FALSE;
	m_pDataSocket = NULL;
}

cSocket::~cSocket()
{

}
void cSocket::OnReceive(int nErrorCode)
//每个线程都是通过一个CConnectSocket对象m_ConnectSocket来完成数据的接受和发送。当线程创建成功以后，
//m_ConnectSocket对象通过OnReceive函数获得数据，然后利用GetRxLine()函数来解析其中FTP命令
{

	myname t;
	t.year = 1;
	char buff[4096];
	ZeroMemory(buff, 4096);
	int nRead = Receive(buff, 4096);//将数据接收到临时缓冲区中
	if (nRead != SOCKET_ERROR && nRead != 0)
	{
		buff[nRead] = '\0';
		m_RxBuffer += CString(buff);//将接收到的内容添加到全局缓冲区中
		GetRxLine();
	}
	CSocket::OnReceive(nErrorCode);
}
void cSocket::OnClose(int nErrorCode) {//socket被关闭时的一个响应

	CSocket::OnClose(nErrorCode);
}
void cSocket::GetRxLine()//解析整个命令行函数
{
	CString strTemp;
	int nIndex;
	while (!m_RxBuffer.IsEmpty())//有接收到的数据待处理
	{
		//找一条完整命令的结束符
		nIndex = m_RxBuffer.Find("\r\n");
		if (nIndex != -1)
		{
			strTemp = m_RxBuffer.Left(nIndex);//把这条命令提取出来
			m_RxBuffer = m_RxBuffer.Mid(nIndex + 2);//更新m_RxBuffer 去掉已经提取出来的命令
			if (!strTemp.IsEmpty())
			{
				m_strCommands.AddTail(strTemp);//可能while循环中提取出多条命令，这里增加一个队列
				//解析并执行命令
				DoFtpCmd(); //去处理这些命令,如果直接处理命令的话，就没有上面m_strCommandsz这个队列缓冲了
			}
		}
		else
			break;
	}
}
void cSocket::parserCmd(CString &cmdName, CString &cmdArg)//从接收缓冲区获取命令
{
	CString strBuff = m_strCommands.RemoveHead();
	int nIndex = strBuff.Find(" ");
	if (nIndex != -1)
	{
		CString strPassword = strBuff;
		cmdName = strBuff.Left(nIndex);
		cmdArg = strBuff.Mid(nIndex + 1);
	}
	else
	{
		cmdName = strBuff;
	}
	if (strcmp(cmdName, "") == 0)
	{
		cmdName = "";
	}
	else {
		cmdName.MakeUpper();
	}
}
void cSocket::DoFtpCmd()//它根据客户端提交的各种命令进行相应的操作
{

	CString cmdName;
	CString cmdArg;
	parserCmd(cmdName, cmdArg);

	int cmdIndex = FTP_CMD_USER;
	//查找命令
	for (cmdIndex; cmdIndex < FTP_CMD_ERROR; cmdIndex++)
	{
		if (state == TRUE && m_strUserName == "anonymous"&&m_bLoggedon==FALSE) {
			cmdArg = "123";
		}
		//发现命令
		if (cmdName == commandList[cmdIndex].ftp_cmd_name)
		{
			if (commandList[cmdIndex].ftp_cmd_arg && (cmdArg.IsEmpty()))
			{
				ResToClient("501 Syntax error:Invalid number of parameters.");

				return;
			}
			break;
		}
	}
	if (cmdIndex == FTP_CMD_ERROR)
	{
		ResToClient("501 Syntax error:Command not understood.");//无效的命令
		return;
	}
	if ((cmdIndex != FTP_CMD_USER && cmdIndex != FTP_CMD_PASS) && !m_bLoggedon)
		//没有成功登陆之前不希望输入指令
	{
		ResToClient("530 Please login with USER and PASS.");
		return;
	}

	switch (cmdIndex)
	{
	case FTP_CMD_USER://输入指定的用户名
	{
		cmdArg.MakeLower();//是所有大写字母变成小写
		m_bLoggedon = FALSE;
		m_strUserName = cmdArg;
		CString strPeerAddress;
		UINT nPeerPort;
		GetPeerName(strPeerAddress, nPeerPort);
		ResToClient("331 User name ok,need password.");//告诉服务器一个新用户已经连接上
		break;
	}
	case FTP_CMD_PASS://指定密码
	{
		if (m_bLoggedon)//是否已经登录
		{
			ResToClient("503 Login with USER first.");
		}
		else
		{
			if (state == TRUE && m_strUserName == "anonymous") {
				m_strCurrentDir = "/";
				//成功登录提示
				m_bLoggedon = TRUE;
				ResToClient(_T("230 User sucessfully logged in."));
				break;
			}
			else {
				//检查用户名和密码
				UserInformation useraccount;
				if (cServer.cSPro.GetUserAccount(m_strUserName, useraccount))
				{

					if ((!useraccount.m_password.Compare(cmdArg) || useraccount.m_password.IsEmpty()))
					{
						//设置用户主目录
						m_strCurrentDir = "/";
						//成功登录提示
						m_bLoggedon = TRUE;
						ResToClient(_T("230 User sucessfully logged in."));
						break;
					}
				}
				ResToClient(_T("530 Not logged in,user or password incorrect!"));
			}
		}
		break;
	}
	case FTP_CMD_QUIT:
	{
		//发送再见消息给客户机
		CConnectThread *pThread = (CConnectThread*)m_parentThread;
		ResToClient(_T("220 %s"), ((Server*)pThread->m_parentWindowServer)->m_goodbyeInfor);
		Close();
		//通知线程已经结束关闭连接
		pThread->PostThreadMessage(WM_THREADMESSAGE, 1, 0);
		break;
	}
	case FTP_CMD_SYST:
	{
		ResToClient("215 Windows FTP Server.");
		break;
	}
	case FTP_CMD_TYPE:
	{
		ResToClient(("200 Type set to %s"), cmdArg);
		break;
	}
	case FTP_CMD_CWD://改变当前目录
	{
		int nResult = cServer.cSPro.ChangeDirectory(m_strUserName, m_strCurrentDir, cmdArg);

		switch (nResult)
		{
		case 0:
			ResToClient(("250 CWD command successful.\"%s\" is current directory."), m_strCurrentDir);
			break;
		case 1:
			ResToClient(("550 CWD command failed.\"%s\":Permission denied."), cmdArg);
			break;
		default:
			ResToClient(("550 CWD command failed.\"%s\":Directory not found."), cmdArg);
			break;
		}
		break;
	}
	case FTP_CMD_PWD://打印当前目录
	{
		ResToClient(("257 \"%s\" is current directory."), m_strCurrentDir);
		break;
	}
	case FTP_CMD_PASV:
	{

		DestroyDataConnection();
		//创建新的数据套接字连接
		m_pDataSocket = new dataSocket(this, -1);
		//AfxMessageBox("开始了PASV！");
		if (!m_pDataSocket->Create())
		{
			DestroyDataConnection();
			ResToClient(_T("421 Failed to create socket."));
			break;
		}

		//开始侦听
		m_pDataSocket->Listen();
		m_pDataSocket->AsyncSelect();

		CString strIP, strTmp;
		UINT nPort;
		//获取我们的IP地址
		GetSockName(strIP, nPort);//strIP是本机的IP地址，nPort是21
								  //获取新的监听套接字的端口号
		m_pDataSocket->GetSockName(strTmp, nPort);
		strIP.Replace(("."), (","));
		//通知客户端要连接的IP地址和端口*/
		ResToClient(("227 Entering Passive Mode(%s,%d,%d)."), strIP, nPort / 256, nPort % 256);
		m_bPassiveMode = TRUE;
		break;
	}
	case FTP_CMD_PORT:
	{
		// specify IP and port (PORT a1,a2,a3,a4,p1,p2) -> IP address a1.a2.a3.a4, port p1*256+p2.
		CString strSub;
		int nCount = 0;
		while (AfxExtractSubString(strSub, cmdArg, nCount++, ','))
		{
			switch (nCount)
			{
			case 1://a1
				m_strRemoteHost = strSub;
				m_strRemoteHost += ".";
				break;
			case 2://a2
				m_strRemoteHost += strSub;
				m_strRemoteHost += ".";
				break;
			case 3://a3
				m_strRemoteHost += strSub;
				m_strRemoteHost += ".";
				break;
			case 4://a4
				m_strRemoteHost += strSub;
				break;
			case 5://p1
				m_nRemotePort = 256 * atoi(strSub);
				break;
			case 6://p2
				m_nRemotePort += atoi(strSub);
				break;
			}
		}
		m_bPassiveMode = FALSE;
		ResToClient(("200 Port command successful."));
		break;
	}
	case FTP_CMD_LIST:
	{
		//如果不是被动模式，我们先需要PORT命令指定端口
		if (!m_bPassiveMode && (m_strRemoteHost == "" || m_nRemotePort == -1))
		{
			ResToClient(("503 Bad sequence of commands."));
			//AfxMessageBox(_T("503 Bad sequence of commands."));
		}

		else
		{
			//如果用户没有指定路径，用当前路径
			if (cmdArg == "")
			{
				cmdArg = m_strCurrentDir;
			}

			else
			{
				//检查参数是文件名还是路径
				CString strResult;
				int nResult = cServer.cSPro.CheckFileName(m_strUserName, cmdArg, m_strCurrentDir, FTP_LIST, strResult);
				if (nResult == 0)
				{
					cmdArg = strResult;
				}
			}
			CString strListing;
			int nResult = cServer.cSPro.GetDirectoryList(m_strUserName, cmdArg, strListing);

			switch (nResult)
			{
			case 1:
				ResToClient(("550 Permission denied."));
				break;
			case 2:
				ResToClient(("550 Directory not found."));
				break;
			default:
				//创建套接字连接获得路径列表
				if (!CreateDataConnection(0, strListing))
				{
					DestroyDataConnection();
				}
				break;
			}
		}
		break;
	case FTP_CMD_NOOP:
	{
		ResToClient("220 ok");
	}
	break;
	case FTP_CMD_RETR:
	{
		//如果不是被动模式，我们先需要PORT命令指定端口
		if (!m_bPassiveMode && (m_strRemoteHost == "" || m_nRemotePort == -1))
		{
			ResToClient(("503 Bad sequence of commands."));
			break;
		}

		CString strResult;
		int nResult = cServer.cSPro.CheckFileName(m_strUserName, cmdArg, m_strCurrentDir, FTP_DOWNLOAD, strResult);

		switch (nResult)
		{
		case 1:
			ResToClient(("550 Permission denied."));
			break;
		case 2:
			ResToClient(("550 File not found."));
			break;
		default:
			//为文件传输创建一个套接字
			if (!CreateDataConnection(1, strResult))
			{
				DestroyDataConnection();
			}
			break;
		}
		break;
	}
	case FTP_CMD_STOR:
	{
		//如果不是被动模式，我们先需要PORT命令指定端口
		if (!m_bPassiveMode && (m_strRemoteHost == "" || m_nRemotePort == -1))
		{
			ResToClient(("503 Bad sequence of commands."));
			break;
		}

		CString strResult;
		int nResult = cServer.cSPro.CheckFileName(m_strUserName, cmdArg, m_strCurrentDir, FTP_UPLOAD, strResult);

		switch (nResult)
		{
		case 1:
			ResToClient(("550 Permission denied."));
			break;
		case 2:
			ResToClient(("550 Filename invalid."));
			break;
		default:
			//为文件传输创建一个套接字
			if (!CreateDataConnection(2, strResult))
			{
				DestroyDataConnection();
			}
			break;
		}
		break;
	}
	case FTP_CMD_REST:
	{

		if (!IsNumeric(cmdArg.GetBuffer(cmdArg.GetLength())))
		{
			cmdArg.ReleaseBuffer();
			ResToClient(("501 Invalid numeric!"));
		}
		else
		{
			cmdArg.ReleaseBuffer();
			m_dwRestartOffset = atol(cmdArg);
			ResToClient(("350 Restarting at %d."), m_dwRestartOffset);
		}
		break;
	}
	case FTP_CMD_DELE:
	{
		CString strResult;
		int nResult = cServer.cSPro.CheckFileName(m_strUserName, cmdArg, m_strCurrentDir, FTP_DELETE, strResult);
		ResToClient((""+nResult));
		switch (nResult)
		{
		case 1:
			ResToClient(("550 Permission denied."));
			break;
		case 2:
			ResToClient(("550 File not found."));
			break;
		default:
			//删除文件
			if (!DeleteFile(strResult))
			{
				ResToClient("450 Internal error deleting the file:\"%s\".", cmdArg);
			}
			else
			{
				ResToClient("250 File \"%s\" wsa deleted successfully.", cmdArg);
			}
			break;
		}
	}
	break;
	case FTP_CMD_BYE:
	{
		//发送再见消息给客户机
		CConnectThread *pThread = (CConnectThread*)m_parentThread;
		ResToClient(_T("220 %s"), ((Server*)pThread->m_parentWindowServer)->m_goodbyeInfor);
		Close();
		//通知线程已经结束关闭连接
		pThread->PostThreadMessage(WM_THREADMESSAGE, 1, 0);
		break;
	}
	case FTP_CMD_HELP:
	{
		//如果客户端不指定命令名显示所有命令
		if (cmdArg == "")
		{
			CString strResponse = "214-The following commands are recognized:\r\n";
			//在命令列表中查找命令
			for (int i = FTP_CMD_ABOR; i < FTP_CMD_ERROR; i++)
			{
				strResponse += commandList[i].ftp_cmd_name;
				strResponse += "\r\n";
			}
			strResponse += "214 HELP command successful.";
			ResToClient(strResponse);
		}
		else
		{
			int nHelpCmd;
			//在命令列表中查找命令
			for (nHelpCmd = FTP_CMD_ABOR; nHelpCmd < FTP_CMD_ERROR; nHelpCmd++)
			{
				//判断是否找到命令
				if (cmdArg.CompareNoCase(commandList[nHelpCmd].ftp_cmd_name) == 0)
				{
					break;
				}
			}
			if (nHelpCmd != FTP_CMD_ERROR)
			{
				//显示命令帮助信息
				ResToClient("214 %s", commandList[nHelpCmd].ftp_cmd_desc);
			}
			else
			{
				ResToClient("501 Unknown command %s", cmdArg);
			}
		}
		break;
	}
	case FTP_CMD_ABOR:
	{
		if (m_pDataSocket)
		{
			if (m_pDataSocket->GetStatus() != XFERMODE_IDLE)
			{
				ResToClient("426 Data connection closed.");
			}
			//销毁数据连接
			m_parentThread->PostThreadMessage(WM_THREADMESSAGE, 0, 0);
		}
		ResToClient("226 ABOR command successful.");
		break;
	}
	case FTP_CMD_RNFR:
	{
		//用户端重命名文件
		CString strResult;
		int nResult = cServer.cSPro.CheckFileName(m_strUserName, cmdArg, m_strCurrentDir, FTP_DELETE, strResult);

		if (nResult == 0)
		{
			m_strRenameFile = strResult;
			m_bRenameFile = TRUE;
			ResToClient("350 File exists, ready for destination name.");
			break;
		}
		else
		{

			nResult = cServer.cSPro.CheckDirectory(m_strUserName, cmdArg, m_strCurrentDir, FTP_DELETE, strResult);

			switch (nResult)
			{
			case 0:
				m_strRenameFile = strResult;
				m_bRenameFile = FALSE;
				ResToClient("350 Directory exists, ready for destination name.");
				break;
			case 1:
				ResToClient("550 Permission denied.");
				break;
			default:
				ResToClient("550 File/directory not found.");
				break;
			}
		}
	}
	break;

	// 重命名文件名或目录
	case FTP_CMD_RNTO:
	{
		if (m_strRenameFile.IsEmpty())
		{
			ResToClient("503 Bad sequence of commands.");
			break;
		}
		if (m_bRenameFile)
		{
			CString strResult;
			// 检查目标文件名
			int nResult = cServer.cSPro.CheckFileName(m_strUserName, cmdArg, m_strCurrentDir, FTP_DELETE, strResult);

			switch (nResult)
			{
			case 0:
				ResToClient("550 File already exists.");
				break;
			case 1:
				ResToClient("550 Permission denied.");
				break;
			default:
				// 重命名文件
				if (!MoveFile(m_strRenameFile, strResult))
				{
					ResToClient("450 Internal error renaming the file: \"%s\".", m_strRenameFile);
				}
				else
				{
					ResToClient("250 File \"%s\" renamed successfully.", m_strRenameFile);
				}
				break;
			}
		}
		else
		{
			CString strResult;
			// 检查目标路径名
			int  nResult = cServer.cSPro.CheckDirectory(m_strUserName, cmdArg, m_strCurrentDir, FTP_DELETE, strResult);

			switch (nResult)
			{
			case 0:
				ResToClient("550 Directory already exists.");
				break;
			case 1:
				ResToClient("550 Permission denied.");
				break;
			case 3:
				ResToClient("550 Directory invalid.");
				break;
			default:
				// 重命名路径
				if (!MoveFile(m_strRenameFile, strResult))
				{
					ResToClient("450 Internal error renaming the directory: \"%s\".", m_strRenameFile);
				}
				else
				{
					ResToClient("250 Directory \"%s\" renamed successfully.", m_strRenameFile);
				}
				break;
			}
		}
	}
	break;
	case FTP_CMD_CDUP:
	{
		cmdArg = "..";
		break;
	}
	case FTP_CMD_RMD:
	{
		CString strResult;
		int nResult = cServer.cSPro.CheckDirectory(m_strUserName, cmdArg, m_strCurrentDir, FTP_DELETE, strResult);

		switch (nResult)
		{
		case 1:
			ResToClient("550 Permission denied.");
			break;
		case 2:
			ResToClient("550 Directory not found.");
			break;
		default:
			//删除目录
			if (!RemoveDirectory(strResult))
			{
				if (GetLastError() == ERROR_DIR_NOT_EMPTY)
				{
					ResToClient("550 Directory not empty.");
				}
				else
				{
					ResToClient("450 Internal error deleting the directory.");
				}
			}
			else
			{
				ResToClient("250 Directory deleted successfully.");
			}
			break;
		}
	}
	break;
	case FTP_CMD_MKD:
	{
		CString strResult;
		int  nResult = cServer.cSPro.CheckFileName(m_strUserName, cmdArg, m_strCurrentDir, FTP_DELETE, strResult);

		switch (nResult)
		{
		case 0:
			ResToClient("550 Directory already exists.");
			break;
		case 1:
			ResToClient("550 Can't create directory.Permission denied.");
			break;
		default:
			//创建目录
			if (!MakeSureDirectoryPathExists(strResult))
			{
				ResToClient("450 Internal error creating the directory.");
			}
			else
			{
				ResToClient("250 Directory created successfully.");
			}
			break;
		}
	}
	break;
	}
	}
}
/*------------------创建数据连接------------------*/
BOOL cSocket::CreateDataConnection(int nTransferType, LPCTSTR lpszData)
{
	if (!m_bPassiveMode)
	{
		m_pDataSocket = new dataSocket(this, nTransferType);
		if (m_pDataSocket->Create())
		{
			m_pDataSocket->AsyncSelect();
			m_pDataSocket->SetRestartOffset(m_dwRestartOffset);//连接到远程站点
			m_pDataSocket->SetData(lpszData);

			if (m_pDataSocket->Connect(m_strRemoteHost, m_nRemotePort))
			{
				if (GetLastError() != WSAEWOULDBLOCK)
				{
					ResToClient(("425 Can't open data connection."));

					return FALSE;
				}
			}

			switch (nTransferType)
			{
			case 0:
				ResToClient(("150 Opening ASCII mode data connection for directory list."));
				break;
			case 1:
			case 2:
				ResToClient(("150 Opening BINARY mode data connection for file transfer."));
				break;
			}
		}
		else
		{
			AfxMessageBox(("421 Failed to create data connection socket."));
			return FALSE;
		}
	}
	else
	{
		m_pDataSocket->SetRestartOffset(m_dwRestartOffset);
		m_pDataSocket->SetData(lpszData);
		m_pDataSocket->SetTransferType(nTransferType, TRUE);
		//这3句执行完才会去执行m_pDataSocket的OnAccept()函数，所以在SetTransferType（）函数
		//中要判断在被动模式（服务器监听客户连接模式）下，是否客户来连接了，如果不判断的话，
		//OnAccept()将被跳过执行，即没有客户端来连接，这也是开始一直无法把数据发出去的原因!!
	}
	return TRUE;
}
/*-----------------------------------*/


void cSocket::DestroyDataConnection() //这是一个消息响应函数，也就是一个线程对象如果去接收并处理一个消息
{
	if (!m_pDataSocket)
		return;

	delete m_pDataSocket;

	// reset transfer status
	m_pDataSocket = NULL;
	m_strRemoteHost = "";
	m_nRemotePort = -1;
	m_dwRestartOffset = 0;
	m_bPassiveMode = FALSE;
}
BOOL cSocket::ResToClient(LPCTSTR lpszStatus, ...)//给客户机发送响应
{
	CString str;
	va_list args;
	va_start(args, lpszStatus);
	str.FormatV(lpszStatus, args);
	va_end(args);
	//判断连接是否在活动状态
	CString response = str + "\r\n";
	if (CSocket::Send(response, strlen(response), 0) == SOCKET_ERROR)
	{
		Close();//关闭线程连接
		return FALSE;
	}
	return TRUE;
}
BOOL cSocket::MakeSureDirectoryPathExists(LPCTSTR lpszDirPath)//创建多层目录
{
	CString strDirPath = lpszDirPath;
	int nPos = 0;//lpszDirPath为要创建的目录，如“c:\dir1\dir2\"
	while ((nPos = strDirPath.Find('\\', nPos + 1)) != -1)
	{
		CreateDirectory(strDirPath.Left(nPos), NULL);//CreateDirectory为创建单层函数目录
	}
	return CreateDirectory(strDirPath, NULL);
}
BOOL cSocket::IsNumeric(char *buff)//检测变量是否为数字或数字字符串
{
	//validate data 验证数据
	char *ptr = buff;
	while (*ptr != '\0')
	{
		if (isdigit(*ptr))//是数字返回true
		{
			ptr++;
		}
		else
			return FALSE;
	}
	return TRUE;
}


