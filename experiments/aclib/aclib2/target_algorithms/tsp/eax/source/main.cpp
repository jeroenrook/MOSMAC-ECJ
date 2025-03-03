#ifndef __ENVIRONMENT__
#include "env.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <climits>


int main( int argc, char* argv[] )
{
  int maxNumOfTrial;
  
  sscanf( argv[1], "%d", &maxNumOfTrial );
  char* dstFile = argv[2];
  
  TEnvironment* gEnv = NULL;
  gEnv = new TEnvironment();
  InitURandom();  

  int d;
// hh added:
  double f;
// hh end

  sscanf( argv[3], "%d", &d );
  gEnv->fNumOfPop = d;
  sscanf( argv[4], "%d", &d );
  gEnv->fNumOfKids = d;
  gEnv->fFileNameTSP = argv[5];
// hh added / modified:
  sscanf( argv[6], "%d", &d );
  gEnv->fTargetTourLength = d;
  sscanf( argv[7], "%lf", &f );
  gEnv->fCutoffTime = f;
  sscanf( argv[8], "%d", &d );
	srand( d );

  gEnv->fFileNameInitPop = NULL;
  if( argc == 10 )
    gEnv->fFileNameInitPop = argv[9];
// hh end

  gEnv->Define();
  
  for( int n = 0; n < maxNumOfTrial; ++n )
  { 
    //jdl added next line:
    gEnv->fBestValueOverall = INT_MAX;

    gEnv->DoIt();

    gEnv->PrintOn( n, dstFile );       
    gEnv->WriteBest( dstFile );  
    // gEnv->WritePop( n, dstFile );
  }
  
  return 0;
}
