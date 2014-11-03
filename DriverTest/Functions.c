#include "Funtions.h"
#include "CtlCodeDef.h"
#include "SSDTHookOpenProcess.h"

extern PServiceDescriptorTable KeServiceDescriptorTable;
extern TSSDTHook gtSSDTHookOpenProcess;
extern TSSDTInlineHook gtSSDTInlineHookOpenProcess;

#pragma PAGECODE 
static VOID DeleteDevice(IN PDRIVER_OBJECT pDriverObject);
static VOID HookSSDT(IN OUT TSSDTHook *ptSSDTHook);
static VOID UnHookSSDT(IN OUT TSSDTHook *ptSSDTHook);
static VOID HookInlineSSDT(IN OUT TSSDTInlineHook *ptSSDTInlineHook);
static VOID UnHookInlineSSDT(IN OUT TSSDTInlineHook *ptSSDTInlineHook);


NTSTATUS CreateDevice (IN PDRIVER_OBJECT pDriverObject) 
{
    NTSTATUS status;
    PDEVICE_OBJECT pDevObj;/*�������ش����豸*/

    //�����豸����
    UNICODE_STRING devName;
    UNICODE_STRING symLinkName; // 
    RtlInitUnicodeString(&devName,L"\\Device\\myDDK_Device");/*��devName��ʼ���ִ�Ϊ "\\Device\\yjxDDK_Device"*/

    //�����豸
    status = IoCreateDevice( pDriverObject,\
        0,\
        &devName,\
        FILE_DEVICE_UNKNOWN,\
        0, TRUE,\
        &pDevObj);
    if (!NT_SUCCESS(status))
    {
        if (status==STATUS_INSUFFICIENT_RESOURCES)
        {
            KdPrint(("��Դ���� STATUS_INSUFFICIENT_RESOURCES"));
        }
        if (status==STATUS_OBJECT_NAME_EXISTS )
        {
            KdPrint(("ָ������������"));
        }
        if (status==STATUS_OBJECT_NAME_COLLISION)
        {
            KdPrint(("//�������г�ͻ"));
        }
        KdPrint(("�豸����ʧ��...++++++++"));
        return status;
    }
    KdPrint(("�豸�����ɹ�...++++++++"));

    pDevObj->Flags |= DO_BUFFERED_IO;
    //������������

    RtlInitUnicodeString(&symLinkName, L"\\??\\my888");
    status = IoCreateSymbolicLink( &symLinkName,&devName);
    if (!NT_SUCCESS(status)) /*status����0*/
    {
        IoDeleteDevice( pDevObj );
        return status;
    }
    return STATUS_SUCCESS;
}

static VOID DeleteDevice(IN PDRIVER_OBJECT pDriverObject)
{
    PDEVICE_OBJECT pDevObj;
    UNICODE_STRING symLinkName;

    RtlInitUnicodeString(&symLinkName, L"\\??\\my888");
    IoDeleteSymbolicLink(&symLinkName);

    pDevObj = pDriverObject->DeviceObject;
    if (pDevObj)
    {
        IoDeleteDevice(pDevObj);
    }
    KdPrint(("�豸ɾ���ɹ�~~~~~~~~~~~~~"));
}

ULONG GetCurAddressWithIdx(IN int nIdx)
{
    LONG ServiceTableBaseAddress=(LONG)KeServiceDescriptorTable->ServiceTableBase;
    PLONG SSDTAddress = (PLONG)(ServiceTableBaseAddress + nIdx * 4);
    ULONG FunCurAddress=(ULONG)(*SSDTAddress);	
    return FunCurAddress;
}

ULONG GetSrcAddressWithName(PUNICODE_STRING pFunctionName)
{
    return (ULONG)MmGetSystemRoutineAddress(pFunctionName);
}

