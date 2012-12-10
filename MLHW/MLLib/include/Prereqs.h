
#ifndef __Prereqs_h__
#define __Prereqs_h__

#include <iostream>
#include <algorithm>
#include <list>
#include <map>
#include <set>
#include <string>
#include <vector>

#include <time.h>

#define INIT_RAND() (srand( time(NULL) ))
#define COIN_TOSS() (rand()%2)
#define RAND(l,u) ( (l) + rand()%(abs((u)-(l))) )
#define RANDF(l,u) ( (l) + ((float)rand()/(float)RAND_MAX)*(u-l) )

#define IS_ZERO(x) ( fabs(x) < 0.000001 )
#define IS_ZEROE(x,eps) ( fabs(x) < (eps) )

#ifdef _DEBUG
	#include <assert.h>
	#define mlAssert assert
#else
	#define mlAssert(x)
#endif

#endif // __Prereqs_h__
