#!/usr/bin/perl

use Cwd;

# script to clean up config dir and prepare petsc server for run
my $numArgs = $#ARGV + 1;
my $pwddir = getcwd;

if($numArgs < 1){
    print "need one command line argument: data directory name for petsc config and data files\n";
    exit -1;
}
my $data_dir = $ARGV[0];

my $cmd_str0 = "mkdir $data_dir"; # make dir if needed
my $cmd_str1 = "rm -f $data_dir/*"; # empty contents

print "command0: $cmd_str0\n";
print "command1: $cmd_str1\n";
system($cmd_str0);
system($cmd_str1);

open(my $fh, '>', "$data_dir/config.txt");
print $fh "pause\n";
print $fh "nofile\n";
print $fh "nofile\n";