NTSTATUS DDK_DispatchRoutine_CONTROL(IN PDEVICE_OBJECT pDevobj, IN PIRP pIrp)
{
    ULONG info;
    int a,b,c;
    int * InputBuffer = NULL;
    int * OutputBuffer = NULL;
    //�õ���ǰջָ��
    PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(pIrp);
    ULONG mf=stack->MajorFunction;//����IRP
    //�õ����뻺������С
    ULONG cbin = stack->Parameters.DeviceIoControl.InputBufferLength;
    //�õ������������С
    ULONG cbout = stack->Parameters.DeviceIoControl.OutputBufferLength;
    //�õ�IOCTL��
    ULONG code = stack->Parameters.DeviceIoControl.IoControlCode;

    KdPrint(("Enter myDriver_DeviceIOControl\n"));
    switch (code)
    { 
    case ARITHMETIC_CODE_BUFFER:
            KdPrint(("ADDCODE_BUFFER\n"));
            //��������ʽIOCTL
            //��ȡ����������	a,b		
            InputBuffer = (int*)pIrp->AssociatedIrp.SystemBuffer;
            a = InputBuffer[0];
            b = InputBuffer[1];
            c = InputBuffer[2];
            KdPrint(("a=%d,b=%d \n", a,b));
            switch(c)
            {
            case 0:
                a += b;
                break;
            case 1:
                a -= b;
                break;
            case 2:
                a *= b;
                break;
            case 3:
                if(b == 0) 
                {
                    a = 0x7fffffff;
                }
                else
                {
                    a /= b;
                }                
            default:
                break;
            }
            //C�������㷵���������û���
            //�������������
            OutputBuffer = (int*)pIrp->AssociatedIrp.SystemBuffer;
            *OutputBuffer = a;

            //����ʵ�ʲ����������������
            info = 4;
            KdPrint(("a+b=%d \n",a));

            break;
    case SSDTHOOK_CODE_BUFFER:
        KdPrint(("SSDTHOOK_CODE_BUFFER\n"));
        //��������ʽIOCTL
        //��ȡ����������	a		
        InputBuffer = (int*)pIrp->AssociatedIrp.SystemBuffer;
        a = InputBuffer[0];
        KdPrint(("PID=%d\n", a));

        OutputBuffer = (int*)pIrp->AssociatedIrp.SystemBuffer;

        gtSSDTHookOpenProcess.nIndex = 122;
        gtSSDTHookOpenProcess.NewAddress = (ULONG)MyNtOpenProcessForSSDTHook;
        gtSSDTHookOpenProcess.nProtectPID = a;

        HookSSDT(&gtSSDTHookOpenProcess);

        //OutputBuffer = (int*)pIrp->AssociatedIrp.SystemBuffer;
        *OutputBuffer = 1;
        //����ʵ�ʲ����������������
        info = 4;
        break;
    case SSDTINLINEHOOK_CODE_BUFFER:
        KdPrint(("SSDTINLINEHOOK_CODE_BUFFER\n"));
        //��������ʽIOCTL
        //��ȡ����������	a		
        InputBuffer = (int*)pIrp->AssociatedIrp.SystemBuffer;
        a = InputBuffer[0];
        KdPrint(("PID=%d\n", a));

        OutputBuffer = (int*)pIrp->AssociatedIrp.SystemBuffer;
__asm int 3;
        gtSSDTInlineHookOpenProcess.nIndex = 122;
        gtSSDTInlineHookOpenProcess.nProtectPID = a;
        gtSSDTInlineHookOpenProcess.tJmpCode.DstAddress = (ULONG)MyNtOpenProcessForInlineHook;
        gtSSDTInlineHookOpenProcess.tJmpCode.OverWriteBytes = 10;

        HookInlineSSDT(&gtSSDTInlineHookOpenProcess);

        //OutputBuffer = (int*)pIrp->AssociatedIrp.SystemBuffer;
        *OutputBuffer = 1;
        //����ʵ�ʲ����������������
        info = 4;
        break;
    }

    //����Ӧ��IPR���д���
    pIrp->IoStatus.Information = info;//���ò������ֽ���Ϊ0��������ʵ������
    pIrp->IoStatus.Status = STATUS_SUCCESS;//���سɹ�
    IoCompleteRequest(pIrp,IO_NO_INCREMENT);//ָʾ��ɴ�IRP
    KdPrint(("�뿪��ǲ����\n"));//������Ϣ
    return STATUS_SUCCESS; //���سɹ�
}

NTSTATUS DDK_GENERAL_DispatchRoutine(IN PDEVICE_OBJECT pDevobj, IN PIRP pIrp)
{
    //����Ӧ��IPR���д���
    pIrp->IoStatus.Information = 0;//���ò������ֽ���Ϊ0��������ʵ������
    pIrp->IoStatus.Status = STATUS_SUCCESS;//���سɹ�
    IoCompleteRequest(pIrp,IO_NO_INCREMENT);//ָʾ��ɴ�IRP
    KdPrint(("�뿪��ǲ����\n"));//������Ϣ
    return STATUS_SUCCESS; //���سɹ�
}

