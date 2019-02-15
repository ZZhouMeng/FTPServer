#pragma once
#include <afxsock.h>
class dataSocket;
enum // FTP 命令
{
	FTP_CMD_USER, FTP_CMD_PASS, FTP_CMD_QUIT, //访问命令 
	FTP_CMD_PASV, FTP_CMD_PORT, FTP_CMD_TYPE, //模式设置命令
	FTP_CMD_CWD, FTP_CMD_PWD, FTP_CMD_MKD, FTP_CMD_CDUP, FTP_CMD_RMD, FTP_CMD_LIST, FTP_CMD_NLST, FTP_CMD_RNFR, FTP_CMD_RNTO, FTP_CMD_DELE,//文件管理命令
	FTP_CMD_REST, FTP_CMD_RETR, FTP_CMD_STOR,  //文件传输命令
	FTP_CMD_ABOR, FTP_CMD_BYE, FTP_CMD_DIR, FTP_CMD_HELP, FTP_CMD_NOOP,/*FTP_CMD_SIZE,*/
	FTP_CMD_SYST,
	FTP_CMD_ERROR,
};
struct FtpCmd
{
	int ftp_cmd_id;
	char *ftp_cmd_name;
	BOOL ftp_cmd_arg;
	char *ftp_cmd_desc;
};
static FtpCmd commandList[] =
{

	{ FTP_CMD_USER,	"USER",  TRUE,	"Supply a username: USER username" },
	{ FTP_CMD_PASS,	"PASS",  TRUE,	"Supply a user password: PASS password" },
	{ FTP_CMD_QUIT,	"QUIT",  FALSE,	"Logout or break the connection: QUIT" },
	{ FTP_CMD_PASV,	"PASV",	FALSE,	"Set server in passive mode: PASV" },
	{ FTP_CMD_PORT,	"PORT", TRUE,	"Specify the client port number: PORT a0,a1,a2,a3,a4,a5" },
	{ FTP_CMD_TYPE,	"TYPE",  TRUE,	"Set filetype: TYPE [A | I]" },
	{ FTP_CMD_CWD,	"CWD",  TRUE,	"Change working directory: CWD [directory-name]" },
	{ FTP_CMD_PWD,	"PWD", FALSE,	"Get current directory: PWD" },
	{ FTP_CMD_MKD,	"MKD",	TRUE,	"Make directory: MKD path-name" },
	{ FTP_CMD_CDUP,	"CDUP", FALSE,	"Change to parent directory: CDUP" },
	{ FTP_CMD_RMD,	"RMD", TRUE,	"Remove directory: RMD path-name" },
	{ FTP_CMD_LIST,	"LIST",  FALSE,	"Get directory listing: LIST [path-name]" },
	{ FTP_CMD_NLST,	"NLST", FALSE,	"Change to parent directory: CDUP" },
	{ FTP_CMD_RNFR,	"RNFR",	TRUE,	"Specify old path name of file to be renamed: RNFR file-name" },
	{ FTP_CMD_RNTO,	"RNTO", TRUE,	"Specify new path name of file to be renamed: RNTO file-name" },
	{ FTP_CMD_DELE,	"DELE",  TRUE ,	"Delete file: DELE file-name" },
	{ FTP_CMD_REST,	"REST",  TRUE,	"Set restart transfer marker: REST marker" },
	{ FTP_CMD_RETR,	"RETR", TRUE,	"Get file: RETR file-name" },
	{ FTP_CMD_STOR,	"STOR",	TRUE,	"Store file: STOR file-name" },
	{FTP_CMD_ABOR,	"ABOR", FALSE,	"Abort transfer: ABOR"},
	{FTP_CMD_BYE,	"BYE",  FALSE,	"Logout or break the connection: BYE"},
	{FTP_CMD_DIR,	"DIR",  FALSE,	"Get directory listing: DIR [path-name]"},
	{FTP_CMD_HELP,	"HELP",  FALSE, "Show help: HELP [command]"},
	{FTP_CMD_NOOP,	"NOOP", FALSE,	"Do nothing: NOOP"},
	{FTP_CMD_SYST,	"SYST", FALSE,	"Get operating system type: SYST"},
	{ FTP_CMD_ERROR,"",	FALSE,	"" },

};
//MFC在CAsyncSocket基础上派生的一个同步阻塞Socket的封装类
class cSocket :
	public CSocket
{
public:
	cSocket();
	virtual ~cSocket();

public:
	CString m_RxBuffer;
	CStringList m_strCommands;
	BOOL m_bLoggedon;
	CString m_strUserName;
	CString m_strCurrentDir;
	CWinThread* m_parentThread;
	dataSocket* m_pDataSocket;
	CString m_strRenameFile;
	BOOL m_bRenameFile;
	BOOL m_bPassiveMode;
	int m_nRemotePort;
	CString m_strRemoteHost;
	DWORD m_dwRestartOffset;

public:
	BOOL ResToClient(LPCTSTR lpszStatus, ...);
	void GetRxLine();
	void DoFtpCmd();
	void parserCmd(CString &cmdName, CString &cmdArg);
	//void DestroyTransDataConnection();
	BOOL IsNumeric(char *buff);
	BOOL MakeSureDirectoryPathExists(LPCTSTR lpszDirPath);
	BOOL CreateDataConnection(int nTransferType, LPCTSTR lpszData);
	void DestroyDataConnection();
	virtual void OnReceive(int nErrorCode);
	virtual void OnClose(int nErrorCode);
};

