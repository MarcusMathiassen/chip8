#include "utility.h"
#include <assert.h> // assert
#include <stdarg.h> // va_list, va_start, va_end
#include <stdio.h> // printf, vprintf
#include <stdlib.h> // malloc, realloc, calloc
#include <string.h> // memcpy
#include <time.h> // timeval
#include <unistd.h>

#define RESET "\033[0m"
#define GRAY "\033[30m"
#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define RGB_GRAY "\033[38;2;110;110;110;m"
#define RGB_WHITE "\033[38;2;255;255;255;m"

//------------------------------------------------------------------------------
//                               Printing Functions
//------------------------------------------------------------------------------

void info(char* fmt, ...)
{
    assert(fmt);
    va_list args;
    va_start(args, fmt);
    printf("%s", RGB_GRAY);
    vprintf(fmt, args);
    puts(RESET);
    va_end(args);
}

void warning(char* fmt, ...)
{
    assert(fmt);
    va_list args;
    va_start(args, fmt);
    printf("%s", YELLOW);
    vprintf(fmt, args);
    puts(RESET);
    va_end(args);
}

void success(char* fmt, ...)
{
    assert(fmt);
    va_list args;
    va_start(args, fmt);
    printf("%s", GREEN);
    vprintf(fmt, args);
    puts(RESET);
    va_end(args);
}

void error(char* fmt, ...)
{
    assert(fmt);
    va_list args;
    va_start(args, fmt);
    printf("%s", RED);
    vprintf(fmt, args);
    puts(RESET);
    va_end(args);
    exit(1);
}

//------------------------------------------------------------------------------
//                               File Utility Functions
//------------------------------------------------------------------------------

char* get_file_path_from_directory(char* dir, char* filename)
{
    assert(dir);
    assert(filename);
    u8  strbuf[1000];
    s64 d_len = strlen(dir);
    s64 f_len = strlen(filename);
    s64 len   = d_len + f_len;
    assert(len < 1000);
    memcpy(strbuf, dir, d_len); // copy dir into strbuf
    memcpy(strbuf + d_len, filename, f_len); // append filename
    char* str = xmalloc(len + 1);
    memcpy(str, strbuf, len);
    str[len] = 0;
    return str;
}

char* get_file_extension(char* filename)
{
    assert(filename);
    s64 len = strlen(filename);
    s64 i   = 0;
    while (filename[len - (++i)] != '.')
        continue;
    ++len; // skip the '.'
    char* str = xmalloc(i + 1);
    memcpy(str, filename + len - i, i);
    str[i] = 0;
    return str;
}

char* remove_file_extension(char* filename)
{
    assert(filename);
    s64 len = strlen(filename);
    s64 i   = 0;
    while (filename[len - (++i)] != '.')
        continue;
    char* str = xmalloc(len - i + 1);
    memcpy(str, filename, len - i);
    str[len-i] = 0;
    return str;
}

char* get_file_directory(char* filename)
{
    assert(filename);
    s64 len = strlen(filename);
    while (filename[--len] != '/')
        continue;
    ++len; // we preserve the '/'
    char* str = xmalloc(len + 1);
    memcpy(str, filename, len);
    str[len] = 0;
    return str;
}
char* get_file_name(char* filename)
{
    assert(filename);
    s64 len = strlen(filename);
    s64 i   = 0;
    while (filename[len - (++i)] != '/')
        continue;
    ++len; // skip the '/'
    char* str = xmalloc(i + 1);
    memcpy(str, filename + len - i, i);
    str[i] = 0;
    return str;
}
void write_to_file(char* filename, char* buffer)
{
    assert(filename);
    assert(buffer);
    FILE* f = fopen(filename, "w");
    if (!f) {
        printf("Error opening file!\n");
        exit(1);
    }
    fputs(buffer, f);
    fclose(f);
}

char* get_file_content(char* filename)
{
    assert(filename);
    char* buffer = NULL;
    s64   string_size, read_size;
    FILE* handler = fopen(filename, "r");

    if (handler) {
        // Seek the last byte of the file
        fseek(handler, 0, SEEK_END);
        // Offset from the first to the last byte, or in other words, filesize
        string_size = ftell(handler);
        // go back to the start of the file
        rewind(handler);

        // Allocate a string that can hold it all
        buffer = (char*)xmalloc(sizeof(u8) * (string_size + 1));

        // Read it all in one operation
        read_size = fread(buffer, sizeof(u8), string_size, handler);

        // fread doesn't set it so put a \0 in the last position
        // and buffer is now officially a string
        buffer[string_size] = '\0';

        if (string_size != read_size) {
            // Something went wrong, throw away the memory and set
            // the buffer to NULL
            free(buffer);
            buffer = NULL;
        }

        // Always remember to close the file.
        fclose(handler);
    }

    return buffer;
}

