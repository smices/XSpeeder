/***************************************************************************************
* AUTHOR : Missu.Link Team
* DATE   : 2013-12-11
* MODULE : ProcessProtect.C
* 
* Command: 
*	Source of IOCTRL Sample Driver
*
* Description:
*		Demonstrates communications between USER and KERNEL.
*
****************************************************************************************
* Copyright (C) 2010 Missu.Link Team.
****************************************************************************************/

//#######################################################################################
//# I N C L U D E S
//#######################################################################################

#ifndef CXX_PROCESSPROTECT_H
#include "ProcessProtect.h"
#endif

#define PROCESS_TERMINATE                  (0x0001)  
#define PROCESS_CREATE_THREAD              (0x0002)  
#define PROCESS_SET_SESSIONID              (0x0004)  
#define PROCESS_VM_OPERATION               (0x0008)  
#define PROCESS_VM_READ                    (0x0010)  
#define PROCESS_VM_WRITE                   (0x0020)  
#define PROCESS_DUP_HANDLE                 (0x0040)  
#define PROCESS_CREATE_PROCESS             (0x0080)  
#define PROCESS_SET_QUOTA                  (0x0100)  
#define PROCESS_SET_INFORMATION            (0x0200)  
#define PROCESS_QUERY_INFORMATION          (0x0400)  
#define PROCESS_SUSPEND_RESUME             (0x0800)  
#define PROCESS_QUERY_LIMITED_INFORMATION  (0x1000)  

static size_t s_cf_proc_name_offset = 0;

//#include "struct.h"

PVOID obHandle;//定义一个void*类型的变量，它将会作为ObRegisterCallbacks函数的第2个参数。

OB_PREOP_CALLBACK_STATUS 
preCall(PVOID RegistrationContext, POB_PRE_OPERATION_INFORMATION pOperationInformation)
{
	UNREFERENCED_PARAMETER(RegistrationContext);

	ANSI_STRING ansi_name;
	ULONG need_len;
	PEPROCESS curproc=(PEPROCESS)pOperationInformation->Object;
	UNICODE_STRING proc_name;
	WCHAR name_buf[32];
	RtlInitEmptyUnicodeString(&proc_name,name_buf,32*sizeof(WCHAR));
	// 这个名字是ansi字符串，现在转化为unicode字符串。
	RtlInitAnsiString(&ansi_name,((PCHAR)curproc + s_cf_proc_name_offset));
	need_len = RtlAnsiStringToUnicodeSize(&ansi_name);
	if(need_len > proc_name.MaximumLength)
	{
		return OB_PREOP_SUCCESS;
	}
	RtlAnsiStringToUnicodeString(&proc_name,&ansi_name,FALSE);

	UNICODE_STRING TempUS;
	RtlInitUnicodeString(&TempUS,L"xbSpeed.exe");
	if(RtlCompareUnicodeString(&TempUS,&proc_name,TRUE) == 0)	//忽略大小写
	{
		if (pOperationInformation->Operation == OB_OPERATION_HANDLE_CREATE)
		{
			if ((pOperationInformation->Parameters->CreateHandleInformation.OriginalDesiredAccess & PROCESS_TERMINATE) == PROCESS_TERMINATE)
			{
				pOperationInformation->Parameters->CreateHandleInformation.DesiredAccess &= ~PROCESS_TERMINATE;
			}
			if ((pOperationInformation->Parameters->CreateHandleInformation.OriginalDesiredAccess & PROCESS_VM_OPERATION) == PROCESS_VM_OPERATION)
			{
				pOperationInformation->Parameters->CreateHandleInformation.DesiredAccess &= ~PROCESS_VM_OPERATION;
			}
			if ((pOperationInformation->Parameters->CreateHandleInformation.OriginalDesiredAccess & PROCESS_VM_READ) == PROCESS_VM_READ)
			{
				pOperationInformation->Parameters->CreateHandleInformation.DesiredAccess &= ~PROCESS_VM_READ;
			}
			if ((pOperationInformation->Parameters->CreateHandleInformation.OriginalDesiredAccess & PROCESS_VM_WRITE) == PROCESS_VM_WRITE)
			{
				pOperationInformation->Parameters->CreateHandleInformation.DesiredAccess &= ~PROCESS_VM_WRITE;
			}
		}
	}
	return OB_PREOP_SUCCESS;
}


