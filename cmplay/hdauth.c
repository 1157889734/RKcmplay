#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include "hdauth.h"

#define HARDWARE_AUTH_FILE "/home/user/bin/.dda"
#define HARDWARE_AUTH_CONTEXT "dedcfauthe"

int CmpRootFsType(char *pcFsType)
{
    FILE *pFile = NULL;
    char acBuf[128];
    char acFileName[]   = "/proc/mounts";

    pFile = fopen(acFileName, "rb");
    if (NULL == pFile)
    {
        return -1;
    }

    memset(acBuf, 0x0, sizeof(acBuf));
    while (fgets(acBuf, sizeof(acBuf), pFile))
    {
        if (strstr(acBuf, "/dev/root") != NULL) 
        {
            fclose(pFile);
            if (strstr(acBuf, pcFsType) != NULL)
            {
                return 0;
            }
            return -1;
        }
        memset(acBuf, 0x0, sizeof(acBuf));
    }
    
    fclose(pFile);
    
    return -1;
}

int GetHardwareAuthResult(void)
{
    FILE *pFile = NULL;
    char acBuf[32];
    int iLen = 0;
    int iRet = 0;
    
    pFile = fopen(HARDWARE_AUTH_FILE, "rb");
    if (NULL == pFile)
    {
        return -1;
    }
    
    memset(acBuf, 0, sizeof(acBuf));
    iLen = fread(acBuf, 1, 10, pFile);
    if (10 != iLen || strcmp(acBuf, HARDWARE_AUTH_CONTEXT))
    {
        return -1;
    }
    
    fclose(pFile);
    
    //iRet = CmpRootFsType("cramfs");
    
    return iRet;
}

/*int main()
{
    int iRet = 0;
    
    iRet = GetHardwareAuthResult();
    printf("iRet %d\n", iRet);
    
    return 0;
}*/

