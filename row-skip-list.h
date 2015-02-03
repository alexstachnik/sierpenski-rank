
#ifndef ROW_SKIP_LIST_H_
#define ROW_SKIP_LIST_H_

#include "fastrand.h"
#include <string.h>

#define MAX_LAYERS 64

struct Row {
  struct Node *llist;
  int numLayers;
  int nnz;
  struct Node *arrStart,*arrEnd;
  struct RandState st;
};

struct Node {
  Element value;
  struct Node *next[1];
};

int randNumLayers(struct Row* row)
{
  return 1+__builtin_clzl(1|xor128(&(row->st)));
}

void makeNode(struct Row *row,
	      struct Node ***cxt,
	      Element val)
{
  int numLayers,i;
  struct Node *newNode,*oldPtr;

  row->nnz=row->nnz+1;

  numLayers = randNumLayers(row);
  row->numLayers=(row->numLayers<numLayers)?numLayers:row->numLayers;

  newNode=malloc(sizeof(struct Node)+sizeof(struct Node*)*(numLayers-1));

  newNode->value=val;

  for (i=0;i<numLayers;++i) {
    oldPtr=*(cxt[i]);
    *(cxt[i])=newNode;
    newNode->next[i]=oldPtr;
  }
}

void printRow(struct Row* row)
{
  int i;
  struct Node* node;
  for (i=row->numLayers-1;i>=0;--i) {
    node=row->llist->next[i];
    while (node != NULL) {
      printf("%d ",node->value);
      node=node->next[i];
    }
    printf("NULL\n");
  }
}

void initRow(struct Row* row)
{
  int i;
  row->llist=malloc(sizeof(struct Node)+sizeof(struct Node*)*(MAX_LAYERS-1));
  for (i=0;i<MAX_LAYERS;++i) {
    row->llist->next[i]=NULL;
  }
  
  row->numLayers=1;
  row->nnz=0;
  row->arrStart=NULL;
  row->arrEnd=NULL;

  initRand(&(row->st));
}

void findCxt(struct Node ***cxt,
	     int numLayers,
	     Position pos)
{
  int i,advanced=0;
  struct Node *next;

  for (i=numLayers-1;i>=1;--i) {
    while ((*cxt[i] != NULL) &&
	   (getPos((*cxt[i])->value) < pos)) {
      cxt[i]=&((*cxt[i])->next[i]);
      advanced=1;
    }
    if (advanced) {
      cxt[i-1]=cxt[i]-1;
    }
  }

  while ((*cxt[i] != NULL) &&
	 (getPos((*cxt[i])->value) < pos)) {
    cxt[i]=&((*cxt[i])->next[i]);
  }

}

inline
int inContigArr(struct Row* row, struct Node* node)
{
  return (node<row->arrEnd) && (node>=row->arrStart);
}

void freeRow(struct Row* row)
{
  struct Node *temp,*temp2;
  temp=row->llist;
  while (temp != NULL) {
    temp2=temp;
    temp=temp->next[0];
    if (!inContigArr(row,temp2)) {
      free(temp2);
    }
  }

  if (row->arrStart != NULL) {
    free(row->arrStart);
  }
}

int lookup(struct Row *row,
	   Position pos)
{
  struct Node **cxt[MAX_LAYERS];
  int i;
  
  for (i=0;i<row->numLayers;++i) {
    cxt[i]=&(row->llist->next[i]);
  }

  findCxt(cxt,row->numLayers,pos);
  if ((*cxt[0] != NULL) &&
      (getPos((*cxt[0])->value) == pos)) {
    return getElt((*cxt[0])->value);
  }
  return 0;
}

void insert(struct Row* row,
	    Element val)
{
  struct Node **cxt[MAX_LAYERS];
  int i;
  
  for (i=0;i<MAX_LAYERS;++i) {
    cxt[i]=&(row->llist->next[i]);
  }

  findCxt(cxt,row->numLayers,getPos(val));
  if ((*cxt[0] != NULL) &&
      (getPos((*cxt[0])->value) == getPos(val))) {
    (*cxt[0])->value=val;
  } else {
    makeNode(row,cxt,val);
  }
}

void freeNode(struct Row* row,
	      struct Node ***cxt,
	      int numLayers)
{
  int i;
  struct Node* oldNode=*cxt[0];

  row->nnz=row->nnz-1;

  for (i=0;i<numLayers;++i) {
    if (*cxt[i] == oldNode) {
      *cxt[i]=oldNode->next[i];
    }
  }

  if (!inContigArr(row,oldNode)) {
    free(oldNode);
  }
}

