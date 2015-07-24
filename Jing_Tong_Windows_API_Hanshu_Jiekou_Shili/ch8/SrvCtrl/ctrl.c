/* ************************************
*����ͨWindows API�� 
* ʾ������
* ctrl.c
* 6.6	����
* ������Ƴ���������ֹͣ���������������
**************************************/
/* ͷ�ļ���*/
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
/* ȫ�ֱ�����*/
extern SC_HANDLE schService;	// ��init.c�ж��壬��ͬ
extern SC_HANDLE schSCManager;
extern LPTSTR szServiceName;

/*************************************
* BOOL StartSampleService(SC_HANDLE schSCManager,LPTSTR szServiceName) 
* ����	��������
*
* ����	SC_HANDLE schSCManager	SCM ���
*		LPTSTR szServiceName	������
**************************************/
BOOL StartSampleService(SC_HANDLE schSCManager,LPTSTR szServiceName) 
{ 
	SC_HANDLE schService;
	SERVICE_STATUS_PROCESS ssStatus; 
	DWORD dwOldCheckPoint; 
	DWORD dwStartTickCount;
	DWORD dwWaitTime;
	DWORD dwBytesNeeded;
	// �򿪷���
	schService = OpenService( 
		schSCManager,			// SCM database 
		szServiceName,          // service name
		SERVICE_ALL_ACCESS); 
	if (schService == NULL) 
	{ 
		return 0; 
	}
	// ��������
	if (!StartService(
		schService,  // handle to service 
		0,           // number of arguments 
		NULL) )      // no arguments 
	{
		printf("Service start error (%u).\n",GetLastError()); 
		return 0; 
	}
	else 
	{
		printf("Service start pending.\n"); 
	}

	// ��֤״̬
	if (!QueryServiceStatusEx( 
		schService,             // handle to service 
		SC_STATUS_PROCESS_INFO, // info level
		(LPBYTE)&ssStatus,              // address of structure
		sizeof(SERVICE_STATUS_PROCESS), // size of structure
		&dwBytesNeeded ) )              // if buffer too small
	{
		return 0; 
	}

	// tick count & checkpoint.
	dwStartTickCount = GetTickCount();
	dwOldCheckPoint = ssStatus.dwCheckPoint;
	// ��ѯ״̬��ȷ�� PENDING ״̬����
	while (ssStatus.dwCurrentState == SERVICE_START_PENDING) 
	{ 
		// �ȴ�һ��ʱ��
		dwWaitTime = ssStatus.dwWaitHint / 10;
		if( dwWaitTime < 1000 )
			dwWaitTime = 1000;
		else if ( dwWaitTime > 10000 )
			dwWaitTime = 10000;
		Sleep( dwWaitTime );
		// �ٴβ�ѯ
		if (!QueryServiceStatusEx( 
			schService,             // handle to service 
			SC_STATUS_PROCESS_INFO, // info level
			(LPBYTE)&ssStatus,              // address of structure
			sizeof(SERVICE_STATUS_PROCESS), // size of structure
			&dwBytesNeeded ) )              // if buffer too small
			break; 
		if ( ssStatus.dwCheckPoint > dwOldCheckPoint )
		{
			// ���̴�����
			dwStartTickCount = GetTickCount();
			dwOldCheckPoint = ssStatus.dwCheckPoint;
		}
		else
		{
			if(GetTickCount()-dwStartTickCount > ssStatus.dwWaitHint)
			{
				// WaitHint ʱ�䵽
				break;
			}
		}
	} 
	// �رվ��
	CloseServiceHandle(schService); 
	// �ж��Ƿ񴴽��ɹ���״̬��PENDING��ΪRUNNING��
	if (ssStatus.dwCurrentState == SERVICE_RUNNING) 
	{
		printf("StartService SUCCESS.\n"); 
		return 1;
	}
	else 
	{ 
		printf("\nService not started. \n");
		printf("  Current State: %d\n", ssStatus.dwCurrentState); 
		printf("  Exit Code: %d\n", ssStatus.dwWin32ExitCode); 
		printf("  Service Specific Exit Code: %d\n", 
			ssStatus.dwServiceSpecificExitCode); 
		printf("  Check Point: %d\n", ssStatus.dwCheckPoint); 
		printf("  Wait Hint: %d\n", ssStatus.dwWaitHint); 
		return 0;
	} 
}

