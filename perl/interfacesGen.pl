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
        GengineGen::print_usage();
        die("Not all command line arguments are set properly");
    }

    parse_interfaces($idl_file_name, $out_dir_path);
}

sub parse_interfaces
{
    (my $idl_file_name, my $out_dir_path)=@_;
    my $project=GengineGen::parse_file($idl_file_name);
    
    my $intemidiate_dir = GengineGen::get_output_directory($project);
    $out_dir_path = GengineGen::get_output_path($intemidiate_dir, $out_dir_path);
    
    for my $interface(@{$project->{"interfaces"}})
    {
        output_interface_declaration($interface, $project->{"name"}, $out_dir_path);
    }
}

sub output_interface_declaration
{
    (my $interface, my $namespace, my $out_dir_path)=@_;
    my $interface_name=$interface->{"name"};
    
    my $file_name=GengineGen::get_rpc_decl_file_name($interface_name);
    my $file_path=GengineGen::get_output_path($file_name, $out_dir_path);

    my $hFile;
    make_path($out_dir_path);
    GengineGen::create_file($hFile,$file_path);
    
    printf($hFile "#pragma once\n\n");
    printf($hFile "#include <string>\n");
    printf($hFile "#include <core/Blob.h>\n");
    printf($hFile "#include <core/Export.h>\n");
    printf($hFile "#include <core/IMicroService.h>\n");
    print_custom_types_forwards($hFile,$interface->{"methods"}, $namespace);
    printf($hFile "\nnamespace ".$namespace."{\n");
    printf($hFile "class GENGINE_API $interface_name: virtual public Gengine::Services::IMicroService\n");
    printf($hFile "{\n");
    printf($hFile "public:\n");
    printf($hFile "    virtual ~".$interface_name."() = default;\n");
    for my $method (@{$interface->{"methods"}})
    {
        my $member="virtual bool ".$method->{"name"}."(";
        printf($hFile "    $member");
        GengineGen::print_method_parameters_C_new($hFile,$method);
        printf($hFile ")=0;\n");
    }
    printf($hFile "};\n");
    printf($hFile "using T${interface_name} = std::shared_ptr<${interface_name}>;\n}");
    close($hFile);
}

sub print_forwards_impl
{
    (my $hFile, my $ctypes_, my $is_struct)=@_;
    my %ctypes = %{$ctypes_};
    foreach my $namespace (keys %ctypes) 
    {
        print($hFile "namespace ".$namespace." {\n");
        my @types = @{$ctypes{$namespace}};
        my @unique_types=uniq(@types);
        for my $utype (@{unique_types})
        {
            print($hFile "struct ".$utype.";\n") if($is_struct eq 1);
            print($hFile "class ".$utype.";\n") if($is_struct eq 0);
            print($hFile "using T".$utype." = std::shared_ptr<".$utype.">;\n");
        }
        print($hFile "}\n");
    }
}

sub print_custom_types_forwards
{
    (my $hFile,my $methods, my $default_namespace)=@_;
    my %struct_types;
    my %class_types;
    for my $method (@{$methods})
    {
        for my $arg (@{$method->{"arguments"}})
        {
            my $input_type=$arg->{"type"};
            if(none {$_ eq $input_type} @GengineGen::plain_types)
            {
                if(GengineGen::is_serializable($arg))
                {
                    push(@{$struct_types{$default_namespace}}, $input_type) if(!GengineGen::has_namespace($arg));
                    push(@{$struct_types{GengineGen::get_namespace($arg)}}, $input_type) if(GengineGen::has_namespace($arg));
                }
                else
                {
                    push(@{$class_types{$default_namespace}}, $input_type) if(!GengineGen::has_namespace($arg));
                    push(@{$class_types{GengineGen::get_namespace($arg)}}, $input_type) if(GengineGen::has_namespace($arg));
                }
            }
        }
    }
    print_forwards_impl($hFile, \%struct_types, 1);
    print_forwards_impl($hFile, \%class_types, 0);   
}
