/* ************************************
*����ͨWindows API��
* ʾ������
* xioctl.c
* 16.2  IO���ơ��ں�ͨ��
**************************************/
/* ͷ�ļ� */
#include <ntddk.h>// �����˺ܶ�NT�ں˵����͡��ṹ���������壬��������ʱ��Ҫ������ͷ�ļ�
#include <string.h>
#include "xioctl.h"
/* ������Ԥ���� */
#define NT_DEVICE_NAME      L"\\Device\\XIOCTL"
#define DOS_DEVICE_NAME     L"\\DosDevices\\IoctlTest"

#if DBG
#define XIOCTL_KDPRINT(_x_) \
                DbgPrint("XIOCTL.SYS: ");\
                DbgPrint _x_;
#else
#define XIOCTL_KDPRINT(_x_)
#endif 

/* �������� */
NTSTATUS DriverEntry(PDRIVER_OBJECT  DriverObject, PUNICODE_STRING RegistryPath);
NTSTATUS XioctlCreateClose(PDEVICE_OBJECT DeviceObject, PIRP Irp);
NTSTATUS XioctlDeviceControl(PDEVICE_OBJECT DeviceObject, PIRP Irp );
VOID XioctlUnloadDriver(PDRIVER_OBJECT DriverObject );
VOID PrintIrpInfo( PIRP Irp );
VOID PrintChars( PCHAR BufferAddress, ULONG CountChars );

#ifdef ALLOC_PRAGMA
#pragma alloc_text( INIT, DriverEntry )
#pragma alloc_text( PAGE, XioctlCreateClose)
#pragma alloc_text( PAGE, XioctlDeviceControl)
#pragma alloc_text( PAGE, XioctlUnloadDriver)
#pragma alloc_text( PAGE, PrintIrpInfo)
#pragma alloc_text( PAGE, PrintChars)
#endif // ALLOC_PRAGMA


/*************************************
* DriverEntry
* ����	��������ں�������������ش�������
**************************************/
NTSTATUS
DriverEntry(
    IN OUT PDRIVER_OBJECT   DriverObject,
    IN PUNICODE_STRING      RegistryPath
    )
{
    NTSTATUS        ntStatus;
    UNICODE_STRING  ntUnicodeString;    // �豸��
    UNICODE_STRING  ntWin32NameString;    // Win32 �豸��
    PDEVICE_OBJECT  deviceObject = NULL;    // �豸����


    RtlInitUnicodeString( &ntUnicodeString, NT_DEVICE_NAME );
    // �����豸
    ntStatus = IoCreateDevice(
        DriverObject,                   // �������� DriverEntry �Ĳ���
        0,                              // ��ʹ���豸��չ
        &ntUnicodeString,               // �豸�� "\Device\XIOCTL"
        FILE_DEVICE_UNKNOWN,            // �豸����
        FILE_DEVICE_SECURE_OPEN,   		  // 
        FALSE,                          // 
        &deviceObject );                // �豸����

    if ( !NT_SUCCESS( ntStatus ) )
    {
        XIOCTL_KDPRINT(("Couldn't create the device object\n"));
        return ntStatus;
    }
    // ��ʼ����������
    DriverObject->MajorFunction[IRP_MJ_CREATE] = XioctlCreateClose;// ����ʱ�����
    DriverObject->MajorFunction[IRP_MJ_CLOSE] = XioctlCreateClose;// �ر�ʱ�����
    // ����IO����
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = XioctlDeviceControl;
    DriverObject->DriverUnload = XioctlUnloadDriver;// ж��ʱ�����
   
    // WIN32 �豸��
    RtlInitUnicodeString( &ntWin32NameString, DOS_DEVICE_NAME );

    // ���豸����WIN32�豸��֮�䴴����������
    ntStatus = IoCreateSymbolicLink(
                        &ntWin32NameString, &ntUnicodeString );

    if ( !NT_SUCCESS( ntStatus ) )
    {
        XIOCTL_KDPRINT(("Couldn't create symbolic link\n"));
        IoDeleteDevice( deviceObject );
    }

    return ntStatus;
}

/*************************************
* XioctlCreateClose
* ����	��������Ĵ������̣���DriverEntryָ��
*				�����������������ڴ����͹ر�ʱ���õ����̡�
*				û��ʵ�ʵĹ��ܣ�ֻ�ǽ�״̬����Ϊ �ɹ�
**************************************/
NTSTATUS
XioctlCreateClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

{
    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;
    
    IoCompleteRequest( Irp, IO_NO_INCREMENT );
    
    return STATUS_SUCCESS;
}

