#pragma once
#include "UserInformation.h"
#define FTP_DOWNLOAD	1
#define FTP_UPLOAD		2
#define FTP_RENAME		3
#define FTP_DELETE		4
#define FTP_CREATE_DIR	5
#define FTP_LIST		6

class ServerAction
{
public:
	ServerAction();
	virtual ~ServerAction();
public:
	CArray<UserInformation, UserInformation&> m_UserAccountInforList;
	BOOL GetUserAccount(LPCTSTR lpszUser, UserInformation &useraccount);
	int  GetDirectoryList(LPCTSTR lpszUser, LPCTSTR lpszDirectory, CString &strResult);
	BOOL FileExists(LPCTSTR lpszFileName, BOOL bIsDirCheck);
	CString GetFileDate(CFileFind &find);
	BOOL ConvertPathToLocal(LPCTSTR lpszUser, CString &strDirectoryIn, CString &strDirectoryOut);
	BOOL CheckAccessRights(LPCTSTR lpszUser, LPCTSTR lpszDirectory, int nOption);
	int  ChangeDirectory(LPCTSTR lpszUser, CString &strCurrentdir, CString &strChangeTo);
	int CheckFileName(LPCTSTR lpszUser, CString strFilename, CString strCurrentdir, int nOption, CString &strResult);
	int CheckDirectory(LPCTSTR lpszUser, CString strDirectory, CString strCurrentdir, int nOption, CString &strResult);
};

