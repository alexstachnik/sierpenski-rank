
#ifndef DIAGONALCALC_H_
#define DIAGONALCALC_H_

#include "GF3.h"

struct MemoData {
  int *factList;
  int *powerList;
};

int reduceZeros(int n)
{
  while ((n>0)&&(n%3==0)) {
    n/=3;
  }
  return n;
}

void initFactorials(int n,
		    int *factList)
{
  int i,d;
  factList[0]=1;
  for (i=1;i<n;++i) {
    d=reduceZeros(i);
    if (d==0) {
      factList[i]=factList[i-1];
    } else {
      factList[i]=mul3(factList[i-1],d%3);
    }
  }
}

void initPowerList(int n,
		   int *powerList)
{
  int i,k,sum;
  for (i=0;i<n;++i) {
    powerList[i]=0;
  }
  for (k=3;k<n;k*=3) {
    for (i=k;i<n;i+=k) {
      ++powerList[i];
    }
  }
  sum=0;
  for (i=0;i<n;++i) {
    sum+=powerList[i];
    powerList[i]=sum;
  }
}

void setupMemoData(int tblSize,
                   struct MemoData *memoData)
{
  memoData->factList=malloc(sizeof(int)*tblSize);
  memoData->powerList=malloc(sizeof(int)*tblSize);

  initFactorials(tblSize,memoData->factList);
  initPowerList(tblSize,memoData->powerList);
}

void freeMemoData(struct MemoData *memoData)
{
  free(memoData->factList);
  free(memoData->powerList);
} 

int nChooseK(int n,
	     int k,
	     struct MemoData memoData)
{
  int *powerList=memoData.powerList;
  int *factList=memoData.factList;
  if (powerList[n] > (powerList[k] + powerList[n-k]))
    return 0;
  assert(powerList[n] == (powerList[k] + powerList[n-k]));
  return div3(factList[n],mul3(factList[k],factList[n-k]));
}

void fillRows(struct Row *rows,
	      int numPolyCoeffs,
	      Element *polyCoeffs,
	      int startRow,
	      int endRow)
{
  int maxN,i,j,k,d;
  struct MemoData memoData;
  
  maxN=2+getPos(polyCoeffs[numPolyCoeffs-1]);
  setupMemoData(maxN,&memoData);

  for (k=0;k<numPolyCoeffs;++k) {
    for (i=startRow;i<endRow;++i) {
      j=getPos(polyCoeffs[k])-i;
      if (j<0)
	break;
      d=nChooseK(getPos(polyCoeffs[k]),i,memoData);
      if (d == 0) {
	continue;
      }
      d=mul3(getElt(polyCoeffs[k]),d);
      insert(&rows[i-startRow],setElt(j,d));
    }
  }

  for (i=0;i<endRow-startRow;++i) {
    bunchRow(&rows[i]);
  }
  
}

#endif