NTSTATUS ProtectProcess(BOOLEAN Enable)
{

	OB_CALLBACK_REGISTRATION obReg;
	OB_OPERATION_REGISTRATION opReg;

	memset(&obReg, 0, sizeof(obReg));
	obReg.Version = ObGetFilterVersion();
	obReg.OperationRegistrationCount = 1;
	obReg.RegistrationContext = NULL;
	RtlInitUnicodeString(&obReg.Altitude, L"321000");

	memset(&opReg, 0, sizeof(opReg)); //初始化结构体变量

	//下面 请注意这个结构体的成员字段的设置
	opReg.ObjectType = PsProcessType;
	opReg.Operations = OB_OPERATION_HANDLE_CREATE|OB_OPERATION_HANDLE_DUPLICATE; 

	opReg.PreOperation = (POB_PRE_OPERATION_CALLBACK)&preCall; //在这里注册一个回调函数指针

	obReg.OperationRegistration = &opReg; //注意这一条语句

	return ObRegisterCallbacks(&obReg, &obHandle); //在这里注册回调函数
}


typedef struct _LDR_DATA_TABLE_ENTRY  
{  
	LIST_ENTRY InLoadOrderLinks;  
	LIST_ENTRY InMemoryOrderLinks;  
	LIST_ENTRY InInitializationOrderLinks;  
	PVOID DllBase;  
	PVOID EntryPoint;  
	ULONG SizeOfImage;  
	UNICODE_STRING FullDllName;  
	UNICODE_STRING BaseDllName;  
	ULONG Flags;  
	USHORT LoadCount;  
	USHORT TlsIndex;  
	union {  
		LIST_ENTRY HashLinks;  
		struct  
		{  
			PVOID SectionPointer;  
			ULONG CheckSum;  
		};  
	};  
	union {  
		struct  
		{  
			ULONG TimeDateStamp;  
		};  
		struct  
		{  
			PVOID LoadedImports;  
		};  
	};  
	PVOID EntryPointActivationContext;  
	PVOID PatchInformation;  
}LDR_DATA_TABLE_ENTRY, *PLDR_DATA_TABLE_ENTRY;  
//////////////////////////////////////////////////////////////////////////


void cfCurProcNameInit()
{
	ULONG i;
	PEPROCESS  curproc;
	curproc = PsGetCurrentProcess();
	// 搜索EPROCESS结构，在其中找到字符串
	for(i=0;i<3*4*1024;i++)
	{
		if(!strncmp("System",(PCHAR)curproc+i,strlen("System"))) 
		{
			s_cf_proc_name_offset = i;
			break;
		}
	}
}

