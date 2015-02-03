
#ifndef _FASTRAND_H_
#define _FASTRAND_H_

struct RandState {
  uint64_t x,y,z,w;
};

void initRand(struct RandState *s)
{
  s->x=13775414763907882882U;
  s->y=1515034416189689256U;
  s->z=5409247145147938665U;
  s->w=14482447969147086210U;
}

uint64_t xor128(struct RandState *s) {
  uint64_t t;
  t=(s->x^(s->x<<11));
  s->x=s->y;
  s->y=s->z;
  s->z=s->w;
  return( s->w=(s->w^(s->w>>19))^(t^(t>>8)) );
}

#endif
