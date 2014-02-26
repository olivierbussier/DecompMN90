#ifndef UTIL_H_INCLUDED
#define UTIL_H_INCLUDED

void *Alloc  (unsigned int size);
void *Realloc(void *buffer,unsigned int size);
void  Free   (void *buffer);

char *strinit(void);
char *str    (char *buffer, const char *format, ...);
void  strend (char *buffer);

void   Fatal (int ErrorCode, const char *message);

#define ALLOC(a) (((a)=Alloc(sizeof(*(a)))))

#define VERBOSE if (Verbose) printf

#endif // UTIL_H_INCLUDED
