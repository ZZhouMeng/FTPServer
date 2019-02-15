#include "stdafx.h"
#include "ServerAction.h"
#include "UserInformation.h"


ServerAction::ServerAction()
{
}


ServerAction::~ServerAction()
{
}

BOOL ServerAction::GetUserAccount(LPCTSTR lpszUser, UserInformation &useraccount)
{
	
	for (int i = 0; i < m_UserAccountInforList.GetSize(); i++)
	{
		
		if (!m_UserAccountInforList[i].m_username.CompareNoCase(lpszUser))
		{
			useraccount = m_UserAccountInforList[i];
			return TRUE;
		}
	}
	
	return false;
};

int ServerAction::CheckFileName(LPCTSTR lpszUser, CString strFilename, CString strCurrentdir, int nOption, CString &strResult)
{
	// make unix style
	strFilename.Replace("\\", "/");
	while (strFilename.Replace("//", "/"));
	strFilename.TrimRight("/");

	if (strFilename == "")
	{
		// no file name
		return 2;
	}

	// 将文件名附加到目录
	CString strDirectory = strCurrentdir;

	// client has specified complete path客户机已指定完整路径 
	int nPos = strFilename.ReverseFind('/');
	if (nPos != -1)
	{
		strDirectory = strFilename.Left(nPos);
		if (strDirectory == "")
			strDirectory = "/";
		strFilename = strFilename.Mid(nPos + 1);
	}

	// get local path
	CString strLocalPath;
	if (!ConvertPathToLocal(lpszUser, strDirectory, strLocalPath))
	{
		// directory does not exist
		return 2;
	}

	// create the complete path
	strResult = strLocalPath + "\\" + strFilename;

	if ((nOption != FTP_UPLOAD) && !FileExists(strResult, FALSE))
	{
		// file does not exist
		return 2;
	}

	// 返回相对路径
	if (nOption == FTP_LIST)
	{
		strResult = strCurrentdir;
		strResult.TrimRight('/');
		strResult += "/" + strFilename;
		return 0;
	}

	// 检查文件访问权限
	if (!CheckAccessRights(lpszUser, strLocalPath, nOption))
	{
		// 用户没有权限执行指定的操作
		return 1;
	}
	// everything is ok
	return 0;
}


