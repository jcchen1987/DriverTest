#ifndef _DDK_DEF_H_
#define _DDK_DEF_H_

#define INITCODE code_seg("INIT") /*指的代码运行后 就从内存释放掉*/
#define PAGECODE code_seg("PAGE") /*表示内存不足时，可以被置换到硬盘*/

//SSDT结构
typedef struct _ServiceDescriptorTable 
{
    PVOID ServiceTableBase; //System Service Dispatch Table 的基地址  
    PVOID ServiceCounterTable;    //包含着 SSDT 中每个服务被调用次数的计数器。这个计数器一般由sysenter 更新。 
    unsigned int NumberOfServices;//由 ServiceTableBase 描述的服务的数目。  
    PVOID ParamTableBase; //包含每个系统服务参数字节数表的基地址-系统服务参数表 
}*PServiceDescriptorTable;  

//SSDT中函数地址及其索引
typedef struct
{
    UNICODE_STRING FunctionName;//导出函数名
    int nIndex;//函数在SSDT中的序号
}TMyFuncInfo;//, *PTFuncInfo;

//SSDT Hook结构
typedef struct
{
    int bHookFlag;
    int nIndex;
    int  nProtectPID;
    ULONG OrgAddress;
    ULONG NewAddress;
}TSSDTHook;

typedef struct
{
    ULONG DstAddress; //要跳转的地址
    ULONG WriteAddress; //写入jmp指令的地址
    CHAR OrgDat[20]; //保存原来数据地址，采用mov eax,Address  jmp eax的写法--B8 Address; FF E0;
    LONG OverWriteBytes; // 时间覆盖指令长度
}TJMPCode;

typedef struct
{
    int bHookFlag;
    int nIndex;
    int  nProtectPID;
    TJMPCode tJmpCode;
    //ULONG OrgAddress;
}TSSDTInlineHook;

#endif