#pragma once
#include <afx.h>

class UserInformation :
	public CObject
{
public:
	UserInformation();
	virtual ~UserInformation();

public:
	DECLARE_SERIAL(UserInformation);//3在.h中加入 DECLARE_SERIAL(CClassName)
	virtual void Serialize(CArchive&);//2覆盖Serialize()函数，在其中完成保存和读取功能
	UserInformation(const UserInformation &user);				// copy-constructor
	UserInformation &operator=(const UserInformation &user);

public:
	CString m_username;  //用户名
	CString m_password; //用户密码
	CString m_path; //文件路径
	BOOL m_allowDownload; //是否允许下载
	BOOL m_allowUpload; //是否允许上传
	BOOL m_allowRename; //是否重命名
	BOOL m_allowDelete;  //是否允许删除
	BOOL m_allowCreateDirectory; //是否允许创建目录
};