BOOL ServerAction::ConvertPathToLocal(LPCTSTR lpszUser, CString &strDirectoryIn, CString &strDirectoryOut)//将相对路径转换为本地路径
{
	UserInformation useraccount;
	if (!GetUserAccount(lpszUser, useraccount))
	{
		// user not valid
		return FALSE;
	}

	CStringList partList;
	CString strSub;
	int nCount = 0;

	// split path in parts
	while (AfxExtractSubString(strSub, strDirectoryIn, nCount++, '/'))
	{
		if (!strSub.IsEmpty())
			partList.AddTail(strSub);
	}


	CString strHomeDir = useraccount.m_path;
	while (!partList.IsEmpty())
	{
		CString strPart = partList.GetHead();
		partList.RemoveHead();

		CString strCheckDir;

		if (strPart == "..")
		{
			// go back one level
			int nPos = strHomeDir.ReverseFind('\\');
			if (nPos != -1)
			{
				strCheckDir = strHomeDir.Left(nPos);
			}
		}
		else
		{
			strCheckDir = strHomeDir + "\\" + strPart;
		}

		// does directory exist ?
		if (FileExists(strCheckDir, TRUE))
		{
			strHomeDir = strCheckDir;
		}
		// does file exist ?
		else if (FileExists(strCheckDir, FALSE))
		{
			strHomeDir = strCheckDir;
		}
	}

	// successfully converted directory
	strDirectoryOut = strHomeDir;
	return TRUE;
}
int ServerAction::CheckDirectory(LPCTSTR lpszUser, CString strDirectory, CString strCurrentdir, int nOption, CString &strResult)//检查目录是否存在,如果用户
{
	// make unix compatible
	strDirectory.Replace("\\", "/");
	while (strDirectory.Replace("//", "/"));
	strDirectory.TrimRight("/");

	if (strDirectory == "")
	{
		// no directory
		return 2;
	}
	else
	{
		// first character '/' ?
		if (strDirectory.Left(1) != "/")
		{
			// 客户机指定了与其当前路径相对应的路径
			strCurrentdir.TrimRight("/");
			strDirectory = strCurrentdir + "/" + strDirectory;
		}
	}

	// split part into 2 parts
	int nPos = strDirectory.ReverseFind('/');
	if (nPos == -1)
		return 2;

	// get left part of directory
	CString strNode = strDirectory.Left(nPos);
	// root ?
	if (strNode == "")
		strNode = "/";
	// get right part of directory
	strDirectory = strDirectory.Mid(nPos + 1);

	CString strLocalPath;

	do
	{
		// does parent directory exist ?是否存在父目录
		if ((!ConvertPathToLocal(lpszUser, strNode, strLocalPath)) && (nOption == FTP_CREATE_DIR))
		{
			// directory could not be found, maybe one level higher
			int nPos = strNode.ReverseFind('/');
			// no more levels
			if (nPos == -1)
				return 2;

			strDirectory = strNode.Mid(nPos + 1) + "/" + strDirectory;
			strNode = strNode.Left(nPos);
			continue;
		}

		//检查目录访问权限
		if (!CheckAccessRights(lpszUser, strLocalPath, nOption))
		{
			// user has no permissions, to execute specified action用户没有权限执行指定的操作
			return 1;
		}

		strNode = strLocalPath;
		break;
	} while (strNode != "/");

	strDirectory.Replace("/", "\\");
	strResult = strNode + "\\" + strDirectory;

	// check if directory exists
	if (!FileExists(strResult, TRUE))
		return 2;

	// function successfull
	return 0;
}
BOOL ServerAction::FileExists(LPCTSTR lpszFileName, BOOL bIsDirCheck)//检查文件或目录是否存在
{

	DWORD dwAttributes = GetFileAttributes(lpszFileName);//判断属性是否存在
	if (dwAttributes == 0xFFFFFFFF)//通过函数GetFileAttributes，如果返回值是0xFFFFFFFF，表示文件不存在
		return FALSE;

	if ((dwAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
	{
		if (bIsDirCheck)
			return TRUE;
		else
			return FALSE;
	}
	else
	{
		if (!bIsDirCheck)
			return TRUE;
		else
			return FALSE;
	}
}
//检查用户是否可以访问指定的目录
BOOL ServerAction::CheckAccessRights(LPCTSTR lpszUser, LPCTSTR lpszDirectory, int nOption)
{
	UserInformation useraccount;
	if (!GetUserAccount(lpszUser, useraccount))
	{
		// user not valid
		return FALSE;
	}

	// 从完整路径开始
	CString strCheckDir = lpszDirectory;

	while (!strCheckDir.IsEmpty())
	{
		CString strPath1 = strCheckDir;
		strPath1.TrimRight("\\");
		CString strPath2 = useraccount.m_path;
		strPath2.TrimRight("\\");

		if (strPath1.CompareNoCase(strPath2) == 0)
		{
			// 检查文件访问权限
			if (((!useraccount.m_allowDownload) && (nOption == FTP_DOWNLOAD)) ||
				((!useraccount.m_allowUpload) && (nOption == FTP_UPLOAD)) ||
				((!useraccount.m_allowRename) && (nOption == FTP_RENAME)) ||
				((!useraccount.m_allowDelete) && (nOption == FTP_DELETE)) ||
				((!useraccount.m_allowCreateDirectory) && (nOption == FTP_CREATE_DIR)))
			{
				return FALSE;
			}
			return TRUE;
		}

		int nPos = strCheckDir.ReverseFind('\\');
		if (nPos != -1)
		{
			// strip subdir 
			strCheckDir = strCheckDir.Left(nPos);
		}
		else
		{
			// we're done
			strCheckDir.Empty();
		}
	}
	// users has no rights to this directory
	return FALSE;
}

int ServerAction::GetDirectoryList(LPCTSTR lpszUser, LPCTSTR lpszDirectory, CString &strResult)//获取指定目录的目录列表
{
	CString strDirectory = lpszDirectory;

	// make unix style
	strDirectory.Replace("\\", "/");
	while (strDirectory.Replace("//", "/"));

	// clear list
	strResult = "";
	UserInformation useraccount;
	if (!GetUserAccount(lpszUser, useraccount))
	{

		// user not found -> no permissions
		return 1;
	}

	CString strLocalPath;
	if (!ConvertPathToLocal(lpszUser, strDirectory, strLocalPath))
	{
		// unable to convert to local path
		return 2;
	}

	// check if user has access right for this directory
	if (!CheckAccessRights(lpszUser, strLocalPath, FTP_DOWNLOAD))
	{
		// user has no permissions, to display this directory
		return 1;
	}

	CFileFind find;
	BOOL bFound = FALSE;

	// check if it's a directory
	if ((GetFileAttributes(strLocalPath) & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
	{
		bFound = find.FindFile(strLocalPath + "\\*.*");
	}
	else
	{
		// it's a file
		bFound = find.FindFile(strLocalPath);
	}

	while (bFound)
	{
		bFound = find.FindNextFile();

		// skip "." and ".." 
		if (find.IsDots())
			continue;

		// permissions
		if (find.IsDirectory())
			strResult += "drwx------";
		else
			strResult += "-rwx------";

		// groups
		strResult += " 1 user group ";
		// file size
		CString strLength;
		strLength.Format("%d", find.GetLength());
		CString strFiller = "              ";
		strResult += strFiller.Left(strFiller.GetLength() - strLength.GetLength());
		strResult += strLength;
		// file date
		strResult += GetFileDate(find);
		// file name
		strResult += find.GetFileName();
		// end of line
		strResult += "\r\n";
	}

	return 0;
}
CString ServerAction::GetFileDate(CFileFind &find)//以UNIX风格返回文件日期
{
	CString strResult;

	CTime time = CTime::GetCurrentTime();

	find.GetLastWriteTime(time);

	CTimeSpan timeSpan = CTime::GetCurrentTime() - time;

	if (timeSpan.GetDays() > 356)
	{
		strResult = time.Format(" %b %d %Y ");
	}
	else
	{
		strResult.Format(" %s %02d:%02d ", time.Format("%b %d"), time.GetHour(), time.GetMinute());
	}

	return strResult;
}
int ServerAction::ChangeDirectory(LPCTSTR lpszUser, CString &strCurrentdir, CString &strChangeTo)//改变到指定目录
{
	// make unix style
	strChangeTo.Replace(_T("\\"), _T("/"));
	while (strChangeTo.Replace(_T("//"), _T("/")));
	strChangeTo.TrimRight(_T("/"));

	// now looks something like this: 
	// ""				= root
	// "/mydir/apps"	= absolute path
	// "mydir/apps"		= relative path

	if (strChangeTo == "")
	{
		// goto root转到根
		strChangeTo = "/";
	}
	else
	{
		// first character '/' ?
		if (strChangeTo.Left(1) != "/")
		{
			// client specified a path relative to their current path客户端指定一个路径相对于他们当前的路径
			strCurrentdir.TrimRight(_T("/"));
			strChangeTo = strCurrentdir + "/" + strChangeTo;
		}
	}

	// goto parent directory转到父目录
	if (strChangeTo.Right(2) == _T(".."))
	{
		strChangeTo.TrimRight(_T(".."));
		strChangeTo.TrimRight(_T("/"));
		int nPos = strChangeTo.ReverseFind('/');
		if (nPos != -1)
		{
			strChangeTo = strChangeTo.Left(nPos);
		}
		if (strChangeTo == "")
		{
			// goto root
			strChangeTo = "/";
		}
	}

	// get local path
	CString strLocalPath;
	if (!ConvertPathToLocal(lpszUser, strChangeTo, strLocalPath))
	{
		// 无法转换到本地路径
		return 2;
	}

	// check if user has access right for this directory
	if (!CheckAccessRights(lpszUser, strLocalPath, FTP_DOWNLOAD))
	{
		// user has no permissions
		return 1;
	}

	// everything went successfully
	strCurrentdir = strChangeTo;
	return 0;
}
