#ifndef __ENVIRONMENT__
#include "env.h"
#endif

#include <math.h> 

     
void MakeRandSol( TEvaluator* eval , TIndi& indi );
void Make2optSol( TEvaluator* eval , TIndi& indi );

TEnvironment::TEnvironment()
{
  fEvaluator = new TEvaluator();
}


TEnvironment::~TEnvironment()
{
  delete [] fIndexForMating;
  delete [] tCurPop;
  delete fEvaluator;
  delete tCross;

  int N = fEvaluator->Ncity;
  for( int i = 0; i < N; ++i ) 
    delete [] fEdgeFreq[ i ];
  delete [] fEdgeFreq;
}


void TEnvironment::Define()
{
  fEvaluator->SetInstance( fFileNameTSP );
  int N = fEvaluator->Ncity;

  fIndexForMating = new int [ fNumOfPop + 1 ];  

  tCurPop = new TIndi [ fNumOfPop ];
  for ( int i = 0; i < fNumOfPop; ++i )
    tCurPop[i].Define( N );

  tBest.Define( N );

  tCross = new TCross( N );
  tCross->eval = fEvaluator;                 
  tCross->fNumOfPop = fNumOfPop;             

  tKopt = new TKopt( N );
  tKopt->eval = fEvaluator;
  tKopt->SetInvNearList();

  fEdgeFreq = new int* [ N ]; 
  for( int i = 0; i < N; ++i ) 
    fEdgeFreq[ i ] = new int [ N ]; 
}


void TEnvironment::DoIt()
{
	
// end hh added

  this->fTimeStart = clock();   

  if( fFileNameInitPop == NULL )
    this->InitPop();                       
  else
    this->ReadPop( fFileNameInitPop );     

  this->fTimeInit = clock();    

// hh changed (added restart mechanism):
	noFurtherRestarts = 0;

	while (1) {
          // end hh changed

          this->Init();
          this->GetEdgeFreq();

          while( 1 )
            {
              this->SetAverageBest();
              // printf( "%d: %d %lf\n", fCurNumOfGen, fBestValue, fAverageValue );

              if( this->TerminationCondition() ) break;

              this->SelectForMating();

              for( int s =0; s < fNumOfPop; ++s )
                {
                  this->GenerateKids( s );     
                  this->SelectForSurvival( s ); 
                }
              ++fCurNumOfGen;
            }

          // hh changed: control restart loop
          if (this->noFurtherRestarts == 0) {
            printf ("hh: restarting, best_in_restart %d avg_pop %lf time_to_global_best = %lf.. \n",fBestValue, fAverageValue, ((double)(clock() - this->fTimeStart)/(double)CLOCKS_PER_SEC));
            this->InitPop();                       
	
          }
          else break;

        } // hh: end restart loop

  this->fTimeEnd = clock();   
}
 

void TEnvironment::Init()
{
  fAccumurateNumCh = 0;
  fCurNumOfGen = 0;
  fStagBest = 0;
  fMaxStagBest = 0;
  fStage = 1;          /* Stage I */
  fFlagC[ 0 ] = 4;     /* Diversity preservation: 1:Greedy, 2:--- , 3:Distance, 4:Entropy (see Section 4) */
  fFlagC[ 1 ] = 1;     /* Eset Type: 1:Single-AB, 2:Block2 (see Section 3) */ 
} 


