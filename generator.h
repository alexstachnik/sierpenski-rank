
#ifndef _GENERATOR_H_
#define _GENERATOR_H_

struct MemoData {
  int *factList;
  int *powerList;
};

int reduceZeros(int x)
{
  while ((x>0)&&(x%3==0)) {
    x/=3;
  }
  return x;
}

void initFactorials(int tblSize,
                    int* factList)
{
  int i,d;
  factList[0]=1;
  for (i=1;i<tblSize;++i) {
    d=reduceZeros(i);
    if (d==0) {
      factList[i]=factList[i-1];
    } else {
      factList[i]=mul3(factList[i-1],d%3);
    }
  }
}

void initPowerList(int tblSize,
                   int* powerList)
{
  int i,k,sum;
  for (i=0;i<tblSize;++i) {
    powerList[i]=0;
  }

  for (k=3;k<tblSize;k*=3) {
    for (i=k;i<tblSize;i+=k) {
      ++powerList[i];
    }
  }

  sum=0;
  for (i=0;i<tblSize;++i) {
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
  int denomPower=memoData.powerList[k]+memoData.powerList[n-k];
  int numerPower=memoData.powerList[n];
  if (numerPower>denomPower) {
    return 0;
  } else {
    assert(numerPower==denomPower);
    return div3(memoData.factList[n],
                mul3(memoData.factList[n-k],memoData.factList[k]));
  }
}

void generateMat(int startRow,
                 int endRow,
                 int dim,
                 Element *polyCoeffs,
                 int numPolyCoeffs)
{
  int i,row,col;
  for (i=startRow;i<endRow+dim-1;++i) {
    for (row=startRow;(row<endRow)&&(i>=row);++row) {
      col=i-row;
    }
  }
}

#endif

