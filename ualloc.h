#ifndef ALLOC_H_INCLUDED
#define ALLOC_H_INCLUDED

void *Alloc  (unsigned int size);
void *Realloc(void *buffer,unsigned int size);
void  Free   (void *buffer);

#define ALLOC(a) (a)=((typeof(a))Alloc(sizeof(*(a))))

#endif // ALLOC_H_INCLUDED