bool TEnvironment::TerminationCondition()
{
  bool returnCode = false;

// hh added:
//  printf ("hh: %d %d\n", fBestValue, fTargetTourLength);
	if ( fTargetTourLength > 0 && fBestValue <= fTargetTourLength) {
          printf ("jdl: stopping because best value found (%d) equal to optimum (%d)\n", fBestValue, fTargetTourLength);
	  this->noFurtherRestarts = 1;
          returnCode = true;
        }
        // jdl: Once we're sure everything works as expected, we can comment next line:
        if (((double)(clock() - this->fTimeStart)/(double)CLOCKS_PER_SEC) > fCutoffTime){
          printf ("jdl: stopping because current time (%lf) larger than cut-off time (%lf)\n", ((double)(clock() - this->fTimeStart)/(double)CLOCKS_PER_SEC), fCutoffTime);
	  this->noFurtherRestarts = 1;
          returnCode = true;
        }
// hh end
		 
  if ( fAverageValue - fBestValue < 0.001 )
    returnCode = true;
  
  // jdl added this code block, and changed the 3 "return true" above into a variable assignment
  if ( returnCode ) {
    timeLastRunTook = (double)(clock() - this->fTimeStart)/(double)CLOCKS_PER_SEC;    
    return true;
  }

  if( fStage == 1 ) // Stage I
  {
    if( fStagBest == int(1500/fNumOfKids) && fMaxStagBest == 0 ){ // 1500/N_ch (See Section 2.2) 
      fMaxStagBest =int( fCurNumOfGen / 10 );                 // fMaxStagBest = G/10 (See Section 2.2) 
    } 
    else if( fMaxStagBest != 0 && fMaxStagBest <= fStagBest ){ // Terminate Stage I (proceed to Stage II) 
      fStagBest = 0;
      fMaxStagBest = 0;
      fCurNumOfGen1 = fCurNumOfGen;
      fFlagC[ 1 ] = 2; 
      fStage = 2;     
    }
    return false;
  }

  if( fStage == 2 ){ // Stage II 
    if( fStagBest == int(1500/fNumOfKids) && fMaxStagBest == 0 ){ // 1500/N_ch 
      fMaxStagBest = int( (fCurNumOfGen - fCurNumOfGen1) / 10 ); // fMaxStagBest = G/10 (See Section 2.2) 
    } 
    else if( fMaxStagBest != 0 && fMaxStagBest <= fStagBest ){ // Terminate Stage II and GA
      return true;
    }

    return false;
  }
}


void TEnvironment::SetAverageBest() 
{
  int stockBest = tBest.fEvaluationValue;
  
  fAverageValue = 0.0;
  fBestIndex = 0;
  fBestValue = tCurPop[0].fEvaluationValue;
  
  // ts: Added the check as the best solution in the population may be the first one (index 0); otherwise wrong report in screen output
  if (fBestValue < fBestValueOverall) {
    fBestValueOverall = fBestValue;
    timeToFindBestSol = (double)(clock() - this->fTimeStart)/(double)CLOCKS_PER_SEC;    
  }
  
  for(int i = 0; i < fNumOfPop; ++i ){
    fAverageValue += tCurPop[i].fEvaluationValue;
    if( tCurPop[i].fEvaluationValue < fBestValue ){
      fBestIndex = i;
      fBestValue = tCurPop[i].fEvaluationValue;
      // jdl added 2 lines below
      if (fBestValue < fBestValueOverall) {
        fBestValueOverall = fBestValue;
        timeToFindBestSol = (double)(clock() - this->fTimeStart)/(double)CLOCKS_PER_SEC;    
      }
    }
  }
  
  tBest = tCurPop[ fBestIndex ];
  fAverageValue /= (double)fNumOfPop;

  if( tBest.fEvaluationValue < stockBest ){
    fStagBest = 0;
    fBestNumOfGen = fCurNumOfGen;
    fBestAccumeratedNumCh = fAccumurateNumCh;
  }
  else ++fStagBest;
}


void TEnvironment::InitPop()
{
  for ( int i = 0; i < fNumOfPop; ++i ){ 
    tKopt->MakeRandSol( tCurPop[ i ] );    /* Make a random tour */
    tKopt->DoIt( tCurPop[ i ] );           /* Apply the local search with the 2-opt neighborhood */ 
  }
}


