
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>
#include <assert.h>
#include <stdlib.h>
//#include <mpi.h>
#include "mpiStubs.h"

#include <time.h>

int g_np;
int g_r;

#include "util.h"
#include "GF3.h"
#include "polycoeffs.h"
#include "densityestimator.h"
#include "row-skip-list.h"
#include "diagonalcalc.h"
#include "pivots.h"
#include "testsuite.h"

void MPI_InitSharedColNNZs(int *colNNZs,
			   int width)
{
  MPI_Allreduce(MPI_IN_PLACE,colNNZs,width,MPI_INT,MPI_SUM,MPI_COMM_WORLD);
}

void MPI_ChoosePivot(struct Pivot *myPivot,
		     struct Pivot *pivots)
{
  int i;
  uint64_t maxCost=-1;

  MPI_Allgather(myPivot,sizeof(struct Pivot),MPI_BYTE,
		pivots,sizeof(struct Pivot),MPI_BYTE,
		MPI_COMM_WORLD);

  for (i=0;i<g_np;++i) {
    if (pivots[i].cost < maxCost) {
      *myPivot=pivots[i];
    }
  }
}

void MPI_DistributeRow(struct Pivot pivot,
		       struct Row *rows,
		       Element *pivotRow)
{
  struct Node *node;
  int i;

  if (pivot.proc == g_r) {
    node=rows[pivot.row].llist->next[0];
    for (i=0;i<pivot.nnz;++i) {
      pivotRow[i]=node->value;
      node=node->next[0];
    }
  }


  MPI_Bcast(pivotRow,pivot.nnz,MPI_INT,
	    pivot.proc,MPI_COMM_WORLD);
}

void eliminateCol(Element pivot,
		  Element *pivotRow,
		  int rowLen,
		  struct Row* rows,
		  int numRows,
		  int *colNNZDeltas)
{
  int i,d,alpha;
#pragma omp parallel for schedule(dynamic,20)
  for (i=0;i<numRows;++i) {
    d=lookup(&rows[i],getPos(pivot));
    if (d!=0) {
      alpha=neg3(div3(d,getElt(pivot)));
      addVecRow(&rows[i],pivotRow,rowLen,alpha,colNNZDeltas);
    }
  }
}

void writeRows(struct Row* rows,
	       int numRows,
	       int width)
{
  int i;
  char filename[255];
  FILE *fout;
  sprintf(filename,"checkpoints/chkpt%d.dat",g_r);
  fout=open(filename,O_CREAT|O_WRONLY|O_TRUNC,S_IRWXU);
  assert(fout != -1);

  write(fout,&numRows,sizeof(int));
  write(fout,&width,sizeof(int));

  for (i=0;i<numRows;++i) {
    bunchRow(&rows[i]);
    write(fout,&rows[i],sizeof(struct Row));
    write(fout,rows[i].arrStart,rows[i].arrEnd-rows[i].arrEnd);
  }

  close(fout);
}

void readRows(struct Row** rowPtr, int *numRowsOut, int *width)
{
  int i,numRows;
  char filename[255];
  size_t rowSize;
  FILE *fout;
  struct Row *rows;
  sprintf(filename,"checkpoints/chkpt%d.dat",g_r);
  fout=open(filename,O_RDONLY);
  assert(fout != -1);

  read(fout,&numRows,sizeof(int));
  read(fout,width,sizeof(int));

  rows=malloc(sizeof(struct Row)*numRows);
  *rowPtr=rows;
  *numRowsOut=numRows;

  for (i=0;i<numRows;++i) {
    read(fout,&rows[i],sizeof(struct Row));
    rowSize=rows[i].arrEnd-rows[i].arrStart;
    rows[i].llist=malloc(rowSize);
    rows[i].arrStart=rows[i].llist;
    rows[i].arrEnd=rows[i].arrStart+rowSize;
    read(fout,rows[i].llist,rowSize);
  }

  close(fout);
}

void MPI_UpdateColNNZs(Element *pivotRow,
		       int *colNNZDeltas,
		       int rowLen,
		       int *colNNZs)
{
  int i;
  MPI_Allreduce(MPI_IN_PLACE,colNNZDeltas,rowLen,MPI_INT,MPI_SUM,MPI_COMM_WORLD);
  for (i=0;i<rowLen;++i) {
    colNNZs[getPos(pivotRow[i])]+=colNNZDeltas[i];
  }
}

