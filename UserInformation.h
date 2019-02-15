#pragma once
#include <afx.h>

class UserInformation :
	public CObject
{
public:
	UserInformation();
	virtual ~UserInformation();

public:
	DECLARE_SERIAL(UserInformation);//3��.h�м��� DECLARE_SERIAL(CClassName)
	virtual void Serialize(CArchive&);//2����Serialize()��������������ɱ���Ͷ�ȡ����
	UserInformation(const UserInformation &user);				// copy-constructor
	UserInformation &operator=(const UserInformation &user);

public:
	CString m_username;  //�û���
	CString m_password; //�û�����
	CString m_path; //�ļ�·��
	BOOL m_allowDownload; //�Ƿ���������
	BOOL m_allowUpload; //�Ƿ������ϴ�
	BOOL m_allowRename; //�Ƿ�������
	BOOL m_allowDelete;  //�Ƿ�����ɾ��
	BOOL m_allowCreateDirectory; //�Ƿ�������Ŀ¼
};

