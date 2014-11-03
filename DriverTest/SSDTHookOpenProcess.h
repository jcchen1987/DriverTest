#ifndef __SSDT_HOOK_OPEN_PROCESS_H__
#define __SSDT_HOOK_OPEN_PROCESS_H__

#include <ntddk.h>
#include "DDKDef.h"

extern PServiceDescriptorTable KeServiceDescriptorTable;

TSSDTHook gtSSDTHookOpenProcess = {0};
TSSDTInlineHook gtSSDTInlineHookOpenProcess = {0};

// ����һ��NtOpenProcess��ԭ��
typedef NTSTATUS __stdcall NTOPENPROCESS(OUT PHANDLE ProcessHandle,
                                                    IN ACCESS_MASK AccessMask, 
                                                    IN POBJECT_ATTRIBUTES ObjectAttributes,
                                                    IN PCLIENT_ID ClientId);

// �Զ����NtOpenProcess���� ZwOpenProcess
#pragma PAGECODE
NTSTATUS __stdcall MyNtOpenProcessForSSDTHook(OUT PHANDLE ProcessHandle, 
                                              IN ACCESS_MASK DesiredAccess, 
                                              IN POBJECT_ATTRIBUTES ObjectAttributes, 
                                              IN PCLIENT_ID ClientId) 
{ 
    NTSTATUS     rc; 
    HANDLE       PID;
    PEPROCESS EP; 

    NTOPENPROCESS *RealNtOpenProcess = (NTOPENPROCESS *)gtSSDTHookOpenProcess.OrgAddress;
    KdPrint(("++++++++++++Entry MyNtOpenProcessForSSDTHook int   ++++++++++++++\n"));  

    rc = (NTSTATUS)RealNtOpenProcess( ProcessHandle, DesiredAccess, ObjectAttributes, ClientId ); 	

    if( (ClientId != NULL) ) 
    { 
        PID = ClientId->UniqueProcess; 	 
        KdPrint(( "------------------------- PID=%d--------------\n",(int*)PID ));

        // ����Ǳ�������PID����ܾ����ʣ������������Ϊ�� 
        if((int)PID == gtSSDTHookOpenProcess.nProtectPID) 
        { 
            KdPrint(("���������� MyPID=%d \n",(int)gtSSDTHookOpenProcess.nProtectPID));
            //������� ����C���Ե� Printf
            ProcessHandle = NULL; //����ǹؼ�
            rc = STATUS_ACCESS_DENIED; //�������ֵ 
            //PsLookupProcessByProcessId((ULONG)PID,&EP);
            EP=PsGetCurrentProcess();			 
            KdPrint((" ACESS Process Name  --:%s--   \n",(PTSTR)((ULONG)EP+0x174)));
        } 
    } 

    return rc; 
} 


#pragma PAGECODE
NTSTATUS __stdcall MyNtOpenProcessForInlineHook(OUT PHANDLE ProcessHandle, 
                                              IN ACCESS_MASK DesiredAccess, 
                                              IN POBJECT_ATTRIBUTES ObjectAttributes, 
                                              IN PCLIENT_ID ClientId) 
{ 
    NTSTATUS     rc; 
    HANDLE       PID;
    PEPROCESS EP; 
    LONG WriteAddr;
    LONG OverwriteBytes;

//     NTOPENPROCESS *WriteAddr = (NTOPENPROCESS *)gtSSDTHookOpenProcess.OrgAddress;
//     KdPrint(("++++++++++++Entry MyNtOpenProcessForSSDTHook int   ++++++++++++++\n"));  
// 
//     rc = (NTSTATUS)WriteAddr( ProcessHandle, DesiredAccess, ObjectAttributes, ClientId ); 	

    _asm int 3;
    if( (ClientId != NULL) ) 
    { 
        PID = ClientId->UniqueProcess; 	 
        KdPrint(( "------------------------- PID=%d--------------\n",(int*)PID ));

        // ����Ǳ�������PID����ܾ����ʣ������������Ϊ�� 
        if((int)PID == gtSSDTInlineHookOpenProcess.nProtectPID) 
        { 
            KdPrint(("���������� MyPID=%d \n",(int)gtSSDTInlineHookOpenProcess.nProtectPID));
            //������� ����C���Ե� Printf
            ProcessHandle = NULL; //����ǹؼ�
            rc = STATUS_ACCESS_DENIED; //�������ֵ 
            //PsLookupProcessByProcessId((ULONG)PID,&EP);
            EP=PsGetCurrentProcess();			 
            KdPrint((" ACESS Process Name  --:%s--   \n",(PTSTR)((ULONG)EP+0x174)));

            //ֱ�ӷ���
            __asm mov eax, rc;
            __asm retn 0x10;
        } 
    }

    //������Ҫ�����Ľ���
    WriteAddr = gtSSDTInlineHookOpenProcess.tJmpCode.WriteAddress;
    OverwriteBytes = gtSSDTInlineHookOpenProcess.tJmpCode.OverWriteBytes;
    WriteAddr += OverwriteBytes;

    KdPrint(("jmp addr:0x%09x", WriteAddr));
    _asm
    {
        push 0xC4;
        push 0x804EB0E8;
        mov eax, WriteAddr;
        jmp eax;
    }

    return rc; 
} 

#endif
