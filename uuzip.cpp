#include<hgl/log/LogInfo.h>
#include<hgl/CodePage.h>
#include<hgl/filesystem/FileSystem.h>
#include<hgl/io/FileOutputStream.h>
#include<hgl/type/MemBlock.h>
#include<stdio.h>
#include<zip.h>

using namespace hgl;
using namespace hgl::io;

constexpr size_t TEMP_BUF_SIZE=HGL_SIZE_1MB;
char temp_buffer[TEMP_BUF_SIZE];
    
bool UnzipFile(const OSString &filename,zip_file_t *zf,const int filesize)
{
    int fs=0;
    int size;

    FileOutputStream fos;
                        
    if(!fos.CreateTrunc(filename))
    {
        LOG_ERROR(OS_TEXT("Create file failed,filename: ")+filename);
        zip_fclose(zf);
        return(false);
    }

    while(fs<filesize)
    {
        size=zip_fread(zf,temp_buffer,TEMP_BUF_SIZE);

        if(size<0)
        {
            LOG_ERROR(OS_TEXT("read file failed at zip"));
            break;
        }

        if(fos.WriteFully(temp_buffer,size)!=size)
        {
            LOG_ERROR(OS_TEXT("write file failed at ")+OSString::valueOf(fs));
            break;
        }

        fs+=size;
    }
    
    fos.Close();
    zip_fclose(zf);

    if(fs==filesize)
    {
        return(true);
    }
    else
    {
        filesystem::FileDelete(filename);
        LOG_ERROR("Uncompress failed.");
        return(false);
    }
}

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

    logger::InitLogger(OS_TEXT("UUZip"));

    OSString output_directory;

    if(argc>3)
    {
        if(argv[3][0]=='.'
         &&argv[3][1]==0)
            filesystem::GetCurrentPath(output_directory);
        else
        {
            output_directory=argv[3];
        }
    }
    else
    {
        filesystem::GetCurrentPath(output_directory);
    }

    LOG_INFO(OS_TEXT("zip file: ")+OSString(argv[2]));
    LOG_INFO(OS_TEXT("output directory: ")+output_directory);

    if(!filesystem::IsDirectory(output_directory))
    {
        LOG_ERROR("output directory error!");
        return 1;
    }
    
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
            LOG_ERROR(OS_TEXT("error codepage: ")+OSString(argv[1]));
            return -1;
        }

        cs.codepage=cp;

        LOG_INFO(OS_TEXT("use codepage: ")+OSString::valueOf(cs.codepage));
    }
    else
    {
        LOG_ERROR(OS_TEXT("use charset: ")+OSString(argv[1]));

        strcpy(cs.charset,to_u8(argv[1]));
        cs.codepage=FindCodePage(cs.charset);
    }

    MemBlock<u16char> filename(1024);
    u16char last_char;
    OSString os_filename;
    OSString full_filename;
    int len=0;
    int u16len=filename.GetMaxLength();

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

                last_char=filename[u16len-1];
                
                os_filename=OSString(filename);
                full_filename=filesystem::MergeFilename(output_directory,os_filename);
                    
                if(sb.size==0
                   &&(last_char=='/'
                    ||last_char=='\\'))
                {
                    LOG_INFO(OS_TEXT("Path: ")+os_filename);
                    
                    if(!filesystem::IsDirectory(full_filename))
                    if(!filesystem::MakePath(full_filename))
                    {
                        LOG_ERROR(OS_TEXT("make path error: ")+full_filename);
                        return 1;
                    }
                }
                else
                {   
                    LOG_INFO(OSString::valueOf(i)+OS_TEXT(": ")+os_filename+OS_TEXT(", size: ")+OSString::valueOf(sb.size));

                    zip_file_t *zf=zip_fopen_index(za,i,0);

                    if(!zf)
                    {
                        LOG_ERROR(OS_TEXT("open file failed at zip,filename: ")+os_filename);
                    }
                    else
                    {
                        UnzipFile(full_filename,zf,sb.size);
                    }
                }
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