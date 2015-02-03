
#include <stdio.h>
#include "diagonalcalc.h"

int main()
{
  int i,j;
  int factList[50];
  int powerList[50];
  initFactorials(50,factList);
  initPowerList(50,powerList);
  for (i=0;i<10;++i) {
    for (j=0;j<=i;++j) {
      printf("%d ",nChooseK(i,j,factList,powerList));
    }
    printf("\n");
  }
  return 0;
}
