#!/bin/bash
# SparrowToRiss, Norbert Manthey, Adrian Balint 2014
# solve CNF formula $1 by simplifying first with coprocessor, then run Sparrow and then run Riss 4.27 on $1 in case the formula was not solved
#
#  USAGE: ./SparrowToRiss.sh <input.cnf> <seed> <tmpDir> <FLIPS> <ScpuSeconds> <sparrowParams> <rissParams>
#
#  Special control with FLIPS: -2 => CPsparrow, -1 => Sparrow, 0 => Riss427, >0 => SparrowToRiss
#
#  Note: <sparrowParams> has to be one argument (wrapped in "")
#  Note: <rissParams> has to be one argument (wrapped in "")
#
#

binaryPath=solvers/SparrowToRiss/

#
# usage
#
if [ "x$1" = "x" -o "x$2" = "x"  -o "x$3" = "x" -o "x$4" = "x" -o "x$5" = "x" -o "x$6" = "x" ]; then
  echo "USAGE: ./SparrowToRiss.sh <input.cnf> <seed> <tmpDir> <FLIPS> <ScpuSeconds> <sparrowParams> <rissParams>"
  exit 1
fi

echo "c"
echo "c SparrowToRiss CSSC 2014"
echo "c Adrian Balint, Norbert Manthey"
echo "c"


#
# check if the file in the first parameter exists
#
if [ ! -f $1 ]
then
  # if the input file does not exists, then abort nicely with a message
  echo "c the file does not exist: $1"
  echo "s UNKNOWN"
  exit 0
fi

#
# variables for the script
#

file=$1						  # first argument is CNF instance to be solved
seed=$2 					  # seed for sparrow
tmpDir=$3 				  # directory for temporary files
sparrowFlips=$4     # number of flips
sparrowSeconds=$5   # number of CPU seconds for sparrow. good default: 120  (is turned into --timeout 120)
sparrowParams=$6    # parameters for sparrow. good default: "-l -k -r1 "
rissParams=$7       # parameters for riss. good default: -R=1.2 -szLBDQueue=60 -szTrailQueue=4000 -lbdIgnL0 -quickRed -keepWorst=0.001 -var-decay-b=0.85 -var-decay-e=0.99 -var-decay-d=10000 -rnd-freq=0.005 -init-act=1 -init-pol=2 -rlevel=1 -alluiphack=2 -clsActB=2 -dontTrust -lhbr=3 -lhbr-sub -actIncMode=2 -laHack -dyn -laEEl -hlaLevel=1 -hlaevery=32 -hlabound=-1 -hlaTop=512 -sInterval=1 -learnDecP=80 -er-size=16 -er-lbd=12 -sUhdProbe=1 -no-sUhdPrRb -sUHLEsize=30 -sUHLElbd=12 -cp3_ee_bIter=400000000 -card_maxC=7 -card_max=2 -pr-uips=0 -pr-keepI=0 -no-pr-nce  -enabled_cp3 -cp3_stats -bve -bve_red_lits=1 -fm -no-cp3_fm_vMulAMO -unhide -cp3_uhdIters=5 -cp3_uhdEE -cp3_uhdTrans -bce -bce-cle -no-bce-bce 

#
# check for all parameters
#
if [ "x$rissParams" == "x" ]
then
	echo "c not all parameters specified ..."
	echo "s UNKNOWN"
	exit 0
fi

# show how script was called
echo "c file:    $file"
echo "c seed:    $seed"
echo "c tmp:     $tmpDir"
echo "c flips:   $sparrowFlips"
echo "c CPU s:   $sparrowSeconds"
echo "c sparrow: $sparrowParams"
echo "c riss:    $rissParams"

# binary of the used SAT solver
satsolver="sparrow	-a"					# name of the binary (if not in this directory, give relative path as well)

# parameters for preprocessor
cp3params="-enabled_cp3 -cp3_stats -up -subsimp -bve -no-bve_gates -no-bve_strength -bve_red_lits=1 -cp3_bve_heap=1 -bve_heap_updates=1 -bve_totalG -bve_cgrow_t=1000 -bve_cgrow=10 -ee -cp3_ee_it -unhide -cp3_uhdIters=5 -cp3_uhdEE -cp3_uhdTrans -cp3_uhdProbe=4 -cp3_uhdPrSize=3 -cp3_uhdUHLE=0"

#
# build parameters for sparrow
# (only if sparrow should be limited [ i.e. #flips is limited ]
#
flipParameter=""
secondsParameter=""
if [ "$sparrowFlips" -gt "0" ] # there is a flip limit
then
	flipParameter=" --maxflips=$sparrowFlips "
  secondsParameter="  --timeout $sparrowSeconds"
fi

# some temporary files 
undo=$tmpDir/cp_undo_$$				# path to temporary file that stores cp3 undo information
tmpCNF=$tmpDir/cp_tmpCNF_$$		# path to temporary file that stores cp3 simplified formula
model=$tmpDir/cp_model_$$			# path to temporary file that model of the preprocessor (stdout)
realModel=$tmpDir/model_$$			# path to temporary file that model of the SAT solver (stdout)
echo "c undo: $undo tmpCNF: $tmpCNF model: $model realModel: $realModel"

ppStart=0
ppEnd=0
solveStart=0
solveEnd=0
rissStart=0
rissEnd=0

