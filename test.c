
#include <stdio.h>
#include <limits.h>
#include <assert.h>
#include <stdlib.h>

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

int main()
{
  test();
  return 0;
}