void TEnvironment::SelectForMating()
{
  /* fIndexForMating[] <-- a random permutation of 0, ..., fNumOfPop-1 */
  tRand->Permutation( fIndexForMating, fNumOfPop, fNumOfPop ); 
  fIndexForMating[ fNumOfPop ] = fIndexForMating[ 0 ];
}

void TEnvironment::SelectForSurvival( int s )
{
}


void TEnvironment::GenerateKids( int s )
{
  tCross->SetParents( tCurPop[fIndexForMating[s]], tCurPop[fIndexForMating[s+1]], fFlagC, fNumOfKids );  
  
  /* Note: tCurPop[fIndexForMating[s]] is replaced with a best offspring solutions in tCorss->DoIt(). 
     fEegeFreq[][] is also updated there. */
  tCross->DoIt( tCurPop[fIndexForMating[s]], tCurPop[fIndexForMating[s+1]], fNumOfKids, 1, fFlagC, fEdgeFreq );

  fAccumurateNumCh += tCross->fNumOfGeneratedCh;
}


void TEnvironment::GetEdgeFreq()
{
  int N = fEvaluator->Ncity;
  int k0, k1;
  
  for( int j1 = 0; j1 < N; ++j1 )
    for( int j2 = 0; j2 < N; ++j2 ) 
      fEdgeFreq[ j1 ][ j2 ] = 0;

  
  for( int i = 0; i < fNumOfPop; ++i )
  {
    for(int j = 0; j < N; ++j )
    {
      k0 = tCurPop[ i ].fLink[ j ][ 0 ];
      k1 = tCurPop[ i ].fLink[ j ][ 1 ];
      ++fEdgeFreq[ j ][ k0 ];
      ++fEdgeFreq[ j ][ k1 ];
    }
  }
}


void TEnvironment::PrintOn( int n, char* dstFile ) 
{  
  // jdl modified printf here to get more clear info, and only what we need
  printf( "\n# END OF RUN %d: best sol = %d, time to find best sol = %lf, time now = %lf\n\n",
          n,
          fBestValueOverall,
          timeToFindBestSol,
          timeLastRunTook);
  fflush(stdout);


  FILE *fp;
  char filename[ 80 ];
  sprintf( filename, "%s_Result", dstFile );
  fp = fopen( filename, "a");
  
  fprintf( fp, "%d %d %d %d %d\n" , 
	   n, 
	   tBest.fEvaluationValue, 
	   fCurNumOfGen, 
	   (int)((double)(this->fTimeInit - this->fTimeStart)/(double)CLOCKS_PER_SEC), 
	   (int)((double)(this->fTimeEnd - this->fTimeStart)/(double)CLOCKS_PER_SEC) );
  
  fclose( fp );
}


void TEnvironment::WriteBest( char* dstFile ) 
{
  FILE *fp;
  char filename[ 80 ];
  sprintf( filename, "%s_BestSol", dstFile );
  fp = fopen( filename, "a");
  
  fEvaluator->WriteTo( fp, tBest );

  fclose( fp );
}


void TEnvironment::WritePop( int n, char* dstFile ) 
{
  FILE *fp;
  char filename[ 80 ];
  sprintf( filename, "%s_POP_%d", dstFile, n );
  fp = fopen( filename, "w");

  for( int s = 0; s < fNumOfPop; ++s )
    fEvaluator->WriteTo( fp, tCurPop[ s ] );

  fclose( fp );
}


void TEnvironment::ReadPop( char* fileName )
{
  FILE* fp;

  if( ( fp = fopen( fileName, "r" ) ) == NULL ){
    printf( "Read Error1\n"); 
    fflush( stdout );
    exit( 1 );
  }

  for ( int i = 0; i < fNumOfPop; ++i ){ 
    if( fEvaluator->ReadFrom( fp, tCurPop[ i ] ) == false ){
      printf( "Read Error2\n"); 
      fflush( stdout );
      exit( 1 );
    }
  }
  fclose( fp );
}


