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

    parse_enums($idl_file_name, $out_dir_path);
}

sub parse_enums
{
    (my $idl_file_name, my $out_dir_path)=@_;
    my $project=GengineGen::parse_file($idl_file_name);
    my $intemidiate_dir = GengineGen::get_output_directory($project);

    $out_dir_path = GengineGen::get_output_path($intemidiate_dir, $out_dir_path);
    for my $enum(@{$project->{"enums"}})
    {
        output_enum_declaration($enum, $project->{"name"}, $out_dir_path);
    }
}

sub output_enum_declaration
{
    (my $enum, my $namespace, my $out_dir_path)=@_;
    my $enum_name=$enum->{"name"};
    my $file_name=GengineGen::get_rpc_enum_file_name($enum_name);
    my $file_path=GengineGen::get_output_path($file_name, $out_dir_path);

    my $hFile;
    make_path($out_dir_path);
    GengineGen::create_file($hFile,$file_path);

    printf($hFile "#pragma once\n\n");
    printf($hFile "#include <cstdint>\n");
    printf($hFile "\nnamespace ".$namespace."{\n");
    printf($hFile "enum class $enum_name: std::uint32_t\n");
    printf($hFile "{\n");
    my $index = 0;
    for my $field (@{$enum->{"fields"}})
    {
    if($index != 0)
        {
            printf($hFile ",\n");
        }
        printf($hFile "    ".$field->{"name"}." = ".$index);
        $index++;
    }
    printf($hFile "\n};\n}");
    close($hFile);
}
