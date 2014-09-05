#!/usr/bin/perl

# checks the sizes of the petsc input files in
# the petsc data dir 
# and determines if they are ready to be read
# they must exist and be of same size
# example call:
# ./get_petsc_dir_state.pl -dir /workspace/matlab_petsc/petsc_data/data1/

use Getopt::Long;

my $data_dir;
my $result;

$cmdres = GetOptions ("dir=s"   => \$data_dir); 
#print "checking $data_dir\n";

# do ls and redirect std error output so it doesnt print
$cmd = "ls $data_dir/*petsc 2> /dev/null";
$ls_output = `$cmd`;

# if no petsc files found we should return -1 and exit
if(!$ls_output){
    print "-1\n";
    exit();
}

@file_sizes_output = `du -hs $data_dir/*petsc`;

my @file_sizes;
@fields = split(/\s+/, "@file_sizes_output");
foreach $field (@fields){
    if($field !~ /petsc/){
        #print "field: $field\n";
        push(@file_sizes,$field);
    }
}

my $file_size1 = $file_sizes[0];
foreach $file_size (@file_sizes){
    if(($file_size =~ /K/) || ($file_size =~ /M/)){
        $result = 0;
    }
    else{
        #print "not ok\n";
        $result = -1;
        print "$result\n";
        exit();
    }
}

print "$result\n";
