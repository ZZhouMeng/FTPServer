#include "stdafx.h"
#include "UserInformation.h"

IMPLEMENT_SERIAL(UserInformation, CObject, 1)//4在.cpp中加入IMPLEMENT_SERIAL(CClassName, CObject, 1 )；
UserInformation::UserInformation()
{
}


UserInformation::~UserInformation()
{
}

void UserInformation::Serialize(CArchive& archive)
{
	if (archive.IsStoring())
	{
		// 'store' data
		archive << m_username;
		archive << m_password;
		archive << m_path;
		//	archive << m_accountDisabled;
		archive << m_allowDownload;
		archive << m_allowUpload;
		archive << m_allowRename;
		archive << m_allowDelete;
		archive << m_allowCreateDirectory;
	}
	else
	{
		// 'load' data
		archive >> m_username;
		archive >> m_password;
		archive >> m_path;
		archive >> m_allowDownload;
		archive >> m_allowUpload;
		archive >> m_allowRename;
		archive >> m_allowDelete;
		archive >> m_allowCreateDirectory;
	}
}
UserInformation::UserInformation(const UserInformation &useracount)
{
	m_username = useracount.m_username;
	m_password = useracount.m_password;
	m_path = useracount.m_path;
	//	m_accountDisabled = useracount.m_accountDisabled;
	m_allowDownload = useracount.m_allowDownload;
	m_allowUpload = useracount.m_allowUpload;
	m_allowRename = useracount.m_allowRename;
	m_allowDelete = useracount.m_allowDelete;
	m_allowCreateDirectory = useracount.m_allowCreateDirectory;
}
UserInformation& UserInformation::operator=(const UserInformation &useracount)
{
	if (&useracount != this)
	{
		m_username = useracount.m_username;
		m_password = useracount.m_password;
		m_path = useracount.m_path;
		//	m_accountDisabled = useracount.m_accountDisabled;
		m_allowDownload = useracount.m_allowDownload;
		m_allowUpload = useracount.m_allowUpload;
		m_allowRename = useracount.m_allowRename;
		m_allowDelete = useracount.m_allowDelete;
		m_allowCreateDirectory = useracount.m_allowCreateDirectory;
	}
	return *this;
}