VOID DDK_Unload (IN PDRIVER_OBJECT pDriverObject)
{
    UnHookSSDT(&gtSSDTHookOpenProcess);
    DeleteDevice(pDriverObject);

    KdPrint(("�����ɹ���ж��...OK-----------"));
}


//HOOK ��������
#pragma PAGECODE
static VOID HookSSDT(IN OUT TSSDTHook *ptSSDTHook) 
{ 
    LONG ServiceTableBaseAddress; 
    PLONG SSDTAddress;

    KdPrint(("++++HOOK START ++++-\n"));
    KdPrint(("�����ɹ���������.............................\n"));

    //�Ѿ���hook��������Ҫ�����ˣ�ֻҪ�ı�PID�Ϳ���
    if(ptSSDTHook->bHookFlag)
    {
        return;
    }
    
         
    //��ȡSSDT��������ֵΪnIndex�ĺ���
    ServiceTableBaseAddress = (LONG)KeServiceDescriptorTable->ServiceTableBase;
    SSDTAddress = (PLONG)(ServiceTableBaseAddress + ptSSDTHook->nIndex * 4);
    ptSSDTHook->OrgAddress =(ULONG)(*SSDTAddress);   

    KdPrint(( "��ʵ��NtOpenProcess��ַ: %x\n",(int) ptSSDTHook->OrgAddress )); 
    KdPrint((" α��NTOpenProcess��ַ: %x\n", (int)ptSSDTHook->NewAddress)); 
   

    __asm //ȥ��ҳ�汣��
    {
        cli;
        mov eax,cr0;
        and eax,not 10000h; //and eax,0FFFEFFFFh
        mov cr0,eax;

    }

    *SSDTAddress= ptSSDTHook->NewAddress; //SSDT HOOK

    __asm 
    { 
        mov eax, cr0; 
        or eax, 10000h;
        mov cr0, eax;
        sti;
    }   
    ptSSDTHook->bHookFlag = 1;//���ñ�HOOK��־
} 

//UnHook��������
////////////////////////////////////////////////////// 
#pragma PAGECODE
static VOID UnHookSSDT(IN OUT TSSDTHook *ptSSDTHook) 
{ 
    LONG ServiceTableBaseAddress; 
    PLONG SSDTAddress;

    if(ptSSDTHook->bHookFlag)
    {
        //��ȡSSDT��������ֵΪ0x7A�ĺ���
        ServiceTableBaseAddress = (LONG)KeServiceDescriptorTable->ServiceTableBase;
        SSDTAddress = (PLONG)(ServiceTableBaseAddress + ptSSDTHook->nIndex * 4);

        __asm //ȥ��ҳ�汣��
        {
            cli;
            mov eax,cr0;
            and eax,not 10000h; //and eax,0FFFEFFFFh
            mov cr0,eax;

        }

        // ��ԭSSDT 
        *SSDTAddress = (ULONG)ptSSDTHook->OrgAddress; 

        __asm 
        { 
            mov eax, cr0; 
            or eax, 10000h;
            mov cr0, eax;
            sti;
        } 
        KdPrint(("UnHookSSDT��ԭSSDT OK \n"));

        ptSSDTHook->bHookFlag = 0;
    }
}

//����openprocess�Ƿ�hook
VOID IsSSDTHookedTest()
{
    TMyFuncInfo tNtCreatKeyInfo;
    ULONG uCurAddress;
    ULONG uSrcAddress;

    tNtCreatKeyInfo.nIndex = 122;
    RtlInitUnicodeString(&tNtCreatKeyInfo.FunctionName, L"NtOpenProcess");

    uCurAddress = GetCurAddressWithIdx(tNtCreatKeyInfo.nIndex);
    uSrcAddress = GetSrcAddressWithName(&tNtCreatKeyInfo.FunctionName);

    KdPrint(("cur address: 0x%08x, src address: 0x%08x", uCurAddress, uSrcAddress));
    if (uCurAddress != uSrcAddress)
    {
        KdPrint(("������Hook����\n"));
    }
    else
    {
        KdPrint(("������û��Hook����\n"));
    }
}