//#######################################################################################
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//@@@@@@@@				D R I V E R   E N T R Y   P O I N T						 @@@@@@@@
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//#######################################################################################
NTSTATUS
DriverEntry(IN PDRIVER_OBJECT pDriverObj, IN PUNICODE_STRING pRegistryString)
{
	NTSTATUS		status = STATUS_SUCCESS;
	UNICODE_STRING  ustrLinkName;
	UNICODE_STRING  ustrDevName;  
	PDEVICE_OBJECT  pDevObj;
	int i = 0;

    dprintf("EasySys Sample Driver\r\n"
            "Compiled %s %s\r\nIn DriverEntry : %wZ\r\n",
			__DATE__, __TIME__, pRegistryString);

	// Register dispatch routines
/*
	for(i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; i++)
	{
		pDriverObj->MajorFunction[i] = DispatchCommon;  
	}
*/
	pDriverObj->MajorFunction[IRP_MJ_CREATE] = DispatchCreate;
	pDriverObj->MajorFunction[IRP_MJ_CLOSE] = DispatchClose;

	// Dispatch routine for communications
	pDriverObj->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DispatchDeviceControl;

	// Unload routine
	pDriverObj->DriverUnload = DriverUnload;

	// Initialize the device name.
	RtlInitUnicodeString(&ustrDevName, NT_DEVICE_NAME);

	// Create the device object and device extension
	status = IoCreateDevice(pDriverObj, 
				0,
				&ustrDevName, 
				FILE_DEVICE_UNKNOWN,
				0,
				FALSE,
				&pDevObj);

	if(!NT_SUCCESS(status))
	{
		dprintf("Error, IoCreateDevice = 0x%x\r\n", status);
		return status;
	}

    //// Get a pointer to our device extension
    //deviceExtension = (PDEVICE_EXTENSION) deviceObject->DeviceExtension;

    //// Save a pointer to the device object
    //deviceExtension->DeviceObject = deviceObject;

	if(IoIsWdmVersionAvailable(1,0x10))
	{
		//如果是支持符号链接用户相关性的系统
		RtlInitUnicodeString(&ustrLinkName, SYMBOLIC_LINK_GLOBAL_NAME);
	}
	else
	{
		//不支持
		RtlInitUnicodeString(&ustrLinkName, SYMBOLIC_LINK_NAME);
	}
	
	// Create a symbolic link to allow USER applications to access it. 
	status = IoCreateSymbolicLink(&ustrLinkName, &ustrDevName);  
	
	if(!NT_SUCCESS(status))
	{
		dprintf("Error, IoCreateSymbolicLink = 0x%x\r\n", status);
		
		IoDeleteDevice(pDevObj); 
		return status;
	}	

	//
	//	TODO: Add initialization code here.
	//

    //// Tell the I/O Manger to do BUFFERED IO
    //deviceObject->Flags |= DO_BUFFERED_IO;

    //// Save the DeviveObject
    //deviceExtension->DeviceObject = deviceObject;

	KdPrint(("ProtectProcess\n"));

	cfCurProcNameInit();

	PLDR_DATA_TABLE_ENTRY ldr;

	ldr = (PLDR_DATA_TABLE_ENTRY)pDriverObj->DriverSection;
	ldr->Flags |= 0x20;

	status=ProtectProcess(TRUE);
	if(!NT_SUCCESS(status))
	{
		dprintf("Error, IoCreateDevice = 0x%x\r\n", status);
	}

	dprintf("DriverEntry Success\r\n");

	return STATUS_SUCCESS;
}

VOID
DriverUnload(IN PDRIVER_OBJECT pDriverObj)
{	
	UNICODE_STRING strLink;

	// Unloading - no resources to free so just return.
	dprintf("Unloading...\r\n");;	

	if(obHandle!=NULL)
		ObUnRegisterCallbacks(obHandle); //obHandle是上面定义的 PVOID obHandle;


	//
	// TODO: Add uninstall code here.
	//
	
	// Delete the symbolic link
	RtlInitUnicodeString(&strLink, SYMBOLIC_LINK_NAME);
	IoDeleteSymbolicLink(&strLink);

	// Delete the DeviceObject
	IoDeleteDevice(pDriverObj->DeviceObject);

	dprintf("Unloaded Success\r\n");

	return;
}

