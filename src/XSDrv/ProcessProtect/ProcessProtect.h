/***************************************************************************************
* AUTHOR : Missu.Link Team
* DATE   : 2013-12-11
* MODULE : ProcessProtect.H
*
* IOCTRL Sample Driver
*
* Description:
*		Demonstrates communications between USER and KERNEL.
*
****************************************************************************************
* Copyright (C) 2010 Missu.Link Team.
****************************************************************************************/

#ifndef CXX_PROCESSPROTECT_H
#define CXX_PROCESSPROTECT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <ntddk.h>
#include <devioctl.h>
#include "common.h"


//
// TODO: Add your include here
//


//////////////////////////////////////////////////////////////////////////

//
// TODO: Add your struct,enum(private) here
//



//////////////////////////////////////////////////////////////////////////
//***************************************************************************************
//* NAME:			DriverEntry
//*
//* DESCRIPTION:	Registers dispatch routines.
//*					
//*	PARAMETERS:		pDriverObj						IN		
//*						Address of the DRIVER_OBJECT created by NT for this driver.
//*					pRegistryString					IN		
//*						UNICODE_STRING which represents this drivers KEY in the Registry.  	
//*
//*	IRQL:			IRQL_PASSIVE_LEVEL.
//*
//*	RETURNS:		NTSTATUS
//***************************************************************************************
NTSTATUS DriverEntry(IN PDRIVER_OBJECT pDriverObj, IN PUNICODE_STRING pRegistryString);

//***************************************************************************************
//* NAME:			DriverUnload
//*
//* DESCRIPTION:	This routine is our dynamic unload entry point.
//*					
//*	PARAMETERS:		pDriverObj						IN		Address of our DRIVER_OBJECT.
//*
//*	IRQL:			IRQL_PASSIVE_LEVEL.
//*
//*	RETURNS:		None
//***************************************************************************************
VOID DriverUnload(IN PDRIVER_OBJECT pDriverObj);

//***************************************************************************************
//* NAME:			DispatchCreate, DispatchClose
//*
//* DESCRIPTION:	This two methods are the dispatch entry point for IRP_MJ_CREATE and IRP_MJ_CLOSE 
//*					routines.  This sample simply completes the requests with success.
//*					
//*	PARAMETERS:		pDevObj							IN		Address of our DRIVER_OBJECT.
//*					pIrp							IN		Address of the IRP.
//*
//*	IRQL:			IRQL_PASSIVE_LEVEL.
//*
//*	RETURNS:		STATUS_SUCCESS
//***************************************************************************************
NTSTATUS DispatchCreate(IN PDEVICE_OBJECT pDevObj, IN PIRP pIrp);
NTSTATUS DispatchClose(IN PDEVICE_OBJECT pDevObj, IN PIRP pIrp);

//***************************************************************************************
//* NAME:			DispatchDeviceControl
//*
//* DESCRIPTION:	This is the dispatch entry point for IRP_MJ_DEVICE_CONTROL.
//*					
//*	PARAMETERS:		pDevObj							IN		Address of our DRIVER_OBJECT.
//*					pIrp							IN		Address of the IRP.
//*
//*	IRQL:			IRQL_PASSIVE_LEVEL.
//*
//*	RETURNS:		NTSTATUS
//*
//*	NOTES:			IRP_MJ_DEVICE_CONTROL
//*					Parameters:
//*					Parameters.DeviceIoControl.OutputBufferLength	Length of OutBuffer 
//*					in bytes (length of buffer from GUI)
//*					Parameters.DeviceIoControl.InputBufferLength	Length of InBuffer 
//*					in bytes (length of buffer from DRIVER)
//*					Parameters.DeviceIoControl.ControlCode			I/O control code
//***************************************************************************************
NTSTATUS DispatchDeviceControl(IN PDEVICE_OBJECT pDevObj, IN PIRP pIrp);

// common dispatch
//***************************************************
//* #define IRP_MJ_CREATE                   0x00
//* #define IRP_MJ_CREATE_NAMED_PIPE        0x01
//* #define IRP_MJ_CLOSE                    0x02
//* #define IRP_MJ_READ                     0x03
//* #define IRP_MJ_WRITE                    0x04
//* #define IRP_MJ_QUERY_INFORMATION        0x05
//* #define IRP_MJ_SET_INFORMATION          0x06
//* #define IRP_MJ_QUERY_EA                 0x07
//* #define IRP_MJ_SET_EA                   0x08
//* #define IRP_MJ_FLUSH_BUFFERS            0x09
//* #define IRP_MJ_QUERY_VOLUME_INFORMATION 0x0a
//* #define IRP_MJ_SET_VOLUME_INFORMATION   0x0b
//* #define IRP_MJ_DIRECTORY_CONTROL        0x0c
//* #define IRP_MJ_FILE_SYSTEM_CONTROL      0x0d
//* #define IRP_MJ_DEVICE_CONTROL           0x0e
//* #define IRP_MJ_INTERNAL_DEVICE_CONTROL  0x0f
//* #define IRP_MJ_SHUTDOWN                 0x10
//* #define IRP_MJ_LOCK_CONTROL             0x11
//* #define IRP_MJ_CLEANUP                  0x12
//* #define IRP_MJ_CREATE_MAILSLOT          0x13
//* #define IRP_MJ_QUERY_SECURITY           0x14
//* #define IRP_MJ_SET_SECURITY             0x15
//* #define IRP_MJ_POWER                    0x16
//* #define IRP_MJ_SYSTEM_CONTROL           0x17
//* #define IRP_MJ_DEVICE_CHANGE            0x18
//* #define IRP_MJ_QUERY_QUOTA              0x19
//* #define IRP_MJ_SET_QUOTA                0x1a
//* #define IRP_MJ_PNP                      0x1b
//* #define IRP_MJ_PNP_POWER                IRP_MJ_PNP      // Obsolete....
//* #define IRP_MJ_MAXIMUM_FUNCTION         0x1b -->
//***************************************************************************************
NTSTATUS DispatchCommon (IN PDEVICE_OBJECT pDevObj, IN PIRP pIrp);

//////////////////////////////////////////////////////////////////////////

#ifdef ALLOC_PRAGMA
    // Allow the DriverEntry routine to be discarded once initialization is completed
#pragma alloc_text(INIT, DriverEntry)
    // 
#pragma alloc_text(PAGE, DriverUnload)
#pragma alloc_text(PAGE, DispatchCreate)
#pragma alloc_text(PAGE, DispatchClose)
#pragma alloc_text(PAGE, DispatchDeviceControl)
#pragma alloc_text(PAGE, DispatchCommon)
#endif // ALLOC_PRAGMA

//////////////////////////////////////////////////////////////////////////

//
// TODO: Add your module declarations here
//



#ifdef __cplusplus
}
#endif
//////////////////////////////////////////////////////////////////////////

#endif	//CXX_PROCESSPROTECT_H
/* EOF */