VOID FILETest()
{    
    NTSTATUS ntStatus;
    HANDLE hfile;
    OBJECT_ATTRIBUTES objectAttributes;
    IO_STATUS_BLOCK iostatus;
    UNICODE_STRING SymbolFileName;
    FILE_STANDARD_INFORMATION fsi;
    FILE_BASIC_INFORMATION fbi;
    PUCHAR pBuf = NULL;
    LONG BufLenth = 100;
    LARGE_INTEGER largeInt;

    RtlInitUnicodeString(&SymbolFileName, L"\\??\\D:\\CreateFileTest.txt");
    //��ʼ��Ŀ������
    InitializeObjectAttributes(&objectAttributes, //ָ��һ����ҪOBJECT_ATTRIBUTES�ṹ��ַ
                               &SymbolFileName,//��һ��UNICODE_STRING�ִ���ַ��ָ����Ҫ����������������������Ƿ����������������豸��)
                               OBJ_CASE_INSENSITIVE, //ָ����ֵ��ʾ �����ִ�Сд��
                               NULL, 
                               NULL );
    //�����ļ�
    ntStatus = ZwCreateFile(&hfile,//PHANDLE����ָ�� ���ڷ��ش��ļ��ľ��
                            GENERIC_WRITE,//ACCESS_MASK���� ��ֵ�����������ļ�����������д������������
                            &objectAttributes,  //��ֵ��OBJECT_ATTRIBUTES�ṹ�ĵ�ַ���ýṹ����Ҫ�򿪵��ļ��� ��Ҫ��InitializeObjectAttributes���г�ʼ��
                            &iostatus,        //ָ��һ��IO_STATUS_BLOCK�ṹ������ֵ֮һ ���ڴ��ZwCreateFile�����Ľ��״̬
                            NULL,        //PLARGE_INTEGER ���ͣ�64λ����ָ�룩����ָ���ļ���ʼ����ʱ�Ĵ�С���ò������ΪNULL����ô�ļ����Ƚ���0��ʼ������д�������
                            FILE_ATTRIBUTE_NORMAL,        //�˲�����������ָ��Ϊ0����FILE_ATTRIBUTE_NORMAL������ļ����Ǳ������͸���д�� ��˲�����������
                            FILE_SHARE_READ,        //ָ������ģʽ �й����FILE_SHARE_READ,дFILE_SHARE_WRITE,ɾ��FILE_SHARE_DELETE�⼸��ģʽ
                            FILE_OPEN_IF,        //��ֵ��ʾ �ļ�������� ��������ʧ��
                            FILE_SYNCHRONOUS_IO_NONALERT,        //ָ���ļ��������ߴ򿪵ĸ��ӱ�־ FILE_SYNCHRONOUS_IO_NONALERT��ʾ���ļ��е����в�����ͬ������û�о���
                            NULL,         //�����豸���м��������򣬴˲���������NULL
                            0 );      //�����豸���м��������򣬴˲���������0
    if(NT_SUCCESS(ntStatus))
    {
        KdPrint(("�����ļ��ɹ�!\n"));
    }
    else
    {
        KdPrint(("�����ļ�ʧ��!\n"));
    }
    //���ļ�
    //ZwOpenFile()

    //д�ļ�
    pBuf = (PUCHAR)ExAllocatePool(PagedPool, BufLenth);
    RtlFillMemory(pBuf, BufLenth, 'a');
    RtlCopyMemory(pBuf, "i am vip_hehe", 20);

    largeInt.QuadPart = 0i64;
    ntStatus = ZwWriteFile(hfile, NULL, NULL, NULL, &iostatus, pBuf, BufLenth, &largeInt, NULL);
    KdPrint(("First:�ɹ�д�� %d �ֽ�\n",iostatus.Information));

    largeInt.QuadPart = 20i64;
    RtlFillMemory(pBuf, BufLenth, 't');
    ntStatus = ZwWriteFile(hfile, NULL, NULL, NULL, &iostatus, pBuf, BufLenth, &largeInt, NULL);
    KdPrint(("Second:�ɹ�д�� %d �ֽ�\n",iostatus.Information));

    //��ȡ/�޸�����
    ntStatus = ZwQueryInformationFile(hfile, &iostatus, &fsi, sizeof(FILE_STANDARD_INFORMATION), FileStandardInformation);
    if(NT_SUCCESS(ntStatus))
    {        
        KdPrint(("��ȡ��Ϣ�ɹ����ļ���С��%d!\n", fsi.EndOfFile.QuadPart));
    }
    else
    {
        KdPrint(("��ȡ��Ϣʧ��!\n"));
    }

    ntStatus = ZwQueryInformationFile(hfile, &iostatus, &fbi, sizeof(FILE_BASIC_INFORMATION), FileBasicInformation);
    fbi.FileAttributes = FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_HIDDEN;
    ZwSetInformationFile(hfile, &iostatus, &fbi, sizeof(FILE_BASIC_INFORMATION), FileBasicInformation);

    //���ļ�
    largeInt.QuadPart = 0i64;
    ZwReadFile(hfile, NULL, NULL, NULL, &iostatus, pBuf, BufLenth, NULL, NULL);
    KdPrint(("Read:�ɹ���ȡ %d �ֽ�\n",iostatus.Information));
    KdPrint(("read text:\n%s\n", pBuf));

    //�ر��ļ�
    ZwClose(hfile);

    ExFreePool(pBuf);
}

