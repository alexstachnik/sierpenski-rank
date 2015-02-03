
#ifndef TESTSKIPLIST_H_
#define TESTSKIPLIST_H_

#include <stdio.h>
#include <limits.h>
#include <assert.h>
#include <stdlib.h>
#include <malloc.h>
#include <time.h>

#include "util.h"
#include "GF3.h"
#include "fastrand.h"
#include "row-skip-list.h"

void makeRow(int len,
	     int nnz,
	     int *arrRow,
	     struct Row *sRow,
	     Element *elts)
{
  int i;
  for (i=0;i<len;++i) {
    arrRow[i]=0;
  }
  for (i=0;i<nnz;++i) {
    insert(sRow,elts[i]);
    arrRow[getPos(elts[i])]=getElt(elts[i]);
  }

  bunchRow(sRow);
}

void sumArrRow(int len,
	       int *rowA,
	       int *rowB,
	       int alpha)
{
  int i;
  for (i=0;i<len;++i) {
    rowA[i]=sum3(rowA[i],mul3(alpha,rowB[i]));
  }
}

int cmpRows(struct Row* sRow,
	    int *arrRow,
	    int len)
{
  int i;

  for (i=0;i<len;++i) {
    if (lookup(sRow,i) != arrRow[i]) {
      return 0;
    }
  }

  return 1;
}

int rowTest1()
{
  struct Row r;
  Element elts[8]={setElt(5,1),setElt(3,1),setElt(7,1),setElt(7,2),setElt(6,1),setElt(2,1),setElt(1,1),setElt(0,1)};
  int arr[20];

  initRow(&r);

  makeRow(20,8,arr,&r,elts);
  if (!cmpRows(&r,arr,20))
    return 0;
  insert(&r,setElt(15,1));
  if (cmpRows(&r,arr,20))
    return 0;  


  freeRow(&r);
  return 1;
}

int rowTest2(int alpha)
{
  struct Row rA,rB;
  Element eltsA[]={setElt(4,1),setElt(6,1),setElt(8,1)};
  Element eltsB[]={setElt(1,1),setElt(3,1),setElt(5,1),setElt(6,1),setElt(9,1)};
  int arrA[20],arrB[20];

  initRow(&rA);
  makeRow(20,3,arrA,&rA,eltsA);
  initRow(&rB);
  makeRow(20,5,arrB,&rB,eltsB);
  if (!cmpRows(&rA,arrA,20) || (!cmpRows(&rB,arrB,20))) {
    return 0;
  }

  addRow(&rA,&rB,alpha);
  sumArrRow(20,arrA,arrB,alpha);
  if (!cmpRows(&rA,arrA,20) || (!cmpRows(&rB,arrB,20))) {
    return 0;
  }

  freeRow(&rA);
  freeRow(&rB);

  return 1;
}

void rowMemTest()
{
  int i;
  struct Row r;
  initRow(&r);

  for (i=0;i<1000;++i) {
    insert(&r,setElt(i,1));
  }
  printf("1000\t%d\n",mallinfo().uordblks);

  for (;i<10000;++i) {
    insert(&r,setElt(i,1));
  }
  printf("10000\t%d\n",mallinfo().uordblks);

  for (;i<100000;++i) {
    insert(&r,setElt(i,1));
  }
  printf("100000\t%d\n",mallinfo().uordblks);

  for (;i<1000000;++i) {
    insert(&r,setElt(i,1));
  }
  printf("1000000\t%d\n",mallinfo().uordblks);

  freeRow(&r);
}

void rowTestPerf()
{
  int i;
  struct Row rA,rB;

  struct timespec timer;
  time_t start;
  long startNS;

  initRow(&rA);
  initRow(&rB);

  clock_gettime(CLOCK_MONOTONIC,&timer);
  start=timer.tv_sec;
  startNS=timer.tv_nsec;

  for (i=0;i<1000000;++i) {
    insert(&rA,setElt(i*2,1));
    insert(&rB,setElt(i*3,(i%2)+1));
  }

  clock_gettime(CLOCK_MONOTONIC,&timer);
  printf("%lf\n",(timer.tv_sec-start)+(1.0e-9)*(timer.tv_nsec-startNS));



  clock_gettime(CLOCK_MONOTONIC,&timer);
  start=timer.tv_sec;
  startNS=timer.tv_nsec;
  addRow(&rA,&rB,1);
  clock_gettime(CLOCK_MONOTONIC,&timer);
  printf("%lf\n",(timer.tv_sec-start)+(1.0e-9)*(timer.tv_nsec-startNS));

  printf("%d\n",rA.nnz);

  
  freeRow(&rA);
  freeRow(&rB);
}

#endif
