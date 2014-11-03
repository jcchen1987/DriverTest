#ifndef _FUNCTIONS_H_
#define _FUNCTIONS_H_

#include<ntddk.h>
#include "DDKDef.h"


//创建设备 
NTSTATUS CreateDevice(IN PDRIVER_OBJECT pDriverObject); 

VOID IsSSDTHookedTest();
VOID FILETest();

//获取SSDT中索引为nIDx的函数当前地址
ULONG GetCurAddressWithIdx(IN int nIdx);

//获取SSDT中索引为nIDx的函数起源地址
ULONG GetSrcAddressWithName(PUNICODE_STRING pFunctionName);

//卸载例程
VOID DDK_Unload (IN PDRIVER_OBJECT pDriverObject); 

//Control派遣函数
NTSTATUS DDK_DispatchRoutine_CONTROL(IN PDEVICE_OBJECT pDevobj, IN PIRP pIrp);

//通用派遣函数
NTSTATUS DDK_GENERAL_DispatchRoutine(IN PDEVICE_OBJECT pDevobj, IN PIRP pIrp);

#endif