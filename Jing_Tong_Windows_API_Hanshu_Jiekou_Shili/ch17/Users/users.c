/* ************************************
*����ͨWindows API��
* ʾ������
* users.c
* 17.3 �û����û���
**************************************/
/* UNICODE */
#ifndef UNICODE
#define UNICODE
#endif
/* ͷ�ļ� */
#include <stdio.h>
#include <assert.h>
#include <windows.h> 
#include <lm.h>

/*************************************
* AddUser
* ����	�����û�
* ����	szServerName�������������Ϊ���������û�������ΪNULL
*			szUserName���û���
*			szPassword������
**************************************/
int AddUser(LPWSTR szServerName, 
			LPWSTR szUserName,
			LPWSTR szPassword)
{
	USER_INFO_1 ui;
	DWORD dwLevel = 1;	// ʹ�� USER_INFO_1 ��Ϊ����
	DWORD dwError = 0;
	NET_API_STATUS nStatus;
	// ��� USER_INFO_1
	ui.usri1_name = szUserName;	// �û���
	ui.usri1_password = szPassword;	// ����
	ui.usri1_priv = USER_PRIV_USER;	// privilege  
	ui.usri1_home_dir = NULL;
	ui.usri1_comment = NULL;
	ui.usri1_flags = UF_SCRIPT;
	ui.usri1_script_path = NULL;
	// ���� NetUserAdd �����û�
	nStatus = NetUserAdd(szServerName,
		dwLevel,
		(LPBYTE)&ui,
		&dwError);

	// �жϽ��
	if (nStatus == NERR_Success)
	{
		wprintf(stderr, L"User %s has been successfully added on %s\n",
		szUserName, szServerName);
	}
	else
	{
		fprintf(stderr, "A system error has occurred: %d\n", nStatus);
	}
	return 0;
}

/*************************************
* AddUserToGroup
* ����	Ϊ�û��������û�
* ����	szServerName�������������Ϊ����������ΪNULL
*			szUserName���û���
*			szGroup���û�����
**************************************/
int AddUserToGroup(LPWSTR szServerName, 
				   LPWSTR szUserName,  
				   LPWSTR szGroup)
{
	NET_API_STATUS nStatus;
	// ���� NetGroupAddUser
	nStatus =  NetGroupAddUser(
		szServerName,
		szGroup,
		szUserName
		);

	// �жϽ��
	if (nStatus == NERR_Success)
		fwprintf(stderr, L"User %s has been successfully added on %s\n",
		szUserName, szServerName);

	else
		fprintf(stderr, "NetGroupAddUser A system error has occurred: %d\n", nStatus);
	return 0;
}

/*************************************
* DelUser
* ����	ɾ���û�
* ����	szServerName�������������Ϊ����������ΪNULL
*			szUserName���û���
**************************************/
int DelUser(LPWSTR szServerName, LPWSTR szUserName)
{
	DWORD dwError = 0;
	NET_API_STATUS nStatus;

	// ���� NetUserDel ɾ���û�
	nStatus = NetUserDel(szServerName, szUserName);
	// �жϲ���ʾ���
	if (nStatus == NERR_Success)
		fwprintf(stderr, L"User %s has been successfully deleted on %s\n",
		szUserName, szServerName);
	else
		fprintf(stderr, "A system error has occurred: %d\n", nStatus);

	return 0;

}

/*************************************
* int ListUsers(LPWSTR pszServerName)
* ����	�о��û�
* ����	szServerName�������������Ϊ����������ΪNULL
**************************************/
int ListUsers(LPWSTR pszServerName)
{
	LPUSER_INFO_0 pBuf = NULL;
	LPUSER_INFO_0 pTmpBuf;
	DWORD dwLevel = 0;
	DWORD dwPrefMaxLen = MAX_PREFERRED_LENGTH;
	DWORD dwEntriesRead = 0;
	DWORD dwTotalEntries = 0;
	DWORD dwResumeHandle = 0;
	DWORD i;
	DWORD dwTotalCount = 0;
	NET_API_STATUS nStatus;

	// ѭ����ֱ�����Գɹ����� NetUserEnum
	do 
	{
		// ����NetUserEnum����
		nStatus = NetUserEnum(pszServerName,
			dwLevel,// ��������Ϊ0��ʹ�� LPUSER_INFO_0 ���ؽ��
			FILTER_NORMAL_ACCOUNT, // ֻ�о١����������͵��û�
			(LPBYTE*)&pBuf,// LPUSER_INFO_0 ���淵�ؽ��
			// MAX_PREFERRED_LENGTH���ڴ���API���䣬��Ҫ��֮�����NetApiBufferFree�ͷ�
			dwPrefMaxLen,
			&dwEntriesRead,// ���˵� Entries
			&dwTotalEntries,// һ���� Entries
			&dwResumeHandle);
		// �ж��Ƿ�ɹ�
		if ((nStatus == NERR_Success) || (nStatus == ERROR_MORE_DATA))
		{
			if ((pTmpBuf = pBuf) != NULL)
			{
				// ѭ����ȡ�û���Ϣ
				for (i = 0; (i < dwEntriesRead); i++)
				{
					assert(pTmpBuf != NULL);

					if (pTmpBuf == NULL)
					{
						fprintf(stderr, "An access violation has occurred\n");
						break;
					}
					// ���
					wprintf(L"\t-- %s\n", pTmpBuf->usri0_name);
					// ��һ��
					pTmpBuf++;
					dwTotalCount++;
				}
			}
		}
		else
			fprintf(stderr, "A system error has occurred: %d\n", nStatus);
		// �ͷ��ڴ�
		if (pBuf != NULL)
		{
			NetApiBufferFree(pBuf);
			pBuf = NULL;
		}
	}
	while (nStatus == ERROR_MORE_DATA); // end do

	// �ͷ��ڴ�
	if (pBuf != NULL)
		NetApiBufferFree(pBuf);

	fprintf(stderr, "Total of %d users\n\n", dwTotalCount);

	return 0;
}

