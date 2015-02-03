
#ifndef POLYCOEFFS_H_
#define POLYCOEFFS_H_

int coeffFileLen(char* filename)
{
  int len=0,n;
  FILE* file;
  file=fopen(filename,"r");
  assert(file != NULL);
  while (fscanf(file,"%d",&n) != EOF) {
    ++len;
  }
  fclose(file);
  return len;
}

int readPoly(char* filename, Element** poly)
{
  int pos,negative,len,i,junk;
  FILE* file;

  len=coeffFileLen(filename);
  (*poly)=malloc(sizeof(Element)*len);

  file=fopen(filename,"r");
  assert(file != NULL);

  for (i=0;i<len;++i) {
    negative=0;
    junk=fscanf(file," -%n ",&negative);

    if (fscanf(file,"%d",&pos) == EOF) {
      assert(("premature EOF",0));
    } else {
      (*poly)[i]=setElt(pos,negative?2:1);
    }
  }

  fclose(file);
  return len;
}

#endif

