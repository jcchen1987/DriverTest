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
    PDEVICE_OBJECT pDevObj;/*用来返回创建设备*/

    //创建设备名称
    UNICODE_STRING devName;
    UNICODE_STRING symLinkName; // 
    RtlInitUnicodeString(&devName,L"\\Device\\myDDK_Device");/*对devName初始化字串为 "\\Device\\yjxDDK_Device"*/

    //创建设备
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
            KdPrint(("资源不足 STATUS_INSUFFICIENT_RESOURCES"));
        }
        if (status==STATUS_OBJECT_NAME_EXISTS )
        {
            KdPrint(("指定对象名存在"));
        }
        if (status==STATUS_OBJECT_NAME_COLLISION)
        {
            KdPrint(("//对象名有冲突"));
        }
        KdPrint(("设备创建失败...++++++++"));
        return status;
    }
    KdPrint(("设备创建成功...++++++++"));

    pDevObj->Flags |= DO_BUFFERED_IO;
    //创建符号链接

    RtlInitUnicodeString(&symLinkName, L"\\??\\my888");
    status = IoCreateSymbolicLink( &symLinkName,&devName);
    if (!NT_SUCCESS(status)) /*status等于0*/
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
    KdPrint(("设备删除成功~~~~~~~~~~~~~"));
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
    //得到当前栈指针
    PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(pIrp);
    ULONG mf=stack->MajorFunction;//区分IRP
    //得到输入缓冲区大小
    ULONG cbin = stack->Parameters.DeviceIoControl.InputBufferLength;
    //得到输出缓冲区大小
    ULONG cbout = stack->Parameters.DeviceIoControl.OutputBufferLength;
    //得到IOCTL码
    ULONG code = stack->Parameters.DeviceIoControl.IoControlCode;

    KdPrint(("Enter myDriver_DeviceIOControl\n"));
    switch (code)
    { 
    case ARITHMETIC_CODE_BUFFER:
            KdPrint(("ADDCODE_BUFFER\n"));
            //缓冲区方式IOCTL
            //获取缓冲区数据	a,b		
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
            //C、驱动层返回数据至用户层
            //操作输出缓冲区
            OutputBuffer = (int*)pIrp->AssociatedIrp.SystemBuffer;
            *OutputBuffer = a;

            //设置实际操作输出缓冲区长度
            info = 4;
            KdPrint(("a+b=%d \n",a));

            break;
    case SSDTHOOK_CODE_BUFFER:
        KdPrint(("SSDTHOOK_CODE_BUFFER\n"));
        //缓冲区方式IOCTL
        //获取缓冲区数据	a		
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
        //设置实际操作输出缓冲区长度
        info = 4;
        break;
    case SSDTINLINEHOOK_CODE_BUFFER:
        KdPrint(("SSDTINLINEHOOK_CODE_BUFFER\n"));
        //缓冲区方式IOCTL
        //获取缓冲区数据	a		
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
        //设置实际操作输出缓冲区长度
        info = 4;
        break;
    }

    //对相应的IPR进行处理
    pIrp->IoStatus.Information = info;//设置操作的字节数为0，这里无实际意义
    pIrp->IoStatus.Status = STATUS_SUCCESS;//返回成功
    IoCompleteRequest(pIrp,IO_NO_INCREMENT);//指示完成此IRP
    KdPrint(("离开派遣函数\n"));//调试信息
    return STATUS_SUCCESS; //返回成功
}

NTSTATUS DDK_GENERAL_DispatchRoutine(IN PDEVICE_OBJECT pDevobj, IN PIRP pIrp)
{
    //对相应的IPR进行处理
    pIrp->IoStatus.Information = 0;//设置操作的字节数为0，这里无实际意义
    pIrp->IoStatus.Status = STATUS_SUCCESS;//返回成功
    IoCompleteRequest(pIrp,IO_NO_INCREMENT);//指示完成此IRP
    KdPrint(("离开派遣函数\n"));//调试信息
    return STATUS_SUCCESS; //返回成功
}

VOID DDK_Unload (IN PDRIVER_OBJECT pDriverObject)
{
    UnHookSSDT(&gtSSDTHookOpenProcess);
    DeleteDevice(pDriverObject);

    KdPrint(("驱动成功被卸载...OK-----------"));
}