/*************************************
* XioctlUnloadDriver
* ����	ж������ʱ���õ����̣�
*       ɾ���������ӣ�ɾ���豸
**************************************/
VOID
XioctlUnloadDriver(
    IN PDRIVER_OBJECT DriverObject
    )

{
    PDEVICE_OBJECT deviceObject = DriverObject->DeviceObject;
    UNICODE_STRING uniWin32NameString;

    RtlInitUnicodeString( &uniWin32NameString, DOS_DEVICE_NAME );


    // ɾ����������    
    IoDeleteSymbolicLink( &uniWin32NameString );
    // ɾ���豸
    if ( deviceObject != NULL )
    {
        IoDeleteDevice( deviceObject );
    }
}

/*************************************
* XioctlDeviceControl
* ����	����IO���Ƶ�����
**************************************/
NTSTATUS
XioctlDeviceControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    PIO_STACK_LOCATION  irpSp;// ��ǰջ��λ��
    NTSTATUS            ntStatus = STATUS_SUCCESS;// ִ��״̬���ɹ�\ʧ��
    ULONG               inBufLength; // ���뻺���С
    ULONG               outBufLength; // ��������С
    PCHAR               inBuf, outBuf; // �����������
    PCHAR               data = "This String is from Device Driver !!!";
    ULONG               datalen = strlen(data)+1;//������ݵĳ���
    PMDL                mdl = NULL;
    PCHAR               buffer = NULL;  

		// ����IRP
    irpSp = IoGetCurrentIrpStackLocation( Irp );
    inBufLength = irpSp->Parameters.DeviceIoControl.InputBufferLength;
    outBufLength = irpSp->Parameters.DeviceIoControl.OutputBufferLength;

    if(!inBufLength || !outBufLength)
    {
        ntStatus = STATUS_INVALID_PARAMETER;
        goto End;
    }

    // �ж�IOCTL
    switch ( irpSp->Parameters.DeviceIoControl.IoControlCode )
    {
    case IOCTL_XIOCTL_BUFFER: 
    		// ��ʾ�յ���IRP
        XIOCTL_KDPRINT(("Called IOCTL_SIOCTL_METHOD_BUFFERED\n")); 
        PrintIrpInfo(Irp);

        // �豸IN OUT ����
        inBuf = Irp->AssociatedIrp.SystemBuffer;
        outBuf = Irp->AssociatedIrp.SystemBuffer;

        // �����뻺���л����Ϣ
        XIOCTL_KDPRINT(("\tData from User : %s", inBuf));
        // �������ݵ��������
        strncpy(outBuf, data, outBufLength);
				// ��ӡ���������������
        XIOCTL_KDPRINT(("\tData to User : %s", outBuf));
        // ����IRP 
        Irp->IoStatus.Information = (outBufLength<datalen?outBufLength:datalen);

       break;
		// �����Զ�������IO ������
		
    default:

        // ��������ʾ֪��IO code
        ntStatus = STATUS_INVALID_DEVICE_REQUEST;
        XIOCTL_KDPRINT(("ERROR: unrecognized IOCTL %x\n",
            irpSp->Parameters.DeviceIoControl.IoControlCode));
        break;
    }

End:
    // �豸״̬�����IPR����
    Irp->IoStatus.Status = ntStatus;
    IoCompleteRequest( Irp, IO_NO_INCREMENT );
    return ntStatus;
}

/*************************************
* PrintIrpInfo
* ����	��ӡIPR��Ϣ
**************************************/
VOID
PrintIrpInfo(
    PIRP Irp)
{
    PIO_STACK_LOCATION  irpSp;
    irpSp = IoGetCurrentIrpStackLocation( Irp );

    XIOCTL_KDPRINT(("\tIrp->AssociatedIrp.SystemBuffer = 0x%p\n",
        Irp->AssociatedIrp.SystemBuffer)); 
    XIOCTL_KDPRINT(("\tIrp->UserBuffer = 0x%p\n", Irp->UserBuffer)); 
    XIOCTL_KDPRINT(("\tirpSp->Parameters.DeviceIoControl.Type3InputBuffer = 0x%p\n",
        irpSp->Parameters.DeviceIoControl.Type3InputBuffer)); 
    XIOCTL_KDPRINT(("\tirpSp->Parameters.DeviceIoControl.InputBufferLength = %d\n",
        irpSp->Parameters.DeviceIoControl.InputBufferLength)); 
    XIOCTL_KDPRINT(("\tirpSp->Parameters.DeviceIoControl.OutputBufferLength = %d\n",
        irpSp->Parameters.DeviceIoControl.OutputBufferLength )); 
    return;
}