//------------------------------------------------------------------------------
//                               General Purpose
//------------------------------------------------------------------------------

void* _malloc(s64 bytes, char* file, s32 line)
{
    assert(bytes != 0);
    // info("malloc(%lld) called. file: %s line: %s", bytes, file, line);
    void* alloc = malloc(bytes);
    if (!alloc) error("malloc(%lld) failed. file: %s line: %s", bytes, file, line);
    return alloc;
}

void* _calloc(s64 size, s64 bytes, char* file, s32 line)
{
    assert(size != 0);
    assert(bytes != 0);
    // info("calloc(%lld, %lld) called. file: %s line: %s", size, bytes, file, line);
    void* alloc = calloc(size, bytes);
    if (!alloc) error("calloc(%lld, %lld) failed. file: %s line: %s", size, bytes, file, line);
    return alloc;
}

void* _realloc(void* ptr, s64 bytes, char* file, s32 line)
{
    assert(ptr);
    assert(bytes != 0);
    // info("realloc(%lld, %lld) called. file: %s line: %s", (int)ptr, bytes, file, line);
    void* alloc = realloc(ptr, bytes);
    if (!alloc) error("realloc(%lld, %lld) failed. file: %s line: %s", ptr, bytes, file, line);
    return alloc;
}

char* strf(char* fmt, ...)
{
    assert(fmt);
    va_list args;
    va_start(args, fmt);
    s64 n = 1 + vsnprintf(0, 0, fmt, args);
    va_end(args);

    char* str = xmalloc(n);

    va_start(args, fmt);
    vsnprintf(str, n, fmt, args);
    va_end(args);

    return str;
}

// Color Whell
char* colors[6] = {
    "\033[31m", // red
    "\033[32m", // green
    "\033[33m", // yellow
    "\033[34m", // blue
    "\033[35m", // magenta
    "\033[36m", // cyan,
};
int   counter      = 0;
int   colors_count = 6;
char* get_previous_color(void)
{
    if (counter == 0) counter = 6;
    return colors[--counter];
}
char* get_next_color(void)
{
    if (counter == colors_count) counter = 0;
    return colors[counter++];
}
char* wrap_with_colored_parens(char* str)
{
    char* current_color = get_next_color();
    return strf("%s(\033[00m%s%s)\033[00m", current_color, str, current_color);
}
char* give_unique_color(char* str)
{
    char* current_color = get_next_color();
    return strf("%s%s\033[00m", current_color, str);
}
char* get_unique_color(void) { return get_next_color(); }
//

//------------------------------------------------------------------------------
//                               Timing Utility Functions
//------------------------------------------------------------------------------

/*
author: jbenet
os x, compile with: gcc -o testo test.c
linux, compile with: gcc -o testo test.c -lrt
*/

#include <stdio.h>
#include <sys/time.h>
#include <time.h>

#ifdef __MACH__
#include <mach/clock.h>
#include <mach/mach.h>
#endif

f64 get_time(void)
{
    f64             ms;
    time_t          s;
    struct timespec ts;

#ifdef __MACH__ // OS X does not have clock_gettime, use clock_get_time
    clock_serv_t    cclock;
    mach_timespec_t mts;
    host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
    clock_get_time(cclock, &mts);
    mach_port_deallocate(mach_task_self(), cclock);
    ts.tv_sec  = mts.tv_sec;
    ts.tv_nsec = mts.tv_nsec;
#else
    clock_gettime(CLOCK_REALTIME, &ts);
#endif
    s  = ts.tv_sec;
    ms = ts.tv_nsec / 1.0e6;
    if (ms > 999.0) {
        ++s;
        ms = 0.0;
    }
    return ms;
}

//------------------------------------------------------------------------------
//                               Tests
//------------------------------------------------------------------------------
void utility_tests(void)
{
    // These leak memory.

    // get_file_directory
    assert(strcmp(get_file_directory("./b/m.thi"), "./b/") == 0);
    assert(strcmp(get_file_directory("./b/m/m.thi"), "./b/m/") == 0);

    // get_file_extension
    assert(strcmp(get_file_extension("./b/m.thi"), "thi") == 0);
    assert(strcmp(get_file_extension("./b/m.thigegerg/o.hrifj"), "hrifj") == 0);

    // get_file_name
    assert(strcmp(get_file_name("./b/m.thi"), "m.thi") == 0);
    assert(strcmp(get_file_name("./b/m/hergergerg.thi"), "hergergerg.thi") == 0);

    // get_file_path_from_directory
    assert(strcmp(get_file_path_from_directory("./b/", "test.thi"), "./b/test.thi") == 0);
    assert(strcmp(get_file_path_from_directory("./b/b/", "test.thi"), "./b/b/test.thi") == 0);
}
