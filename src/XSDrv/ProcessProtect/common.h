/**************************************************************************************
* AUTHOR : Missu.Link Team
* DATE   : 2016-05-07
* MODULE : common.h
*
* Command: 
*	IOCTRL Common Header
*
* Description:
*	Common data for the IoCtrl driver and application
*
****************************************************************************************
* Copyright (C) 2020 Missu.Link Team.
****************************************************************************************/

#pragma once 

//#######################################################################################
// D E F I N E S
//#######################################################################################

#if DBG
#define dprintf DbgPrint
#else
#define dprintf
#endif

//��֧�ַ��������û������
#define NT_DEVICE_NAME               L"\\Device\\devProcessProtect"             // Driver Name
#define SYMBOLIC_LINK_NAME           L"\\DosDevices\\ProcessProtect"            // Symbolic Link Name
#define WIN32_LINK_NAME              "\\\\.\\ProcessProtect"                    // Win32 Link Name

//֧�ַ��������û������
#define SYMBOLIC_LINK_GLOBAL_NAME    L"\\DosDevices\\Global\\ProcessProtect"    // Symbolic Link Name

#define DATA_TO_APP                  "This string from driver to app"

//
// Device IO Control Codes
//
#define IOCTL_BASE                   0x800
#define MY_CTL_CODE(i)               CTL_CODE( FILE_DEVICE_UNKNOWN, IOCTL_BASE + i, METHOD_BUFFERED, FILE_ANY_ACCESS )

#define IOCTL_HELLO_WORLD            MY_CTL_CODE(0)
#define IOCTRL_REC_FROM_APP          MY_CTL_CODE(1)
#define IOCTRL_SEND_TO_APP           MY_CTL_CODE(2)

//
// TODO: Add your IOCTL define here
//


//
// TODO: Add your struct,enum(public) define here
//


/* EOF */

