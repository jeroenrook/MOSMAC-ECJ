install: ./irace_2.0/R/*.R
	R CMD INSTALL irace_2.0 --library=./irace_2.0/installed/

jsonlite: ./irace_2.0/installed/jsonlite

./irace_2.0/installed/jsonlite:
	@mkdir -p ./irace_2.0/installed/  
	R CMD INSTALL ./irace_2.0/jsonlite_0.9.16.tar.gz --library=./irace_2.0/installed/