NTSTATUS
DispatchCreate(IN PDEVICE_OBJECT pDevObj, IN PIRP pIrp)
{
	pIrp->IoStatus.Status = STATUS_SUCCESS;
	pIrp->IoStatus.Information = 0;

	IoCompleteRequest(pIrp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;
}


NTSTATUS
DispatchClose(IN PDEVICE_OBJECT pDevObj, IN PIRP pIrp)
{
	pIrp->IoStatus.Status = STATUS_SUCCESS;
	pIrp->IoStatus.Information = 0;

	IoCompleteRequest(pIrp, IO_NO_INCREMENT);

	// Return success
	return STATUS_SUCCESS;
}

NTSTATUS
DispatchCommon(IN PDEVICE_OBJECT pDevObj, IN PIRP pIrp)
{
	pIrp->IoStatus.Status = STATUS_SUCCESS;
	pIrp->IoStatus.Information = 0L;

	IoCompleteRequest( pIrp, 0 );

	// Return success
	return STATUS_SUCCESS;
}

NTSTATUS 
DispatchDeviceControl(IN PDEVICE_OBJECT pDevObj, IN PIRP pIrp)
{
	NTSTATUS status               = STATUS_INVALID_DEVICE_REQUEST;	 // STATUS_UNSUCCESSFUL
	PIO_STACK_LOCATION pIrpStack  = IoGetCurrentIrpStackLocation(pIrp);
	ULONG uIoControlCode          = 0;
	PVOID pIoBuffer				  = NULL;
	ULONG uInSize                 = 0;
	ULONG uOutSize                = 0;

	// Get the IoCtrl Code
	uIoControlCode = pIrpStack->Parameters.DeviceIoControl.IoControlCode;

	pIoBuffer = pIrp->AssociatedIrp.SystemBuffer;
	uInSize = pIrpStack->Parameters.DeviceIoControl.InputBufferLength;
	uOutSize = pIrpStack->Parameters.DeviceIoControl.OutputBufferLength;

	switch(uIoControlCode)
	{
		case IOCTL_HELLO_WORLD:
			{			
				dprintf("MY_CTL_CODE(0)=%d\r\n,MY_CTL_CODE");

				// Return success
				status = STATUS_SUCCESS;
			}
			break;

		case IOCTRL_REC_FROM_APP:
			{
				// Receive data form Application
				//dprintf("IOCTRL_REC_FROM_APP\r\n");

				// Do we have any data?
				if( uInSize > 0 )
				{
					dprintf("Get Data from App: %ws\r\n", pIoBuffer);
				}

				// Return success
				status = STATUS_SUCCESS;
			}
			break;

		case IOCTRL_SEND_TO_APP:
			{
				// Send data to Application
				//dprintf("IOCTRL_SEND_TO_APP\r\n");
			
				// If we have enough room copy the data upto the App - note copy the terminating character as well...
				if( uOutSize >= strlen( DATA_TO_APP ) + 1 )
				{
					RtlCopyMemory(  pIoBuffer,
									DATA_TO_APP, 
									strlen( DATA_TO_APP ) + 1 );

					// Update the length for the App
					pIrp->IoStatus.Information = strlen( DATA_TO_APP ) + 1;

					dprintf("Send Data to App: %s\r\n", pIoBuffer);
					
					// Return success
					status = STATUS_SUCCESS;
				}
			}
			break;
			
		//
		// TODO: Add execute code here.
		//

		default:
			{
				// Invalid code sent
				dprintf("Unknown IOCTL: 0x%X (%04X,%04X)\r\n", 
                                          uIoControlCode,
                                          DEVICE_TYPE_FROM_CTL_CODE(uIoControlCode),
                                          IoGetFunctionCodeFromCtlCode(uIoControlCode));
				status = STATUS_INVALID_PARAMETER;	
			}
			break;
	}

	if(status == STATUS_SUCCESS)
	{
		pIrp->IoStatus.Information = uOutSize;
	}
	else
	{
		pIrp->IoStatus.Information = 0;
	}

	// Complete the I/O Request
	pIrp->IoStatus.Status = status;

	IoCompleteRequest(pIrp, IO_NO_INCREMENT);

	return status;
}


//
// TODO: Add your module definitions here.
//



/* EOF */
