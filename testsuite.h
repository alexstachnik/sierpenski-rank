
#ifndef TESTSUITE_H_
#define TESTSUITE_H_

#include "testskiplist.h"

void testPos1(int n)
{
  assert(getPos(setElt(n,1))==n);
  assert(getPos(setElt(n,2))==n);
  assert(getElt(setElt(n,2))==2);
  assert(getElt(setElt(n,1))==1);
}

void testNChooseK()
{
  struct MemoData memoData;
  setupMemoData(5,&memoData);
  assert(nChooseK(0,0,memoData)==1);
  assert(nChooseK(1,0,memoData)==1);
  assert(nChooseK(1,1,memoData)==1);
  assert(nChooseK(2,0,memoData)==1);
  assert(nChooseK(2,1,memoData)==2);
  assert(nChooseK(2,2,memoData)==1);
  assert(nChooseK(3,0,memoData)==1);
  assert(nChooseK(3,1,memoData)==0);
  assert(nChooseK(3,2,memoData)==0);
  assert(nChooseK(3,3,memoData)==1);
  assert(nChooseK(4,0,memoData)==1);
  assert(nChooseK(4,1,memoData)==1);
  assert(nChooseK(4,2,memoData)==0);
  assert(nChooseK(4,3,memoData)==1);
  assert(nChooseK(4,4,memoData)==1);
  freeMemoData(&memoData);
}

void test()
{
  assert(sum3(1,1)==2);
  assert(sum3(1,2)==0);
  assert(sum3(2,2)==1);
  assert(sub3(2,2)==0);
  assert(sub3(1,2)==2);
  assert(mul3(1,0)==0);
  assert(mul3(2,2)==1);
  assert(div3(2,2)==1);
  assert(div3(1,2)==2);

  testPos1(0x70000000);
  testPos1(0x7FFFFFFF);
  testPos1(0x00000000);
  testPos1(0x00000001);

  testNChooseK();

  assert(rowTest1());
  assert(rowTest2(0));
  assert(rowTest2(1));
  assert(rowTest2(2));
}

#endif
