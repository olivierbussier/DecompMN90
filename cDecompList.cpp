#include <typeinfo>

#include "cDecompList.h"

#include "ualloc.h"

cDecompList::cDecompList()
{
  ALLOC(Next);//,typeof(Next));
}

cDecompList::~cDecompList()
{
    //dtor
}
