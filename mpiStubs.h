#define MPI_IN_PLACE 0
#define MPI_INT 0
#define MPI_BYTE 0
#define MPI_SUM 0
#define MPI_COMM_WORLD 0

void MPI_Allreduce(int inPlace,int *colNNZs,int width,int type,int op,int comm)
{
  assert(inPlace==MPI_IN_PLACE);
  assert(op==MPI_SUM);
}


void MPI_Allgather(void *sendBuf,size_t countSend,int sendType,void *recvBuf,size_t countRecv,int recvType,int comm)
{
  int i;
  assert(recvType==MPI_BYTE);
  assert(sendType==MPI_BYTE);
  for (i=0;i<countSend;++i) {
    ((char*)recvBuf)[i]=((char*)sendBuf)[i];
  }
}

void MPI_Bcast(void *sendBuf,int len,int sendType,int sendProc,int comm)
{
  assert(sendProc==0);
}

void MPI_Init(int *argc,char ***argv)
{}

void MPI_Comm_rank(int comm,int *r)
{
  *r=0;
}

void MPI_Comm_size(int comm,int *np)
{
  *np=1;
}

void MPI_Barrier(int comm)
{
}

void MPI_Finalize()
{
}

