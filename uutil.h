#ifndef UTIL_H_INCLUDED
#define UTIL_H_INCLUDED

char *strinit(void);
char *str    (char *buffer, const char *format, ...);
void  strend (char *buffer);

#define VERBOSE if (Verbose) printf

#endif // UTIL_H_INCLUDED
