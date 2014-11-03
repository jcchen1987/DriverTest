#include <stdio.h>
#include <Windows.h>
#include "CtlCodeDef.h"

int Arithmetic(HANDLE hDevice, int nNum1, int nNum2, int Operator)
{
    int InBuf[3];
    InBuf[0] = nNum1;
    InBuf[1] = nNum2;
    InBuf[2] = Operator;

    int nRet;
    DWORD dwRetByte = 0;
    DeviceIoControl(hDevice, ARITHMETIC_CODE_BUFFER, InBuf, 3 * 4, &nRet, 4, &dwRetByte, NULL);
    return nRet;
}

void hook(HANDLE hDevice, int pid, DWORD dwCode)
{
    int nRet;
    DWORD dwRetByte = 0;
    DeviceIoControl(hDevice, dwCode, &pid, 4, &nRet, 4, &dwRetByte, NULL);    
}

int main()
{
    char SymLinkName[] = "\\\\.\\my888";
    HANDLE hDevice = CreateFile(SymLinkName, //\\??\\My_DriverLinkName
                                GENERIC_READ | GENERIC_WRITE,
                                0,		// share mode none
                                NULL,	// no security
                                OPEN_EXISTING,
                                FILE_ATTRIBUTE_NORMAL,
                                NULL );		// no template
    if(hDevice == INVALID_HANDLE_VALUE)
    {
        printf("设备打开错误[%s], err[%d]\n", SymLinkName, GetLastError());
        return 0;
    }

    int x = 1000;
    int y = 0;
    int z = 1;
//     z = Arithmetic(hDevice, x, y, 0);
//     printf("%d + %d = %d\n", x, y, z);
// 
//     z = Arithmetic(hDevice, x, y, 1);
//     printf("%d - %d = %d\n", x, y, z);
// 
//     z = Arithmetic(hDevice, x, y, 2);
//     printf("%d * %d = %d\n", x, y, z);
// 
//     z = Arithmetic(hDevice, x, y, 3);
    printf("%d / %d = %d\n", x, y, z);

    printf("\n\ninputpid:\n");
    int pid = 0;
    scanf("%d", &pid);
    hook(hDevice, pid, SSDTHOOK_CODE_BUFFER);

    return 1;
}