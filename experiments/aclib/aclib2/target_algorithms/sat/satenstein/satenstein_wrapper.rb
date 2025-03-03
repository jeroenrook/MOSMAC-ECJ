#!/usr/bin/ruby
# ParamILS wrapper for CPLEX 12.1

#=== Deal with inputs.
module Enumerable
  def dups
    inject({}) {|h,v| h[v]=h[v].to_i+1; h}.reject{|k,v| v==1}.keys
  end
end


if ARGV.length < 5
	puts "satenstein_wrapper.rb is a wrapper for the SATENSTEIN algorithm."
	puts "Usage: ruby satenstein_wrapper.rb <instance_relname> <instance_specifics> <cutoff_time> <cutoff_length> <seed> <params to be passed on>."
	exit -1
end
cnf_filename = ARGV[0]
instance_specifics = ARGV[1]
cutoff_time = ARGV[2].to_f
cutoff_length = ARGV[3].to_i
seed = ARGV[4].to_i

#=== Here I assume instance_specifics only contains the desired target quality or nothing at all for the instance, but it could contain more (to be specified in the instance_file or instance_seed_file)
if instance_specifics == ""
	qual = 0
else
	qual = instance_specifics.split[0]
end

allparaname=[];
allparavalue=[];
paramstring="";
paranum=(ARGV.length-5)/2
for i in 0...paranum
   if ARGV[5+i*2]=~/-DLS(.*)/
       allparaname[i]="-#{$1}"
    else
      if ARGV[5+i*2]=~/-CON(.*)/
        allparaname[i]="-#{$1}"
      else
        allparaname[i]=ARGV[5+i*2]
      end
    end
   allparavalue[i]=ARGV[6+i*2]
   paramstring="#{paramstring} #{allparaname[i]} #{allparavalue[i]} "
#   puts allparaname[i]
#   puts allparavalue[i]
end
 
# check if there is some duplicate names 
# if do have one exit with a warning message
if allparaname.dups.inspect.length>2
   puts "Warning, some duplicate parameters! #{allparaname.dups.inspect}"
   exit 1
end

# paramstring = ARGV[5...ARGV.length].join(" ")

#=== Build algorithm command and execute it.
cmd = "./target_algorithms/sat/satenstein/ubcsat -alg satenstein #{paramstring} -inst #{cnf_filename} -cutoff #{cutoff_length+2} -timeout #{cutoff_time} -target #{qual} -seed #{seed} -r stats stdout default,best"
filename = "./ubcsat_output#{rand}.txt"
timename= "./ubcsat_time#{rand}.txt"
runsolver_executable ="target_algorithms/runsolver/runsolver"
#runsolver_executable ="/ubc/cs/project/arrow/cchris13/zilla/features/SAT/runsolver/runsolverx32/runsolver"


memout="2000"
exec_cmd = "#{runsolver_executable} --timestamp -w #{timename} -o #{filename} -C #{cutoff_time} -M #{memout} #{cmd}"
#exec_cmd = "/ubc/cs/research/arrow-raid1/projects/SATenstein/satenstein/timerun #{cutoff_time+5} #{cmd} 1> #{filename} 2>#{timename}"

puts "Calling: #{exec_cmd}"
system exec_cmd

#=== Parse algorithm output to extract relevant information for ParamILS.
solved = "TIMEOUT"
runtime = cutoff_time
runlength = -1
best_sol = -1
satflag=0
File.open(filename){|file|
	while line = file.gets
		if line =~ /SuccessfulRuns = (\d+)/
			numsolved = $1.to_i
			if numsolved > 0
				solved = "SAT"
                                satflag=1
			else
				solved = "TIMEOUT"
			end
		end
		if line =~ /CPUTime_Mean = (.*)$/
			#runtime = $1.to_f
		end
		if line =~ /Steps_Mean = (\d+)/
			runlength = $1.to_i
		end
		if line =~ /BestSolution_Mean = (\d+)/
			best_sol = $1.to_i
		end
	end
}
File.open(timename){|file|
        while line = file.gets
                if line =~ /^CPU time \(s\): (.*)/
                        runtime = $1.to_f
                end
        end
}
if satflag<1
  runtime=cutoff_time
end
File.delete(filename)
File.delete(timename)
puts "Result for ParamILS: #{solved}, #{runtime}, #{runlength}, #{best_sol}, #{seed}"
