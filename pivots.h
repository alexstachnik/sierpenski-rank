
#ifndef PIVOTS_H_
#define PIVOTS_H_

struct Pivot {
  int proc;
  int row;
  int nnz;
  Element elt;
  uint64_t cost;
};

void initLocalColNNZs(int *colNNZs,
		      struct Row *rows,
		      int width,
		      int numRows)
{
  int i;
  struct Node *node;
  for (i=0;i<width;++i) {
    colNNZs[i]=0;
  }
  for (i=0;i<numRows;++i) {
    node=rows[i].llist->next[0];
    while (node != NULL) {
      ++colNNZs[getPos(node->value)];
      node=node->next[0];
    }
  }
}

void findPivot(int *colNNZs,
	       struct Row *rows,
	       int numRows,
	       struct Pivot *pivot)
{
  int i;
  Position pos;
  struct Node *node;
  uint64_t maxCost=-1,cost;

  pivot->cost=maxCost;
  pivot->proc=g_r;
  for (i=0;i<numRows;++i) {
    node=rows[i].llist->next[0];
    while (node != NULL) {
      pos=getPos(node->value);
      cost=((uint64_t)colNNZs[pos]-1)*((uint64_t)rows[i].nnz-1);
      if (cost<maxCost) {
	maxCost=cost;
	pivot->cost=cost;
	pivot->elt=node->value;
	pivot->row=i;
	pivot->nnz=rows[i].nnz;
      }
      node=node->next[0];
    }
    if (maxCost == 0) {
      break;
    }
  }
}

#endif
