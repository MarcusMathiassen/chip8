#ifndef UTILITY_H
#define UTILITY_H

#include "typedefs.h"

//------------------------------------------------------------------------------
//                               Printing Functions
//------------------------------------------------------------------------------

void info(char* fmt, ...);
void warning(char* fmt, ...);
void success(char* fmt, ...);
void error(char* fmt, ...);

//------------------------------------------------------------------------------
//                               File Functions
//------------------------------------------------------------------------------
char* get_file_path_from_directory(char* dir, char* filename);
char* get_file_extension(char* filename);
char* remove_file_extension(char* filename);
char* get_file_directory(char* filename);
char* get_file_name(char* filename);
char* get_file_content(char* filename);
void  write_to_file(char* filename, char* buffer);

//------------------------------------------------------------------------------
//                               General Purpose
//------------------------------------------------------------------------------
void* _malloc(s64 bytes, char* file, int line);
void* _calloc(s64 size, s64 bytes, char* file, int line);
void* _realloc(void* ptr, s64 bytes, char* file, int line);

#define xmalloc(n) _malloc(n, __FILE__, __LINE__)
#define xrealloc(n, m) _realloc(n, m, __FILE__, __LINE__)
#define xcalloc(n, m) _calloc(n, m, __FILE__, __LINE__)

char* strf(char* fmt, ...);

char* get_previous_color(void);
char* get_next_color(void);
char* wrap_with_colored_parens(char* str);
char* give_unique_color(char* str);
char* get_unique_color(void);

//------------------------------------------------------------------------------
//                               Timing Functions
//------------------------------------------------------------------------------
f64 get_time(void);

//------------------------------------------------------------------------------
//                               Tests
//------------------------------------------------------------------------------

void utility_tests(void);

#endif
