#include<ntddk.h>
#include "Funtions.h"
#include "DDKDef.h"

#pragma INITCODE 
NTSTATUS DriverEntry(PDRIVER_OBJECT lpDriverObject, PUNICODE_STRING pUniStr)
{
    KdPrint(("�����ɹ�������...OK++++++++"));

    //�����豸
    CreateDevice(lpDriverObject);

    //����
    //IsSSDTHookedTest();
    //FILETest();

    //ע����ǲ����
    lpDriverObject->MajorFunction[IRP_MJ_CREATE] = DDK_GENERAL_DispatchRoutine;
    lpDriverObject->MajorFunction[IRP_MJ_CLOSE] = DDK_GENERAL_DispatchRoutine;
    lpDriverObject->MajorFunction[IRP_MJ_READ] = DDK_GENERAL_DispatchRoutine;
    lpDriverObject->MajorFunction[IRP_MJ_WRITE] = DDK_GENERAL_DispatchRoutine;
    lpDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DDK_DispatchRoutine_CONTROL;
    lpDriverObject->DriverUnload = DDK_Unload;
    return 1;
}
