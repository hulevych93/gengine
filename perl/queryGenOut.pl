#!/usr/bin/perl
package RPCGenerator;

use strict;
use warnings;
use Digest::MD5 qw(md5 md5_hex md5_base64);
use File::Path;
use File::Basename;
use File::Path qw/make_path/;
use Cwd 'abs_path';
use List::MoreUtils qw(uniq);
use List::Util qw(none);

BEGIN 
{
    #set path to RecDescent relative to current script, not current dir 
    #(to ensure that script can be successfully caled from various locations)
    my $path=dirname(abs_path($0));
    unshift(@INC, ($path."/modules"));
}

use Parse::RecDescent;
use GengineGen;

main();

sub main
{
    #parse command line arguments
    my $num_args=$#ARGV+1;
    my $idl_file_name="";
    my $out_dir_path="";
    my $out_dir_name="";
    my $i;
    for($i=0;$i<$num_args;$i++)
    {
        my $opt_name=$ARGV[$i];
        my $opt_val=$ARGV[$i+1];

        if($opt_name eq "-idl_file:")
        {
            $idl_file_name=$opt_val;
            $i++;#skip next arg
            next;
        }
        if($opt_name eq "-out_dir:")
        {
            $out_dir_name=$opt_val;
            $i++;#skip next arg
            next;
        }
        if($opt_name eq "-out_path:")
        {
            $out_dir_path=$opt_val;
            $i++;#skip next arg
            next;
        }
    }
    if(not $idl_file_name)
    {
        print_usage();
        die("Not all command line arguments are set properly");
    }

    parse($idl_file_name, $out_dir_path, $out_dir_name);
}

sub parse
{
    (my $idl_file_name, my $out_dir_path, my $out_dir_name)=@_;
    my $project=GengineGen::parse_file($idl_file_name);
    
    my $intemidiate_dir = GengineGen::get_output_directory($project);
    
    for my $enum(@{$project->{"enums"}})
    {
        my $out_dir_path = GengineGen::get_output_path($intemidiate_dir, $out_dir_path);
        my $file_name=GengineGen::get_rpc_enum_file_name($enum->{"name"});
        my $file_path=GengineGen::get_output_path($file_name, $out_dir_path);
        
        print STDOUT $file_path.";";
    }
    
    for my $interface(@{$project->{"interfaces"}})
    {
        my $out_dir_path = GengineGen::get_output_path($intemidiate_dir, $out_dir_path);
        my $file_name=GengineGen::get_rpc_decl_file_name($interface->{"name"});
        my $file_path=GengineGen::get_output_path($file_name, $out_dir_path);
        
        print STDOUT $file_path.";";
    }
    
    for my $struct(@{$project->{"structs"}})
    {
        my $out_dir_path = GengineGen::get_output_path($intemidiate_dir, $out_dir_path);
        my $file_name=GengineGen::get_rpc_data_file_name($struct->{"name"});
        my $file_path=GengineGen::get_output_path($file_name, $out_dir_path);
        
        print STDOUT $file_path.";";
    }
}
