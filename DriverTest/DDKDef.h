#ifndef _DDK_DEF_H_
#define _DDK_DEF_H_

#define INITCODE code_seg("INIT") /*ָ�Ĵ������к� �ʹ��ڴ��ͷŵ�*/
#define PAGECODE code_seg("PAGE") /*��ʾ�ڴ治��ʱ�����Ա��û���Ӳ��*/

//SSDT�ṹ
typedef struct _ServiceDescriptorTable 
{
    PVOID ServiceTableBase; //System Service Dispatch Table �Ļ���ַ  
    PVOID ServiceCounterTable;    //������ SSDT ��ÿ�����񱻵��ô����ļ����������������һ����sysenter ���¡� 
    unsigned int NumberOfServices;//�� ServiceTableBase �����ķ������Ŀ��  
    PVOID ParamTableBase; //����ÿ��ϵͳ��������ֽ�����Ļ���ַ-ϵͳ��������� 
}*PServiceDescriptorTable;  

//SSDT�к�����ַ��������
typedef struct
{
    UNICODE_STRING FunctionName;//����������
    int nIndex;//������SSDT�е����
}TMyFuncInfo;//, *PTFuncInfo;

//SSDT Hook�ṹ
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
    ULONG DstAddress; //Ҫ��ת�ĵ�ַ
    ULONG WriteAddress; //д��jmpָ��ĵ�ַ
    CHAR OrgDat[20]; //����ԭ�����ݵ�ַ������mov eax,Address  jmp eax��д��--B8 Address; FF E0;
    LONG OverWriteBytes; // ʱ�串��ָ���
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