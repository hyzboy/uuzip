#include<hgl/log/LogInfo.h>
#include<hgl/CodePage.h>
#include<hgl/filesystem/FileSystem.h>
#include<hgl/io/FileInputStream.h>
#include<hgl/type/MemBlock.h>
#include<stdio.h>
#include<zip.h>

using namespace hgl;
using namespace hgl::io;

int os_main(int argc,os_char **argv)
{
    os_out<<OS_TEXT("uuzip ver 0.01")<<std::endl;
    os_out<<OS_TEXT("Uncompress zip files with Unicode. offical web: ")<<HGL_OFFICAL_WEB_URL_OS<<std::endl<<std::endl;

    if(argc<2)
    {
        os_out<<OS_TEXT("Usage:   uuzip <charset or codepage> <zip file> [output directory]")<<std::endl;
        os_out<<OS_TEXT("         uuzip charset\n")<<std::endl;
        os_out<<OS_TEXT("Example: uuzip shift-jis c:\\test.zip c:\\test")<<std::endl<<std::endl;
        os_out<<OS_TEXT("Example: uuzip 932 c:\\test.zip c:\\test")<<std::endl<<std::endl;
        return 0;
    }

    if(hgl::stricmp(argv[1],OS_TEXT("charset"))==0)
    {
        os_out<<OS_TEXT("Usage: uuzip <charset> <zip file> [output directory]\n")<<std::endl
            <<OS_TEXT("Charsets: gb2312,gbk,gb18030,big5,shift-jis,ks_c_5601-1987.....")<<std::endl
            <<OS_TEXT("Codepage: 936,950,20936,54936...(only Windows platform)")<<std::endl;

        return 0;
    }

    const OSString charset_name=argv[1];
    const OSString zip_filename=argv[2];

    os_out<<OS_TEXT("zip file: ")<<argv[2]<<std::endl;
    os_out<<OS_TEXT("char set: ")<<argv[1]<<std::endl;
    
    zip_error zerr;
    zip_error_init(&zerr);

    zip_source_t *zs=zip_source_win32w_create(argv[2],0,-1,&zerr);
   
    zip_error_fini(&zerr);
    zip_error_init(&zerr);
    
    zip_t *za=zip_open_from_source(zs,0,&zerr);
    zip_error_fini(&zerr);

    zip_stat_t sb;

    CharSet cs;

    if(hgl::isdigit(argv[1][0]))
    {
        uint cp;

        if(!hgl::stou(argv[1],cp))
        {
            std::wcerr<<L"error codepage: "<<argv[1]<<std::endl;
            return -1;
        }

        cs.codepage=cp;

        os_out<<OS_TEXT("use codepage: ")<<cs.codepage<<std::endl;
    }
    else
    {
        os_out<<OS_TEXT("use charsets: ")<<argv[1]<<std::endl;

        strcpy(cs.charset,to_u8(argv[1]));
        cs.codepage=FindCodePage(cs.charset);
    }

    MemBlock<u16char> filename(256);
    int len=0;
    int u16len=256;

    if(zs)
    {
        for(int i=0;i<zip_get_num_entries(za,0);i++)
        {
            if(zip_stat_index(za,i,ZIP_FL_ENC_RAW,&sb)==0)
            {
                len=hgl::strlen(sb.name);

                u16len=get_utf16_length(cs,sb.name,len);

                if(u16len>filename.GetMaxLength())
                    filename.SetLength(u16len);
                    
                to_utf16(cs,filename,filename.GetMaxLength(),sb.name,len);

                os_out<<i<<OS_TEXT(": ")<<filename.data()<<OS_TEXT(", size: ")<<sb.size<<std::endl;
            }
        }

        zip_source_close(zs);
        zip_source_free(zs);
    }

    if(za)
    {
        zip_close(za);
    }

    return 0;
}