/*************************************
* int ListGroup(LPWSTR pszServerName)
* ����	�о��û���
* ����	szServerName�������������Ϊ����������ΪNULL
**************************************/
int ListGroup(LPWSTR pszServerName)
{

	DWORD dwLevel = 0;
	DWORD dwPrefMaxLen = MAX_PREFERRED_LENGTH;
	DWORD dwEntriesRead = 0;
	DWORD dwTotalEntries = 0;
	DWORD dwResumeHandle = 0;
	DWORD i;
	DWORD dwTotalCount = 0;
	NET_API_STATUS nStatus;


	LPLOCALGROUP_INFO_0 pBuf = NULL;
	LPLOCALGROUP_INFO_0 pTmpBuf;

	do // begin do
	{
		// ����NetLocalGroupEnum ����������NetLocalGroup����
		nStatus = NetLocalGroupEnum(
			pszServerName,
			0,
			(LPBYTE*)&pBuf,
			dwPrefMaxLen,
			&dwEntriesRead,
			&dwTotalEntries,
			&dwResumeHandle);
		// �жϽ��
		if ((nStatus == NERR_Success) || (nStatus == ERROR_MORE_DATA))
		{
			if ((pTmpBuf = pBuf) != NULL)
			{
				// ѭ�����
				for (i = 0; (i < dwEntriesRead); i++)
				{
					assert(pTmpBuf != NULL);

					if (pTmpBuf == NULL)
					{
						fprintf(stderr, "An access violation has occurred\n");
						break;
					}

					wprintf(L"\t-- %s\n", pTmpBuf->lgrpi0_name);
					pTmpBuf++;
					dwTotalCount++;
				}
			}
		}

		else
			fprintf(stderr, "A system error has occurred: %d\n", nStatus);
		// �ͷ��ڴ�
		if (pBuf != NULL)
		{
			NetApiBufferFree(pBuf);
			pBuf = NULL;
		}
	}

	while (nStatus == ERROR_MORE_DATA); // end do

	if (pBuf != NULL)
		NetApiBufferFree(pBuf);

	fprintf(stderr, "Total of %d groups\n\n", dwTotalCount);

	return 0;
}

/*************************************
* ShowUsersInfo
* ����	��ʾָ���û�����Ϣ
* ����	szServerName�������������Ϊ����������ΪNULL
*			pszUserName���û���
**************************************/
int ShowUsersInfo(LPWSTR pszServerName,LPWSTR pszUserName)
{

	DWORD dwLevel = 4;// ʹ�� LPUSER_INFO_4 ���ؽ��
	LPUSER_INFO_4 pBuf = NULL;
	NET_API_STATUS nStatus;

	nStatus = NetUserGetInfo(pszServerName,
		pszUserName,
		dwLevel,	// pBuf��������
		(LPBYTE *)&pBuf);
	
	// �жϲ�������
	if (nStatus == NERR_Success)
	{
		if (pBuf != NULL)
		{
			wprintf(L"\n\tAccount:      %s\n", pBuf->usri4_name);
			wprintf(L"\tComment:      %s\n", pBuf->usri4_comment);
			wprintf(L"\tUser comment: %s\n", pBuf->usri4_usr_comment);
			wprintf(L"\tFull name:    %s\n", pBuf->usri4_full_name);
			wprintf(L"\tpriv:    %d\n", pBuf->usri4_priv);
		}
	}

	else
		fprintf(stderr, "A system error has occurred: %d\n", nStatus);
	// �ͷ��ڴ�
	if (pBuf != NULL)
		NetApiBufferFree(pBuf);
	return 0;

}

/*************************************
* wmain
* ����	��ں��������ݲ����ж���Ҫ���õĹ��ܺ���
* ����	�μ�usage���
**************************************/
int __cdecl wmain(int ac, wchar_t * av[])
{

	if (ac == 4 && lstrcmpW( av[1], L"-a") == 0)
	{
		AddUser(NULL, av[2], av[3]);
	}

	else if (ac == 4 && lstrcmpW( av[1], L"-g") == 0)
	{
		AddUserToGroup(NULL, av[2], av[3]);
	}
	else 	if (ac == 3 && lstrcmpW( av[1], L"-i") == 0)
	{
		ShowUsersInfo(NULL, av[2]);
	}
	else 	if (ac == 2 && lstrcmpW( av[1], L"-i") == 0)
	{
		ListUsers(NULL);
		ListGroup(NULL);
	}
	else	if (ac == 3 && lstrcmpW( av[1], L"-d") == 0)
	{
		DelUser(NULL, av[2]);
	}
	else
	{
		printf("usage: \n"
			"\t %ws -a <username> <password> to add a user\n"
			"\t %ws -g <username> <group> add a user to a group"
			"\t %ws -i <username> to show user info\n"
			"\t %ws -d <username> to del a user\n", 
			av[0], av[0], av[0], av[0]);
	}
	return 0;
}