//HOOK 函数构建
#pragma PAGECODE
static VOID HookSSDT(IN OUT TSSDTHook *ptSSDTHook) 
{ 
    LONG ServiceTableBaseAddress; 
    PLONG SSDTAddress;

    KdPrint(("++++HOOK START ++++-\n"));
    KdPrint(("驱动成功被加载中.............................\n"));

    //已经被hook过，不需要处理了，只要改变PID就可以
    if(ptSSDTHook->bHookFlag)
    {
        return;
    }
    
         
    //读取SSDT表中索引值为nIndex的函数
    ServiceTableBaseAddress = (LONG)KeServiceDescriptorTable->ServiceTableBase;
    SSDTAddress = (PLONG)(ServiceTableBaseAddress + ptSSDTHook->nIndex * 4);
    ptSSDTHook->OrgAddress =(ULONG)(*SSDTAddress);   

    KdPrint(( "真实的NtOpenProcess地址: %x\n",(int) ptSSDTHook->OrgAddress )); 
    KdPrint((" 伪造NTOpenProcess地址: %x\n", (int)ptSSDTHook->NewAddress)); 
   

    __asm //去掉页面保护
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
    ptSSDTHook->bHookFlag = 1;//设置被HOOK标志
} 

//UnHook函数构建
////////////////////////////////////////////////////// 
#pragma PAGECODE
static VOID UnHookSSDT(IN OUT TSSDTHook *ptSSDTHook) 
{ 
    LONG ServiceTableBaseAddress; 
    PLONG SSDTAddress;

    if(ptSSDTHook->bHookFlag)
    {
        //读取SSDT表中索引值为0x7A的函数
        ServiceTableBaseAddress = (LONG)KeServiceDescriptorTable->ServiceTableBase;
        SSDTAddress = (PLONG)(ServiceTableBaseAddress + ptSSDTHook->nIndex * 4);

        __asm //去掉页面保护
        {
            cli;
            mov eax,cr0;
            and eax,not 10000h; //and eax,0FFFEFFFFh
            mov cr0,eax;

        }

        // 还原SSDT 
        *SSDTAddress = (ULONG)ptSSDTHook->OrgAddress; 

        __asm 
        { 
            mov eax, cr0; 
            or eax, 10000h;
            mov cr0, eax;
            sti;
        } 
        KdPrint(("UnHookSSDT还原SSDT OK \n"));

        ptSSDTHook->bHookFlag = 0;
    }
}

