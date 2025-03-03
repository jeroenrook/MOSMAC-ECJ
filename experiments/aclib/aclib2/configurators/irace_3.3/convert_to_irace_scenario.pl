#!/usr/bin/perl -w
use strict;
use warnings FATAL => 'all';
use diagnostics;
use Cwd;
use Cwd 'abs_path';
use File::Basename;

my $buffer = "";
my $valid = 1;
my $paramfile;
my $real_target_runner;
my @real_target_runner_args;
my $cutoff_time;
# ./scenario/$problem/$scenario_name/
my $cwd = getcwd();
my $rootdir = "./";
my $bindir = abs_path (dirname(abs_path($0)) . "/../..");

sub quote { my $s = shift; return "\"$s\""; }
my $as_numeric = sub { return shift; };
#my $handle_paramfile = sub { $paramfile = shift; return quote($rootdir.$paramfile. ".irace"); };
my $handle_paramfile = sub { $paramfile = shift; return quote($rootdir."paramfile.irace"); };
my $handle_overall_obj = sub { 
    my $value = shift;
    if ($value eq "mean") {
        return quote("t-test"); 
    } elsif ($value eq "PAR10") {
        print STDERR "warning: value '$value' for overall-obj not strictly supported by irace, we will just use 't-test'\n";
        return quote("t-test"); 
    } else {
        print STDERR "warning: value '$value' for overall-obj not supported by irace\n";
        return quote($value);
    }
};

my $bindir_path = sub {
    return quote(abs_path($bindir . "/" . shift));
};
my $rootdir_path = sub {
    return quote($rootdir. shift);
};

my $handle_target_runner = sub {
    $real_target_runner = shift;
    return &$bindir_path("configurators/irace_3.3/irace_wrapper.py");
};
my $handle_target_runner_arg = sub {
    my $arg = shift;
    push(@real_target_runner_args, $arg);
};

my $handle_cutoff = sub {
    $cutoff_time = shift;
    return $cutoff_time;
};

my $handle_test_instances = sub {
    print STDERR "warning: test-instances will be ignored, since we will use a different tool to test the best configurations\n";
};

my %coding = (
              'executable' => ['targetRunner', $handle_target_runner ],
              'arg' => ['', $handle_target_runner_arg],
              'deterministic' => ['deterministic', $as_numeric],
              'paramfile' => ['parameterFile', $handle_paramfile],
              'cutoff-time' => ['boundMax', $handle_cutoff],
	      #'cutoff-time' => ['', $handle_cutoff],
              'run-obj' => [''],
              'overall-obj' => ['testType', $handle_overall_obj],
              'wallclock-limit' => ['maxTime', $as_numeric],
              'cpu-limit' => ['maxTime', $as_numeric],
              'max-evaluations' => ['maxExperiments', $as_numeric],
              'training-instances' => ['trainInstancesFile', $bindir_path],
              #'test-instances' => ['testInstances'],
              'test-instances' => ['', $handle_test_instances],
              'instance-features' => [''],
             );

sub match_and_convert {
    my $line = shift;
    unless ($line =~/([^\s]+)\s*=/) {
        die "cannot match within $line\n";
    }
    my $aclib = $1;
    unless (defined ($coding{$aclib})) {
        die "cannot find a coding for '$aclib' in $line\n";
    }
    my ($irace, $func) = @{ $coding{$aclib} };
    if ($line =~/${aclib}\s*=\s*(.+)\s*$/) {
        my $value = $1;
        if (defined($func)) {
            $value = &$func($value);
        } else {
            $value = quote($value);
        }
        if (${irace}) {
            $buffer .= "${irace} = $value\n";
        } elsif (not defined($func)) {
            print STDERR "warning: '${aclib}' is ignored\n";
            return 0;
        }
        return 1;
    }
    die "error: cannot find a value for '$aclib' in $line\n";
}

# Read scenario file
my $fh;
open $fh, '<', "$ARGV[0]" or die "Can't open $ARGV[0]: $!\n";
foreach my $line (<$fh>) {
    $valid = 0 if (not match_and_convert ($line));
}
close $fh;

my @params;
my @forbidden;
my %types;
my %paramline;
my %defaults;

sub fix_param_name {
    my $s = shift;
    $s =~ s/[^a-z0-9_]/_/ig;
    $s =~ s/^[^a-z0-9.]/x/ig;
    $s =~ s/[^a-z0-9_]$/_/ig;
    return $s;
}

sub convert_aclib_forbidden {
    my $s = "";
    foreach my $exp (@_) {
        # print STDERR $exp . "\n";
        my ($var, $value) = $exp =~ /([^\s]+)\s*=\s*([^,\s]+)/;
        $s .= " & " if ($s);
        $s .= fix_param_name($var) . " == " . quote($value);
        # print STDERR "$s\n";
    }
    return $s;
}

