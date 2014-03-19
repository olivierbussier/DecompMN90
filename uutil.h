#ifndef UTIL_H_INCLUDED
#define UTIL_H_INCLUDED

void strinit(void);
char *strget(void);
void str    (const char *format, ...);
void strend (void);

#define VERBOSE if (Verbose) printf

#endif // UTIL_H_INCLUDED
