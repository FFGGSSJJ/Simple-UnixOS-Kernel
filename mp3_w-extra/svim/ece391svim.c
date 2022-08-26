#include <stdint.h>

#include "ece391support.h"
#include "ece391syscall.h"
#include "lib.h"

#define NUM_COLS    80
#define NUM_ROWS    25
#define BUFSIZE 256
#define file_max_size 16384 // 4096*4

uint8_t file[file_max_size];
int32_t size;
int32_t fd;

int open_file(void)
{
    uint8_t buf[BUFSIZE];

    if (0 != ece391_getargs(buf, BUFSIZE))
    {
        ece391_fdputs(1, (uint8_t *)"could not read arguments\n");
        return -1;
    }

    if (-1 == (fd = ece391_open(buf)))
    {
        ece391_fdputs(1, (uint8_t *)"file not found\n");
        return -1;
    }

    return ece391_read(fd, file, file_max_size);
}

void show_file(void)
{
    clear();
    int i;
    for (i = 0; i < size; i++)
    {
        putc(file[i]);
    }
}

// move terminal display to the bottom
void init_cmd(void)
{
    int i;
    for (i = 0; i < NUM_ROWS; i++)
        ece391_fdputs(1, (uint8_t*)"\n");
}

int main()
{
    int32_t cnt;
    uint8_t buf[BUFSIZE];

    // initialize file buffer and size
    if (-1 == (size = open_file()))
        return -1;

    init_cmd();

    set_video_mode();

    show_file();

    // cmd-modify-show loop
    while (1)
    {
        // get cmd
        if (-1 == (cnt = ece391_read(0, buf, BUFSIZE - 1)))
        {
            ece391_fdputs(1, (uint8_t *)"read from keyboard failed\n");
            return 3;
        }
        if (cnt > 0 && '\n' == buf[cnt - 1])
            cnt--;
        buf[cnt] = '\0';

        // parse cmd
        // case x
        if ('x' == buf[0] && size > 0)
        {
            size--;
            file[size] = '\0';
        }

        // case a
        if ('a' == buf[0])
        {
            strncpy((int8_t*)(file + size), (int8_t*)(buf + 1), cnt);
            size += cnt - 1; // since we ignore the first char
        }

        // case o
        if ('o' == buf[0])
        {
            file[size++] = '\n';
        }

        // case z
        if (('z' == buf[0]))
        {
            clear();
            break;
        }

        // show modified file
        show_file();
    }

    if (size != ece391_write(fd, file, size))
        return -1;

    return 0;
}