//rowA <- rowA + alpha*rowB
void addVecRow(struct Row* rowA,
	       Element *rowB,
	       int rowLen,
	       int alpha,
	       int *colNNZDeltas)
{
  struct Node **cxt[MAX_LAYERS];
  struct Node *bPtr;
  int i,newVal;
  
  if (alpha == 0)
    return;

  for (i=0;i<MAX_LAYERS;++i) {
    cxt[i]=&(rowA->llist->next[i]);
  }

  for (i=0;i<rowLen;++i) {
    findCxt(cxt,rowA->numLayers,getPos(rowB[i]));
    newVal=mul3(alpha,getElt(rowB[i]));
    if ((*cxt[0] != NULL) &&
        (getPos((*cxt[0])->value) == getPos(rowB[i]))) {
      newVal=sum3(getElt((*cxt[0])->value),newVal);
      if (newVal == 0) {
	freeNode(rowA,cxt,rowA->numLayers);
#pragma omp atomic
	--colNNZDeltas[i];
      } else {
	(*cxt[0])->value=setElt(getPos(rowB[i]),newVal);
      }
    } else {
      makeNode(rowA,cxt,setElt(getPos(rowB[i]),newVal));
#pragma omp atomic
      ++colNNZDeltas[i];
    }
  }
}


//rowA <- rowA + alpha*rowB
void addRow(struct Row* rowA,
	    struct Row* rowB,
	    int alpha)
{
  struct Node **cxt[MAX_LAYERS];
  struct Node *bPtr;
  int i,newVal;
  
  if (alpha == 0)
    return;

  for (i=0;i<MAX_LAYERS;++i) {
    cxt[i]=&(rowA->llist->next[i]);
  }

  for (bPtr=rowB->llist->next[0];
       bPtr != NULL;
       bPtr=bPtr->next[0]) {
    findCxt(cxt,rowA->numLayers,getPos(bPtr->value));
    newVal=mul3(alpha,getElt(bPtr->value));
    if ((*cxt[0] != NULL) &&
        (getPos((*cxt[0])->value) == getPos(bPtr->value))) {
      newVal=sum3(getElt((*cxt[0])->value),newVal);
      if (newVal == 0) {
	freeNode(rowA,cxt,rowA->numLayers);
      } else {
	(*cxt[0])->value=setElt(getPos(bPtr->value),newVal);
      }
    } else {
      makeNode(rowA,cxt,setElt(getPos(bPtr->value),newVal));
    }
  }
}

inline int detNumLayers(int nodeCount)
{
  if (nodeCount == 0)
    return MAX_LAYERS;
  return 1+__builtin_ctz(nodeCount);
}

inline void* incMemPtr(void *start, int numPtrs, int numStructs)
{
  return (void*)((char*)start+sizeof(struct Node*)*numPtrs+sizeof(struct Node)*numStructs);
}

void* copyNode(struct Node* node,
	       int numLayers,
	       void *memStart,
	       void *memEnd)
{
  size_t ptrJump;
  void *nodeEnd=incMemPtr(memStart,numLayers-1,1);
  struct Node *newNode=memStart;
  int i;

  newNode->value=node->value;

  ptrJump=0;
  for (i=0;i<numLayers;++i) {
    newNode->next[i]=incMemPtr(nodeEnd,ptrJump,(1<<i)-1);
    if ((void*)(newNode->next[i]) >= memEnd) {
      newNode->next[i]=NULL;
    }
    ptrJump += (1<<i)-1;
  }

  return nodeEnd;
}

void bunchRow(struct Row *row)
{
  struct Node *node, *newLList;
  void *memStart, *memEnd;
  size_t size;
  int i,numLayers;
  
  size=0;
  numLayers=1;
  for (i=row->nnz>>1;i>0;i=i>>1) {
    size+=i;
    ++numLayers;
  }
  size=size*sizeof(struct Node*);
  size+=sizeof(struct Node)*(row->nnz+1)+
    sizeof(struct Node*)*(MAX_LAYERS-1);

  memStart=malloc(size);
  newLList=memStart;
  memEnd=(void*)((char*)memStart+size);

  node=row->llist;
  i=0;
  while (node != NULL) {
    memStart=copyNode(node,detNumLayers(i),memStart,memEnd);
    ++i;
    node=node->next[0];
  }

  freeRow(row);
  row->arrStart=newLList;
  row->llist=newLList;
  row->arrEnd=memEnd;
  row->numLayers=numLayers;
}

#endif

