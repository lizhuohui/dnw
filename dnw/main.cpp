#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

const char* dev = "/dev/secbulk0";

void CheckInputParameter(int argc, char* argv[])
{
    //    if(argc == 1) // when no parameter input
    //    {
    //        printf("Auto set input file is 0x50008000\n");
    //        argc = 2;
    //        char *p = argv++;
    //        p = (char*)malloc(sizeof(char*));
    //        unsigned long tmp = 0x50008000;
    //        *p = &tmp;
    //    }
    if( 2 != argc )
    {
        printf("Usage: dwn <filename>\n");
        while (1)
        {
        }
    }
}

int OpenFile(char *FileName)
{
    int fd = open(FileName, O_RDONLY);
    if (fd == -1)
    {
        printf("Error: Can not open file - %s\n", FileName);
        while (1)
        {
        }
    }
    return fd;
}

struct stat GetStates(int fd)
{
    struct stat file_stat;
    if( -1 == fstat(fd, &file_stat) )
    {
        printf("Get file size filed!\n");
        while (1)
        {
        }
    }
    return file_stat;
}

int main(int argc, char* argv[])
{
    CheckInputParameter(argc, argv);

    int fd = OpenFile(argv[1]);

    struct stat file_stat = GetStates(fd);

    unsigned char* file_buffer = (unsigned char*)malloc(file_stat.st_size+10);
    if(NULL == file_buffer)
    {
        printf("malloc failed!\n");
        //goto error;
    }
    if( file_stat.st_size !=  read(fd, file_buffer+8, file_stat.st_size))
    {
        printf("Read file failed!\n");
        //goto error;
    }

    printf("file name : %s\n", argv[1]);
    printf("file size : %d bytes\n", (int)file_stat.st_size);

    int fd_dev = open(dev, O_WRONLY);
    if( -1 == fd_dev)
    {
        printf("Can not open %s\n", dev);
        //goto error;
    }

    *((unsigned long*)file_buffer) = 0x32000000; 	//load address
    *((unsigned long*)file_buffer+1) = file_stat.st_size+10;	//file size
    unsigned short sum = 0;

    int i;
    for (i=8; i<file_stat.st_size+8; i++)
    {
        sum += file_buffer[i];
    }
    /*****************************/
    file_buffer [file_stat.st_size + 8] = sum & 0xff;
    file_buffer [file_stat.st_size + 9] = sum >> 8;
    /*****************************/
    printf("Writing data...\n");
    size_t remain_size = file_stat.st_size+10;
    size_t block_size = remain_size / 100;
    size_t writed = 0;
    while(remain_size>0)
    {
        size_t to_write = remain_size > block_size ? block_size:remain_size;
        if( to_write != write(fd_dev, file_buffer+writed, to_write))
        {
            printf("failed!\n");
            return 1;
        }
        remain_size -= to_write;
        writed += to_write;
        printf("\r%d%\t %d bytes     ", writed*100/(file_stat.st_size+10), writed);
        fflush(stdout);

    }

    printf("OK\n");
    return 0;

//error:
    if(-1!=fd_dev) close(fd_dev);
    if(fd != -1) close(fd);
    if( NULL!=file_buffer )
        free(file_buffer);
    return -1;
}