#
# If DRAT is not used, start the usual solver
# Otherwise, skip Coprocessor and Sparrow, and start with Riss directly
#

#
# handle DRAT case
#
drup=""
if [ "x$doDRAT" != "x" ]
then
	# disable fm, because it does not support DRAT proofs
	drup="-drup=$tmpDir/drat_$$.proof -verb-proof=0 -no-dense -no-fm -proofFormat=DRAT"
fi

#
# run coprocessor with parameters added to this script
# and output to stdout of the preprocessor is redirected to stderr
#

exitCode=0
if [ "$sparrowFlips" -ne "0" ] # if no flips should be done, then this part can be skipped
then

	ppStart=`date +%s`
	#
	# simplify with coprocessor, or simply use the 
	#
	if [ "$sparrowFlips" -ne "-1" ]
	then
		$binaryPath./coprocessor $file $realModel -enabled_cp3 -undo=$undo -dimacs=$tmpCNF $cp3params 1>&2
		exitCode=$?
	else
		echo "c skipped preprocessor due to infinite flips" 1>&2
		tmpCNF=$file  # simply use the input file, but then be careful with the remaining steps!
	fi
	ppEnd=`date +%s`
	echo "c preprocessed $(( $ppEnd - $ppStart)) seconds" 1>&2
	echo "c preprocessed $(( $ppEnd - $ppStart)) seconds with exit code $exitCode"

	# solved by preprocessing
	if [ "$exitCode" -eq "10" -o "$exitCode" -eq "20" ]
	then 
		echo "c solved by preprocessor"
		winningSolver="Coprocessor"
	else
		echo "c not solved by preprocessor -- do search"
		if [ "$exitCode" -eq "0" ]
		then
			#
			# exit code == 0 -> could not solve the instance
			# dimacs file will be printed always
			# exit code could be 10 or 20, depending on whether coprocessor could solve the instance already
			#
	
			#
			# run your favorite solver (output is expected to look like in the SAT competition, s line and v line(s) )
			# and output to stdout of the sat solver is redirected to stderr
			#
			echo "c starting sparrow solver" 1>&2
			echo "c sparrow command line: ./$satsolver $flipParameter $secondsParameter $sparrowParams $tmpCNF $seed" 1>&2
			solveStart=`date +%s`
			$binaryPath./$satsolver $flipParameter $secondsParameter $sparrowParams $tmpCNF $seed > $model
			exitCode=$?
			solveEnd=`date +%s`
			echo "c solved $(( $solveEnd - $solveStart )) seconds" 1>&2
	
			#
			# undo the model
			# coprocessor can also handle "s UNSATISFIABLE"
			#
			echo "c post-process with coprocessor"
			if [ "$sparrowFlips" -ne "-1" ]
			then
				$binaryPath./coprocessor -post -undo=$undo -model=$model > $realModel
			else
				cat $model > $realModel # simply forward sparrow model
			fi
	
			#
			# verify final output if SAT?
			#
			if [ "$exitCode" -eq "10" ]
			then
				echo "c verify model ..."
				winningSolver="sparrow"
				# ./verify SAT $realModel $file
			fi
		else
			#
			# preprocessor returned some unwanted exit code
			#
			echo "c preprocessor has been unable to solve the instance"
			#
			# run sat solver on initial instance
			# and output to stdout of the sat solver is redirected to stderr
			#
			echo "c sparrow command line: ./$satsolver $flipParameter $secondsParameter $sparrowParams $file $seed" 1>&2
			solveStart=`date +%s`
			$binaryPath./$satsolver $flipParameter $secondsParameter $sparrowParams $file $seed > $realModel
			exitCode=$?
			solveEnd=`date +%s`
			echo "c solved $(( $solveEnd - $solveStart )) seconds" 1>&2
		fi
	fi
else
	echo "c skipped Coprocessor and Sparrow due to flips=0"
fi # end DRAT if
#
# 
#

#
# not yet solved, and riss should not be used
#
if [ "$sparrowFlips" -ne "-1" -a "$exitCode" -ne "10" -a "$exitCode" -ne "20" ]
then 
	echo "c use Riss 4.27" 1>&2
	# lets use Riss 4.27 for everything that could not be solved within the limits
	rissStart=`date +%s`
	#
	# use Riss
	# If DRAT should be used, the variable $drup contains the location to the proof, and disables FM in the solver (all other techniques work with DRAT)
	#
	$binaryPath./riss $file $realModel $rissParams
	exitCode=$?
	rissEnd=`date +%s`
	echo "c Riss 4.27 used $(( $rissEnd - $rissStart)) seconds with exit code $exitCode" 1>&2
	if [ "$exitCode" -eq "10" -o "$exitCode" -eq "20" ]
	then 
		winningSolver="Riss 4.27" 
	fi
fi


#
# print times
#
echo "c pp-time: $(( $ppEnd - $ppStart)) SLS-time: $(( $solveEnd - $solveStart ))  CDCL-time: $(( $rissEnd - $rissStart))" 1>&2
echo "c solved with: $winningSolver" 1>&2

#
# print solution
#
cat $realModel

#
# remove tmp files
#
rm -f $undo $undo.map $model $realModel
if [ "$sparrowFlips" -ne "-1" ]
then
	rm -f $tmpCNF # delete only, if it does not point to the file
fi

#
# return with correct exit code
#
exit $exitCode

