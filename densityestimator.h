
#ifndef _DENSITYESTIMATOR_H_
#define _DENSITYESTIMATOR_H_

void fillDensity(double* arr,
                 size_t start,
                 size_t end,
                 double fill)
{
  const double C1=0.6315,C2=0.2894,C3=0.0791;

  if (start==end) {
    arr[start]=fill;
  } else {
    size_t subRangeSize=(1+end-start)/3;
    fillDensity(arr,start,start+subRangeSize-1,fill*C1);
    fillDensity(arr,start+subRangeSize,start+subRangeSize*2-1,fill*C2);
    fillDensity(arr,start+subRangeSize*2,start+3*subRangeSize-1,fill*C3);
  }
}

void computeDensity(int numRows,
                    int numProcs,
                    int myProc,
                    int *myStart,
                    int *myEnd)
{
  int i,chunk;

  int arrSize=roundUpPowerOfThree(numRows);
  double *arr=malloc(sizeof(double)*arrSize);
  double chunkSize=0.0,minChunkSize=1.0/((double)numProcs);

  fillDensity(arr,0,arrSize-1,1.0);

  *myStart=0;
  *myEnd=arrSize;
  chunk=0;
  for (i=0;i<arrSize;++i) {
    chunkSize+=arr[i];
    if (chunkSize>minChunkSize) {
      if (chunk == myProc) {
        *myEnd=i+1;
        break;
      }
      ++chunk;
      chunkSize=0;
      *myStart=i+1;
    }
  }

  *myStart=min(*myStart,numRows);
  *myEnd=min(*myEnd,numRows);

  assert((myProc != (numProcs-1)) || (*myEnd == numRows));

  free(arr);
}

#endif