/*************************************
* DWORD StopService( SC_HANDLE hSCM, 
LPTSTR szServiceName, 
BOOL fStopDependencies, 
DWORD dwTimeout )  
* ����	ֹͣ����
*
* ����	SC_HANDLE hSCM			SCM ���
*		LPTSTR szServiceName	������
*		BOOL fStopDependencies	�Ƿ���������ķ���
*		DWORD dwTimeout			��ʱ
**************************************/
DWORD StopService(SC_HANDLE hSCM, 
				  LPTSTR szServiceName, 
				  BOOL fStopDependencies, 
				  DWORD dwTimeout ) 
{
	SERVICE_STATUS_PROCESS ssp;
	SERVICE_STATUS ss;
	DWORD dwStartTime = GetTickCount();
	DWORD dwBytesNeeded;
	// �򿪷���
	SC_HANDLE hService = OpenService( 
		hSCM,          // SCM ��� 
		szServiceName,          // ������
		SERVICE_ALL_ACCESS); 

	// ��ѯ״̬��ȷ���Ƿ��Ѿ�ֹͣ
	if ( !QueryServiceStatusEx( 
		hService, 
		SC_STATUS_PROCESS_INFO,
		(LPBYTE)&ssp, 
		sizeof(SERVICE_STATUS_PROCESS),
		&dwBytesNeeded ) )
	{
		return GetLastError();
	}
	if ( ssp.dwCurrentState == SERVICE_STOPPED ) 
	{
		return ERROR_SUCCESS;
	}
	// ����� STOP_PENDING ״̬����ֻ��ȴ�
	while ( ssp.dwCurrentState == SERVICE_STOP_PENDING ) 
	{
		Sleep( ssp.dwWaitHint );
		// ѭ����ѯ��ֱ��״̬�ı�
		if ( !QueryServiceStatusEx( 
			hService, 
			SC_STATUS_PROCESS_INFO,
			(LPBYTE)&ssp, 
			sizeof(SERVICE_STATUS_PROCESS),
			&dwBytesNeeded ) )
		{
			return GetLastError();
		}
		if ( ssp.dwCurrentState == SERVICE_STOPPED )
		{
			return ERROR_SUCCESS;
		}
		if ( GetTickCount() - dwStartTime > dwTimeout )
		{
			return ERROR_TIMEOUT;
		}
	}

	// �Ƚ�����������
	if ( fStopDependencies ) 
	{
		DWORD i;
		DWORD dwBytesNeeded;
		DWORD dwCount;

		LPENUM_SERVICE_STATUS   lpDependencies = NULL;
		ENUM_SERVICE_STATUS     ess;
		SC_HANDLE               hDepService;

		// ʹ�� 0 ��С�� buf,��ȡbuf�Ĵ�С
		// ��� EnumDependentServices ֱ�ӷ��سɹ���˵��û����������
		if ( !EnumDependentServices( hService, SERVICE_ACTIVE, 
			lpDependencies, 0, &dwBytesNeeded, &dwCount ) ) 
		{
			if ( GetLastError() != ERROR_MORE_DATA )
				return GetLastError(); // Unexpected error

			// ���仺�����洢�������������
			lpDependencies = (LPENUM_SERVICE_STATUS) HeapAlloc( 
				GetProcessHeap(), HEAP_ZERO_MEMORY, dwBytesNeeded );

			if ( !lpDependencies )
				return GetLastError();

			__try {
				// �����������
				if ( !EnumDependentServices( hService, SERVICE_ACTIVE, 
					lpDependencies, dwBytesNeeded, &dwBytesNeeded,
					&dwCount ) )
					return GetLastError();

				for ( i = 0; i < dwCount; i++ ) 
				{
					ess = *(lpDependencies + i);

					// �򿪷���
					hDepService = OpenService( hSCM, ess.lpServiceName, 
						SERVICE_STOP | SERVICE_QUERY_STATUS );
					if ( !hDepService )
						return GetLastError();

					__try {
						// ��������
						if ( !ControlService( hDepService, 
							SERVICE_CONTROL_STOP,
							&ss ) )
							return GetLastError();

						// �ȴ��������
						while ( ss.dwCurrentState != SERVICE_STOPPED ) 
						{
							Sleep( ss.dwWaitHint );
							if ( !QueryServiceStatusEx( 
								hDepService, 
								SC_STATUS_PROCESS_INFO,
								(LPBYTE)&ssp, 
								sizeof(SERVICE_STATUS_PROCESS),
								&dwBytesNeeded ) )
								return GetLastError();

							if ( ss.dwCurrentState == SERVICE_STOPPED )
								break;

							if ( GetTickCount() - dwStartTime > dwTimeout )
								return ERROR_TIMEOUT;
						}

					} 
					__finally 
					{
						// �رշ���
						CloseServiceHandle( hDepService );

					}
				}
			} 
			__finally 
			{
				// �ͷ��ڴ�
				HeapFree( GetProcessHeap(), 0, lpDependencies );
			}
		} 
	}

	// ���е����������Ѿ�����������ָ������
	if ( !ControlService( hService, SERVICE_CONTROL_STOP, &ss ) )
		return GetLastError();
	while ( ss.dwCurrentState != SERVICE_STOPPED ) 
	{
		Sleep( ss.dwWaitHint );
		if ( !QueryServiceStatusEx( 
			hService, 
			SC_STATUS_PROCESS_INFO,
			(LPBYTE)&ssp, 
			sizeof(SERVICE_STATUS_PROCESS),
			&dwBytesNeeded ) )
			return GetLastError();

		if ( ss.dwCurrentState == SERVICE_STOPPED )
			break;

		if ( GetTickCount() - dwStartTime > dwTimeout )
			return ERROR_TIMEOUT;
	}
	return ERROR_SUCCESS;
}

