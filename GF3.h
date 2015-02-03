
#ifndef GF3_H_
#define GF3_H_

#include "util.h"
#include <assert.h>

typedef uint32_t Position;
typedef uint32_t Element;

#define POSITION_MASK 0x7FFFFFFF
#define ELEMENT_MASK 0x80000000

inline Element setElt(Position pos, int val)
{
  assert((val>0) && (val<=2) && (pos < ELEMENT_MASK));
  return ((val-1) << 31)|pos;
}

inline int getElt(Element elt)
{
  return ((elt&ELEMENT_MASK)>>31)+1;
}

inline Position getPos(Element elt)
{
  return elt&POSITION_MASK;
}

inline int sum3(int a, int b)
{
  int result=a+b;
  return (result>=3)?(result-3):result;
}

inline int mul3(int a, int b)
{
  int result=a*b;
  return (result>=3)?(result-3):result;
}

inline int neg3(int a)
{
  if (a==0) return 0;
  return 3-a;
}

inline int sub3(int a, int b)
{
  return sum3(a,neg3(b));
}

int inv3(int a)
{
  assert(a != 0);
  if (a==1) return 1;
  return 2;
}

int div3(int a, int b)
{
  return mul3(a,inv3(b));
}

#endif