sub fix_condition {
    my $s = shift;
    $s =~ s/ in / \%in\% /g;
    $s =~ s/\s\{"?/ c("/g;
    $s =~ s/"?\}/")/g;
    $s =~ s/\s*,\s*/", "/g;
    $s =~ s/\s\|\|\s/ | /g;
    $s =~ s/\s&&\s/ & /g;
    for my $name (keys %paramline) {
        my $fixed = fix_param_name($name);
        $s =~ s/\Q ${name} \E/ ${fixed} /g;
    }
    return $s;
}

sub read_parameter {
    my $line = shift;
    if ($line =~ /^\s*#/) {
        return; # Skip comments.
    }
    if ($line =~ /^\{\s*(.*)\}/) {

        my @aclibforbidden = split(/,\s*/, $1);
        if (@aclibforbidden) {
            push (@forbidden, convert_aclib_forbidden(@aclibforbidden));
        }
    } elsif ($line =~ /([^\s]+)\s+(categorical|ordinal|integer|real)\s+[[{](.+)[\]}]\s*[[](.*)[\]]\s*(log)?/) {
        my ($name, $type, $domain, $default) = ($1, substr($2, 0, 1), $3, $4, $5);
        $paramline{$name} = fix_param_name($name) . " \"-${name} \" $type ($domain)";
        #print "$name: $default\n";
        $defaults{$name} = $default;
        push(@params, $name);
    } elsif ($line =~ /([^\s]+)\s+\|(\s.+)$/) {
        my ($name, $condition) = ($1, $2);
        $condition = fix_condition($condition);
        die "parameter $name not found!" unless ($paramline{$name});
        $paramline{$name} .= " |$condition";
    }
}
open $fh, '<', "$paramfile" or die "Can't open $paramfile: $!\n";

foreach my $line (<$fh>) {
    read_parameter($line);
}

#open my $fhparamout, '>', "$cwd/${paramfile}.irace" or die "Can't open $cwd/${paramfile}.irace: $!\n";
open my $fhparamout, '>', "$cwd/paramfile.irace" or die "Can't open $cwd/paramfile.irace: $!\n";



for (my $i = 0; $i < @real_target_runner_args; $i++) {
    $real_target_runner .= " " . $real_target_runner_args[$i];
}

#if ($cutoff_time){
#    print $fhparamout "cutoff_time \"--cutoff \" c (\"$cutoff_time\")\n";
#}
print $fhparamout "real_target_runner \"$real_target_runner --config \" c (\"\")\n";
foreach my $param (@params) {
    print $fhparamout $paramline{$param} . "\n";
}

# FIXME: Either no default for all parameters or default missing for inactive
# parameters. We need to handle the case where default is given only for some
# active parameters.
my $has_default = 0;
foreach my $param (@params) {
    if (not defined($defaults{$param}) or $defaults{$param} eq "") {
        $defaults{$param} = 'NA';
    } else {
        $has_default = 1;
    }
}

# Write scenario file
#open my $fhout, '>', "$ARGV[0].irace" or die "Can't open $ARGV[0].irace: $!\n";
open my $fhout, '>', "scenario.irace" or die "Can't write scenario.irace: $!\n";
print $fhout $buffer;
print $fhout "trainInstancesDir = \"" . $rootdir . "\"\n";
print $fhout "configurationsFile = \"" . $rootdir . "paramfile.default.irace\"\n" if ($has_default);
print $fhout "forbiddenFile = \"" . $rootdir . "paramfile.forbidden.irace\"\n" if (@forbidden);
print $fhout "aclib = TRUE\n";
print $fhout "execDir = \"" . $rootdir . "\"\n";
print $fhout "digits = 10\n";
close $fhout;

if ($has_default) {
    open my $fhdefault, '>', "paramfile.default.irace" or die "Can't open paramfile.default.irace: $!\n";
    my @temp = map { fix_param_name($_)} @params;
    print $fhdefault join(" ", @temp). "\n";
    foreach my $param (@params) {
        print $fhdefault $defaults{$param}. " ";
    }
    print $fhdefault "\n";
    close $fhdefault;
} else {
    print "warning: no default configuration detected\n"
}

close $fhparamout;
close $fh;

if (@forbidden) {
    open my $fhforbidden, '>', "paramfile.forbidden.irace" or die "Can't open paramfile.forbidden.irace: $!\n";
    print $fhforbidden join("\n", @forbidden) . "\n";
    close $fhforbidden;
}