/*************************************
* BOOL ControlSampleService(DWORD fdwControl) 
* ����	������Ϳ�����
*
* ����	DWORD fdwControl		������ֵ
*		SCM	�����������ֱ��ʹ��ȫ�ֱ���
**************************************/
BOOL ControlSampleService(DWORD fdwControl) 
{ 
	SERVICE_STATUS ssStatus; 
	DWORD fdwAccess; 
	DWORD dwStartTickCount, dwWaitTime;

	// Access
	switch (fdwControl) 
	{ 
	case SERVICE_CONTROL_STOP: 
		fdwAccess = SERVICE_STOP; 
		break; 
	case SERVICE_CONTROL_PAUSE: 
	case SERVICE_CONTROL_CONTINUE: 
		fdwAccess = SERVICE_PAUSE_CONTINUE; 
		break; 
	case SERVICE_CONTROL_INTERROGATE: 
		fdwAccess = SERVICE_INTERROGATE; 
		break; 
	default: 
		fdwAccess = SERVICE_INTERROGATE; 
	} 

	// �򿪷���
	schService = OpenService( 
		schSCManager,        // SCManager ��� 
		szServiceName,		 // ������
		fdwAccess);          // ��ȡȨ��
	if (schService == NULL) 
	{
		printf("OpenService failed (%d)\n", GetLastError()); 
		return FALSE;
	}

	// ���Ϳ�����
	if (! ControlService( 
		schService,   // ����ľ��
		fdwControl,   // ������
		&ssStatus) )  // ״̬
	{
		printf("ControlService failed (%d)\n", GetLastError()); 
		return FALSE;
	}

	// ��ʾ״̬
	printf("\nStatus of Sample_Srv: \n");
	printf("  Service Type: 0x%x\n", ssStatus.dwServiceType); 
	printf("  Current State: 0x%x\n", ssStatus.dwCurrentState); 
	printf("  Controls Accepted: 0x%x\n", 
		ssStatus.dwControlsAccepted); 
	printf("  Exit Code: %d\n", ssStatus.dwWin32ExitCode); 
	printf("  Service Specific Exit Code: %d\n", 
		ssStatus.dwServiceSpecificExitCode); 
	printf("  Check Point: %d\n", ssStatus.dwCheckPoint); 
	printf("  Wait Hint: %d\n", ssStatus.dwWaitHint); 

	return TRUE; 
}