#include<iostream>
#include<hgl/log/LogInfo.h>

using namespace hgl;

int os_main(int argc,os_char **argv)
{
    LOG_INFO(OS_TEXT("uuzip ver 0.01"));
    LOG_INFO(OS_TEXT("Uncompress zip files with Unicode\n"));

    if(argc<2)
    {
        LOG_INFO(OS_TEXT("Usage: uuzip <zip file>\n"));
        return 0;
    }

    return 0;
}