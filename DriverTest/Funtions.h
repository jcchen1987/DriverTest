#ifndef _FUNCTIONS_H_
#define _FUNCTIONS_H_

#include<ntddk.h>
#include "DDKDef.h"


//�����豸 
NTSTATUS CreateDevice(IN PDRIVER_OBJECT pDriverObject); 

VOID IsSSDTHookedTest();
VOID FILETest();

//��ȡSSDT������ΪnIDx�ĺ�����ǰ��ַ
ULONG GetCurAddressWithIdx(IN int nIdx);

//��ȡSSDT������ΪnIDx�ĺ�����Դ��ַ
ULONG GetSrcAddressWithName(PUNICODE_STRING pFunctionName);

//ж������
VOID DDK_Unload (IN PDRIVER_OBJECT pDriverObject); 

//Control��ǲ����
NTSTATUS DDK_DispatchRoutine_CONTROL(IN PDEVICE_OBJECT pDevobj, IN PIRP pIrp);

//ͨ����ǲ����
NTSTATUS DDK_GENERAL_DispatchRoutine(IN PDEVICE_OBJECT pDevobj, IN PIRP pIrp);

#endif