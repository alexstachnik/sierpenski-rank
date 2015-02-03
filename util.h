
#ifndef UTIL_H_
#define UTIL_H_

typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;

size_t roundUpPowerOfThree(size_t n)
{
  size_t i=1;
  while (i<n) {
    i*=3;
  }
  return i;
}

int min(int a, int b)
{
  return a<b?a:b;
}

#endif