static VOID HookInlineSSDT(IN OUT TSSDTInlineHook *ptSSDTInlineHook)
{
    LONG ServiceTableBaseAddress; 
    PLONG SSDTAddress;
    PLONG WriteAddress;
    CHAR JmpDat[10] = {0};

    KdPrint(("++++Inline HOOK START ++++-\n"));
    KdPrint(("�����ɹ���������.............................\n"));

    //�Ѿ���hook��������Ҫ�����ˣ�ֻҪ�ı�PID�Ϳ���
    if(ptSSDTInlineHook->bHookFlag)
    {
        return;
    }


    //��ȡSSDT��������ֵΪnIndex�ĺ���
    ServiceTableBaseAddress = (LONG)KeServiceDescriptorTable->ServiceTableBase;
    SSDTAddress = (PLONG)(ServiceTableBaseAddress + ptSSDTInlineHook->nIndex * 4);
    WriteAddress =(PLONG)(*SSDTAddress);   
    gtSSDTInlineHookOpenProcess.tJmpCode.WriteAddress = (ULONG)(*SSDTAddress);

    

    KdPrint(( "��ʵ��NtOpenProcess��ַ: %x\n",(int) gtSSDTInlineHookOpenProcess.tJmpCode.WriteAddress )); 
    //KdPrint((" α��NTOpenProcess��ַ: %x\n", (int)ptSSDTHook->NewAddress)); 


    __asm //ȥ��ҳ�汣��
    {
        cli;
        mov eax,cr0;
        and eax,not 10000h; //and eax,0FFFEFFFFh
        mov cr0,eax;
    }
    //����ԭ������
    memcpy(gtSSDTInlineHookOpenProcess.tJmpCode.OrgDat, WriteAddress, gtSSDTInlineHookOpenProcess.tJmpCode.OverWriteBytes);
    //������ת�ṹ
    JmpDat[0] = 0xB8;
    memcpy(JmpDat + 1, &gtSSDTInlineHookOpenProcess.tJmpCode.DstAddress, sizeof(LONG));
    JmpDat[5] = 0xFF;
    JmpDat[6] = 0xE0;
    JmpDat[7] = JmpDat[8] = JmpDat[9] = 0x90; 

    //д��
    memcpy(WriteAddress, JmpDat, 10);

    //*SSDTAddress= ptSSDTInlineHook->NewAddress; //SSDT HOOK

    __asm 
    { 
        mov eax, cr0; 
        or eax, 10000h;
        mov cr0, eax;
        sti;
    }   
    ptSSDTInlineHook->bHookFlag = 1;//���ñ�HOOK��־
}
static VOID UnHookInlineSSDT(IN OUT TSSDTInlineHook *ptSSDTInlineHook)
{
    LONG ServiceTableBaseAddress; 
    PLONG SSDTAddress;

    if(ptSSDTInlineHook->bHookFlag)
    {
        __asm //ȥ��ҳ�汣��
        {
            cli;
            mov eax,cr0;
            and eax,not 10000h; //and eax,0FFFEFFFFh
            mov cr0,eax;

        }

        // ��ԭSSDT 
        memcpy(ptSSDTInlineHook->tJmpCode.WriteAddress, ptSSDTInlineHook->tJmpCode.OrgDat, ptSSDTInlineHook->tJmpCode.OverWriteBytes);

        __asm 
        { 
            mov eax, cr0; 
            or eax, 10000h;
            mov cr0, eax;
            sti;
        } 
        KdPrint(("UnHookInlineSSDT��ԭSSDT OK \n"));

        ptSSDTInlineHook->bHookFlag = 0;
    }

}