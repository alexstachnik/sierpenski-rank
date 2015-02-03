
struct Row {
  int nnz;
  int size;
  Element *data;
};

void initRow(struct Row* row)
{
  row->nnz=0;
  row->size=1;
  row->data=malloc(sizeof(Element));
}

void freeRow(struct Row* row)
{
  free(row->data);
}

void shrinkRow(struct Row* row)
{
  int i;
  Element *newDat=malloc(sizeof(Element)*row->nnz);
  for (i=0;i<row->nnz;++i) {
    newDat[i]=row->data[i];
  }
  free(row->data);
  row->data=newDat;
  row->size=row->nnz;
}

void pushBackElt(struct Row* row,
                 Element d)
{
  if (row->nnz>=row->size) {
    row->size=(row->nnz+1)*2;
    row->data=realloc(row->data,row->size*sizeof(Element));
  }
  row->data[row->nnz]=d;
  ++(row->nnz);
}

int lookupElt(struct Row* row,
              Position d)
{
  uint32_t len = row->nnz;

  if (len <= 1) {
    if (len == 0) {
      return 0;
    }
    if (getPos(row->data[0]) == d) {
      return getElt(row->data[0]);
    }
    return 0;
  }

  uint32_t b=1<<(31-__builtin_clz(len-1));
  uint32_t mask=0;
  while (b != 0) {
    uint32_t k=mask|b;
    if ((k<len)&&(getPos(row->data[k]) <= j)) {
      mask=k;
    }
    b=b>>1;
  }
  if (getPos(row->data[mask]) == j) {
    return getElt(row->data[mask]);
  } else {
    return 0;
  }
}

void addRow(struct Row* sum,
            struct Row* addend,
            int d)
{
  int i=0,j=0,k=0,lenA=sum->nnz,lenB=addend->nnz;
  Element *newRow;
  if (d==0)
    return;

  newRow=malloc(sizeof(Element)*(sum->nnz+addend->nnz));
  while ((i<lenA)&&(j<lenB)) {
    if (getPos(sum->data[i])<getPos(addend->data[j])) {
      newRow[k]=sum->data[i];
      ++i;
      ++k;
    } else if (getPos(sum->data[i])>getPos(addend->data[j])) {
      {
        Position pos=getPos(addend->data[j]);
        int val=mul3(d,getVal(addend->data[j]));
        newRow[k]=setElt(pos,val);
        incrNNZDelta(pos,1);
      }
      ++j;
      ++k;
    } else {
      {
        Position pos=getPos(addend->data[j]);
        int val=mul3(d,getVal(addend->data[j]));
        val=add3(val,getVal(sum->data[j]));
        if (val != 0) {
          newRow[k]=setElt(pos,val);
          ++k;
        } else {
          incrNNZDelta(pos,-1);
        }
        ++i;
        ++j;
      }
    }
  }

  for (;i<lenA;++i) {
    newRow[k]=sum->data[i];
    ++k;
  }
  for (;j<lenB;++j) {
    {
      Position pos=getPos(addend->data[j]);
      int val=mul3(d,getVal(addend->data[j]));
      newRow[k]=setElt(pos,val);
      incrNNZDelta(pos,1);
    }
    ++k;
  }

  free(sum->data);
  sum->data=newRow;
  sum->nnz=k;
  shrinkRow(sum);
}
