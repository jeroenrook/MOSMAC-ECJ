program : cross.h env.h evaluator.h indi.h kopt.h rand.h sort.h cross.cpp env.cpp evaluator.cpp indi.cpp kopt.cpp main.cpp rand.cpp sort.cpp  
 
	g++ -O3 -c ./cross.cpp -o cross.o
	g++ -O3 -c ./env.cpp -o env.o
	g++ -O3 -c ./evaluator.cpp -o evaluator.o
	g++ -O3 -c ./indi.cpp -o indi.o
	g++ -O3 -c ./kopt.cpp -o kopt.o
	g++ -O3 -c ./main.cpp -o main.o
	g++ -O3 -c ./rand.cpp -o rand.o
	g++ -O3 -c ./sort.cpp -o sort.o
 
	g++ -static -O3 cross.o env.o evaluator.o indi.o kopt.o main.o rand.o sort.o -o jikken