//测试openprocess是否被hook
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
        KdPrint(("函数被Hook！！\n"));
    }
    else
    {
        KdPrint(("函数被没有Hook！！\n"));
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
    //初始化目标属性
    InitializeObjectAttributes(&objectAttributes, //指定一个需要OBJECT_ATTRIBUTES结构地址
                               &SymbolFileName,//是一个UNICODE_STRING字串地址，指定需要操作对象名（在这里可以是符号链接名，或者设备名)
                               OBJ_CASE_INSENSITIVE, //指定此值表示 不区分大小写。
                               NULL, 
                               NULL );
    //创建文件
    ntStatus = ZwCreateFile(&hfile,//PHANDLE类型指针 用于返回打开文件的句柄
                            GENERIC_WRITE,//ACCESS_MASK类型 此值用于描述打开文件操作（读，写，或者其它）
                            &objectAttributes,  //此值是OBJECT_ATTRIBUTES结构的地址，该结构包含要打开的文件名 需要用InitializeObjectAttributes进行初始化
                            &iostatus,        //指向一个IO_STATUS_BLOCK结构，返回值之一 用于存放ZwCreateFile操作的结果状态
                            NULL,        //PLARGE_INTEGER 类型（64位整数指针）该数指定文件初始分配时的大小。该参数如果为NULL，那么文件长度将从0开始，随着写入而增长
                            FILE_ATTRIBUTE_NORMAL,        //此参数在驱动下指定为0或者FILE_ATTRIBUTE_NORMAL，如果文件不是被创建和覆盖写入 则此参数将被忽略
                            FILE_SHARE_READ,        //指定共享模式 有共享读FILE_SHARE_READ,写FILE_SHARE_WRITE,删除FILE_SHARE_DELETE这几中模式
                            FILE_OPEN_IF,        //此值表示 文件存在则打开 不存在则失败
                            FILE_SYNCHRONOUS_IO_NONALERT,        //指定文件创建或者打开的附加标志 FILE_SYNCHRONOUS_IO_NONALERT表示在文件中的所有操作均同步，并没有警报
                            NULL,         //对于设备和中间驱动程序，此参数必须是NULL
                            0 );      //对于设备和中间驱动程序，此参数必须是0
    if(NT_SUCCESS(ntStatus))
    {
        KdPrint(("创建文件成功!\n"));
    }
    else
    {
        KdPrint(("创建文件失败!\n"));
    }
    //打开文件
    //ZwOpenFile()

    //写文件
    pBuf = (PUCHAR)ExAllocatePool(PagedPool, BufLenth);
    RtlFillMemory(pBuf, BufLenth, 'a');
    RtlCopyMemory(pBuf, "i am vip_hehe", 20);

    largeInt.QuadPart = 0i64;
    ntStatus = ZwWriteFile(hfile, NULL, NULL, NULL, &iostatus, pBuf, BufLenth, &largeInt, NULL);
    KdPrint(("First:成功写入 %d 字节\n",iostatus.Information));

    largeInt.QuadPart = 20i64;
    RtlFillMemory(pBuf, BufLenth, 't');
    ntStatus = ZwWriteFile(hfile, NULL, NULL, NULL, &iostatus, pBuf, BufLenth, &largeInt, NULL);
    KdPrint(("Second:成功写入 %d 字节\n",iostatus.Information));

    //获取/修改属性
    ntStatus = ZwQueryInformationFile(hfile, &iostatus, &fsi, sizeof(FILE_STANDARD_INFORMATION), FileStandardInformation);
    if(NT_SUCCESS(ntStatus))
    {        
        KdPrint(("获取信息成功，文件大小：%d!\n", fsi.EndOfFile.QuadPart));
    }
    else
    {
        KdPrint(("获取信息失败!\n"));
    }

    ntStatus = ZwQueryInformationFile(hfile, &iostatus, &fbi, sizeof(FILE_BASIC_INFORMATION), FileBasicInformation);
    fbi.FileAttributes = FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_HIDDEN;
    ZwSetInformationFile(hfile, &iostatus, &fbi, sizeof(FILE_BASIC_INFORMATION), FileBasicInformation);

    //读文件
    largeInt.QuadPart = 0i64;
    ZwReadFile(hfile, NULL, NULL, NULL, &iostatus, pBuf, BufLenth, NULL, NULL);
    KdPrint(("Read:成功读取 %d 字节\n",iostatus.Information));
    KdPrint(("read text:\n%s\n", pBuf));

    //关闭文件
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
    KdPrint(("驱动成功被加载中.............................\n"));

    //已经被hook过，不需要处理了，只要改变PID就可以
    if(ptSSDTInlineHook->bHookFlag)
    {
        return;
    }


    //读取SSDT表中索引值为nIndex的函数
    ServiceTableBaseAddress = (LONG)KeServiceDescriptorTable->ServiceTableBase;
    SSDTAddress = (PLONG)(ServiceTableBaseAddress + ptSSDTInlineHook->nIndex * 4);
    WriteAddress =(PLONG)(*SSDTAddress);   
    gtSSDTInlineHookOpenProcess.tJmpCode.WriteAddress = (ULONG)(*SSDTAddress);

    

    KdPrint(( "真实的NtOpenProcess地址: %x\n",(int) gtSSDTInlineHookOpenProcess.tJmpCode.WriteAddress )); 
    //KdPrint((" 伪造NTOpenProcess地址: %x\n", (int)ptSSDTHook->NewAddress)); 


    __asm //去掉页面保护
    {
        cli;
        mov eax,cr0;
        and eax,not 10000h; //and eax,0FFFEFFFFh
        mov cr0,eax;
    }
    //保存原来数据
    memcpy(gtSSDTInlineHookOpenProcess.tJmpCode.OrgDat, WriteAddress, gtSSDTInlineHookOpenProcess.tJmpCode.OverWriteBytes);
    //构造跳转结构
    JmpDat[0] = 0xB8;
    memcpy(JmpDat + 1, &gtSSDTInlineHookOpenProcess.tJmpCode.DstAddress, sizeof(LONG));
    JmpDat[5] = 0xFF;
    JmpDat[6] = 0xE0;
    JmpDat[7] = JmpDat[8] = JmpDat[9] = 0x90; 

    //写入
    memcpy(WriteAddress, JmpDat, 10);

    //*SSDTAddress= ptSSDTInlineHook->NewAddress; //SSDT HOOK

    __asm 
    { 
        mov eax, cr0; 
        or eax, 10000h;
        mov cr0, eax;
        sti;
    }   
    ptSSDTInlineHook->bHookFlag = 1;//设置被HOOK标志
}
static VOID UnHookInlineSSDT(IN OUT TSSDTInlineHook *ptSSDTInlineHook)
{
    LONG ServiceTableBaseAddress; 
    PLONG SSDTAddress;

    if(ptSSDTInlineHook->bHookFlag)
    {
        __asm //去掉页面保护
        {
            cli;
            mov eax,cr0;
            and eax,not 10000h; //and eax,0FFFEFFFFh
            mov cr0,eax;

        }

        // 还原SSDT 
        memcpy(ptSSDTInlineHook->tJmpCode.WriteAddress, ptSSDTInlineHook->tJmpCode.OrgDat, ptSSDTInlineHook->tJmpCode.OverWriteBytes);

        __asm 
        { 
            mov eax, cr0; 
            or eax, 10000h;
            mov cr0, eax;
            sti;
        } 
        KdPrint(("UnHookInlineSSDT还原SSDT OK \n"));

        ptSSDTInlineHook->bHookFlag = 0;
    }

}