#define TIME_START //clock_gettime(CLOCK_MONOTONIC,&timer);start=timer.tv_sec;startNS=timer.tv_nsec;
#define TIME_END(x) //clock_gettime(CLOCK_MONOTONIC,&timer);if (rank>2000) {x+=(timer.tv_sec-start)+(1.0e-9)*(timer.tv_nsec-startNS);}

int mainLoop(struct Row *rows,
	     int numRows,
	     int width)
{
  int *colNNZs;
  int rank=0;
  struct Pivot *pivots;
  struct Pivot myPivot;

  struct timespec timer;
  time_t start;
  long startNS;
  double p1=0.0,p2=0.0,p3=0.0,p4=0.0,p5=0.0;

  Element *pivotRow;
  int *colNNZDeltas;

  colNNZs=malloc(sizeof(int)*width);
  initLocalColNNZs(colNNZs,rows,width,numRows);
  MPI_InitSharedColNNZs(colNNZs,width);
  pivots=malloc(sizeof(struct Pivot)*g_np);
  printf("%d %d\n",g_r,mallinfo().uordblks);  
  while (1) {

    if (g_r ==0)
      printf("%d\n",rank);
    
    TIME_START;
    findPivot(colNNZs,rows,numRows,&myPivot);
    TIME_END(p1);

    TIME_START;
    MPI_ChoosePivot(&myPivot,pivots);
    TIME_END(p2);

    if (myPivot.cost == -1) {
      break;
    }
    pivotRow=malloc(sizeof(Element)*myPivot.nnz);
    colNNZDeltas=malloc(sizeof(int)*myPivot.nnz);

    TIME_START;
    MPI_DistributeRow(myPivot,rows,pivotRow);
    TIME_END(p3);

    TIME_START;
    eliminateCol(myPivot.elt,pivotRow,myPivot.nnz,rows,numRows,colNNZDeltas);
    TIME_END(p4);

    TIME_START;
    MPI_UpdateColNNZs(pivotRow,colNNZDeltas,myPivot.nnz,colNNZs);
    TIME_END(p5);

    free(pivotRow);
    free(colNNZDeltas);
    ++rank;
    if (rank % 200 == 0 && rank > 0) {
      writeRows(rows,numRows,width);
    }

  }

  if (g_r==0) {
    printf("%d %lf %lf %lf %lf %lf %lf\n",g_r,p1,p2,p3,p4,p5,p1+p2+p3+p4+p5);
  }
  MPI_Barrier(MPI_COMM_WORLD);
  if (g_r==1) {
    printf("%d %lf %lf %lf %lf %lf %lf\n",g_r,p1,p2,p3,p4,p5,p1+p2+p3+p4+p5);
  }
  MPI_Barrier(MPI_COMM_WORLD);
  if (g_r==2) {
    printf("%d %lf %lf %lf %lf %lf %lf\n",g_r,p1,p2,p3,p4,p5,p1+p2+p3+p4+p5);
  }
  MPI_Barrier(MPI_COMM_WORLD);


  free(colNNZs);
  free(pivots);
  return rank;
}


int main(int argc, char** argv)
{
  int i,numPolyCoeffs,startRow,endRow,numRows,width,rank;
  Element *polyCoeffs;
  struct Row *rows;

  MPI_Init(&argc,&argv);
  MPI_Comm_rank(MPI_COMM_WORLD,&g_r);
  MPI_Comm_size(MPI_COMM_WORLD,&g_np);

  assert(sizeof(uint32_t)==4);
  assert(sizeof(uint64_t)==8);
  assert(argc == 2);
  test();

  if (strcmp(argv[1],"-")==0) {
    readRows(&rows,&numRows,&width);
    mainLoop(rows,numRows,width);
  } else {
    numPolyCoeffs=readPoly(argv[1],&polyCoeffs);
    width=1+getPos(polyCoeffs[numPolyCoeffs-1]);
    computeDensity(width,g_np,g_r,&startRow,&endRow);
    //printf("r, start, end: %d %d %d\n",g_r,startRow,endRow);
    
    numRows=endRow-startRow;
    rows=malloc(sizeof(struct Row)*numRows);
    for (i=0;i<numRows;++i) 
      initRow(&rows[i]);
    fillRows(rows,numPolyCoeffs,polyCoeffs,startRow,endRow);
    free(polyCoeffs);
    
    rank=mainLoop(rows,numRows,width);
  }
  if (g_r == 0) {
    printf("Rank: %d\n", rank);
  }

  MPI_Finalize();
  return 0;
}
