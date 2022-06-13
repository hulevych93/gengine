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
    my $header = 2;
    my $idl_file_name="";
    my $out_dir_name="";
    my $out_dir_path="";
    my $data="";
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
        if($opt_name eq "-data:")
        {
            $data=$opt_val;
            $i++;#skip next arg
            next;
        }
        if($opt_name eq "-header")
        {
            $header = 1;
            $i++;#skip next arg
            next;
        }
        if($opt_name eq "-source")
        {
            $header = 0;
            $i++;#skip next arg
            next;
        }
    }
    if(not $idl_file_name)
    {
        print_usage();
        die("Not all command line arguments are set properly");
    }

    parse_data($idl_file_name, $out_dir_name, $out_dir_path, $header, $data);
}

sub parse_data
{
    (my $idl_file_name, my $out_dir_name, my $out_dir_path, my $header, my $data)=@_;
    my $project=GengineGen::parse_file($idl_file_name);
    fillup_data_extensions(@{$project->{"structs"}});

    my $intemidiate_dir = GengineGen::get_output_directory($project);
    
    for my $struct(@{$project->{"structs"}})
    {
        if($header or ($struct->{"name"} eq $data))
        {
            if($header eq 1)
            {
                output_data_declaration($struct, $project->{"name"}, $out_dir_name, $out_dir_path, $intemidiate_dir);
            }
            elsif($header eq 0)
            {
                output_data_definition($struct, $project->{"name"}, $out_dir_name, $out_dir_path, $intemidiate_dir);
            }
        }
    }
}

sub fillup_data_extensions
{
    (my @structs)=@_;
    for my $struct(@structs)
    {
        for my $thatStruct(@structs)
        {
            if(is_extension_data($thatStruct))
            {
                if(get_extension_data($thatStruct) eq $struct->{"name"})
                {
                    push @{$struct->{"children"}}, $thatStruct;
                    $thatStruct->{"parent"} = $struct;
                }
            }
        }
    }
}

sub output_data_declaration
{
    (my $struct, my $namespace, my $out_dir_name, my $out_dir_path, my $intemidiate_dir)=@_;
    my $out_dir_name_2 = GengineGen::get_output_path($intemidiate_dir, $out_dir_name);
    my $out_dir_path_2 = GengineGen::get_output_path($out_dir_name_2, $out_dir_path);
    my $struct_name=$struct->{"name"};

    my $file_name=GengineGen::get_rpc_data_file_name($struct_name);
    my $file_path=GengineGen::get_output_path($file_name, $out_dir_path_2);

    my $hFile;
    make_path($out_dir_path_2);
    GengineGen::create_file($hFile,$file_path);

    printf($hFile "#pragma once\n\n");
    printf($hFile "#include <core/Hashable.h>\n");
    printf($hFile "\n") if(print_data_includes($hFile, $struct));
    printf($hFile "\n") if(print_data_dependence_includes($hFile, $struct, $out_dir_name, $intemidiate_dir));
    printf($hFile "\n") if(print_data_dependence_forwards($hFile, $struct, $namespace));
    printf($hFile "namespace ".$namespace." {\n");
    GengineGen::print_comment_if_any($hFile, $struct);  printf($hFile "\n");
    print_data_declaration($hFile, $struct);
    printf($hFile "{\n");
    print_contructor_method_decl($hFile, $struct); printf($hFile "\n");
    printf($hFile "\n") if(print_creation_method_decl($hFile, $struct)); 
    print_data_serialization_methods_decl($hFile, $struct); printf($hFile "\n");
    print_data_operators_and_other_methods_decl($hFile, $struct);  printf($hFile "\n");
    print_data_fields($hFile, $struct);
    printf($hFile "};\n\n");
    printf($hFile "using T".$struct_name." = std::shared_ptr<".$struct_name.">;\n"); 
    printf($hFile "}\n");
    print_data_hashable($hFile, $struct, $namespace);
    close($hFile);
}

sub output_data_definition
{
    (my $struct, my $namespace,  my $out_dir_name, my $out_dir_path, my $intemidiate_dir)=@_;
    my $out_dir_name_2 = GengineGen::get_output_path($intemidiate_dir, $out_dir_name);
    my $out_dir_path_2 = GengineGen::get_output_path($out_dir_name_2, $out_dir_path);
    my $struct_name=$struct->{"name"};
    my $file_name=GengineGen::get_rpc_data_def_file_name($struct_name);
    my $hFile;
    GengineGen::create_file($hFile,$file_name);

    printf($hFile "#include <".GengineGen::get_output_path(GengineGen::get_rpc_data_file_name($struct_name), $out_dir_path_2).">\n");
    printf($hFile "#include <core/ToString.h>\n");
    print_include_children_def($hFile, $struct, $out_dir_path_2);   printf($hFile "\n");
    printf($hFile "using namespace Gengine;\n\n");
    printf($hFile "namespace ".$namespace." {\n\n");
    printf($hFile "\n") if(print_contructor_method_def($hFile, $struct)); 
    printf($hFile "\n") if(print_creation_method_def($hFile, $struct));   
    print_data_serialization_methods_def($hFile, $struct);   printf($hFile "\n");
    print_data_operators_and_other_methods_def($hFile, $struct);   printf($hFile "\n");
    printf($hFile "}");
    close($hFile);
}

sub is_dispatchable
{
    (my $struct)=@_;
    return 1 if($struct->{"annotations"}->{"Dispatch"});
    return undef;
}

sub get_dispatch
{
    (my $struct)=@_;
    return $struct->{"annotations"}->{"Dispatch"};
}

sub is_dispatch_case
{
    (my $struct)=@_;
    return 1 if($struct->{"annotations"}->{"DispatchCase"});
    return undef;
}

sub get_dispatch_case
{
    (my $struct)=@_;
    return $struct->{"annotations"}->{"DispatchCase"};
}

sub is_extension_data
{
    (my $struct)=@_;
    return 1 if($struct->{"annotations"}->{"Extends"});
    return undef;
}

sub get_extension_data
{
    (my $struct)=@_;
    return $struct->{"annotations"}->{"Extends"};
}

sub print_data_includes
{
    (my $hFile, my $struct)=@_;
    my $result = 0;
    printf($hFile "#include <memory>\n");
    if(is_extension_data($struct))
    {
        printf($hFile "#include \"".$struct->{"annotations"}->{"Extends"}.".h\"\n");
        $result = 1;
    }
    if(GengineGen::is_json_serializable($struct))
    {
        printf($hFile "#include <json/JSON.h>\n");
        $result = 1;
    }
    if(GengineGen::is_binary_serializable($struct))
    {
        printf($hFile "#include <serialization/ISerializable.h>\n");
        $result = 1;
    }
    return $result;
}

sub print_creation_method_decl
{
    (my $hFile, my $struct)=@_;
    if(is_dispatchable($struct))
    {
        my $struct_name=$struct->{"name"};
        printf($hFile "    static std::shared_ptr<".$struct_name."> Create(const std::uint32_t type);\n");
    }
    return 1 if(is_dispatchable($struct));
    return undef;
}

sub print_include_children_def
{
    (my $hFile, my $struct, my $out_dir_path)=@_;
    my $struct_name=$struct->{"name"};
    if(GengineGen::has_children($struct))
    {
        for my $child (@{GengineGen::get_children($struct)})
        {
            print_include_children_def($hFile, $child, $out_dir_path);
        }
    }
    else
    {
        printf($hFile "#include <".GengineGen::get_output_path(GengineGen::get_rpc_data_file_name($struct_name), $out_dir_path).">\n");
    }
}

sub find_dispatch
{
    (my $struct)=@_;
    if(is_extension_data($struct))
    {
        return find_dispatch($struct->{"parent"});
    }
    elsif(is_dispatchable($struct))
    {
        return get_dispatch($struct);
    }
    return undef;
}

sub print_contructor_method_decl
{
    (my $hFile, my $struct)=@_;
    my $struct_name=$struct->{"name"};
    if(is_dispatchable($struct))
    {
        my $dispatch = get_dispatch($struct);
        printf($hFile "protected:\n");
        printf($hFile "    ".$struct_name."(".$dispatch." type);\n\n");
        printf($hFile "public:");
    }
    elsif(is_extension_data($struct))
    {
        if(is_dispatch_case($struct))
        {
            printf($hFile "    ".$struct_name."();");
        }
        else
        {
            my $dispatch = find_dispatch($struct);
            if($dispatch)
            {
                printf($hFile "protected:\n");
                printf($hFile "    ".$struct_name."(".$dispatch." type);\n\n");
                printf($hFile "public:");
            }
        }
    }
    else
    {
        printf($hFile "    ".$struct_name."() = default;");
    }
}

sub print_contructor_method_def
{
    (my $hFile, my $struct)=@_;
    my $result = 0;
    my $struct_name=$struct->{"name"};
    if(is_dispatchable($struct))
    {
        my $dispatch = get_dispatch($struct);
        printf($hFile $struct_name."::".$struct_name."(".$dispatch." type)\n");
        printf($hFile ": type(type)\n");
        printf($hFile "{\n");
        printf($hFile "}\n");
        $result = 1;
    }
    elsif(is_extension_data($struct))
    {
        if(is_dispatch_case($struct))
        {
            my $dispatch = find_dispatch($struct);
            my $dispatchCase = get_dispatch_case($struct);
            printf($hFile $struct_name."::".$struct_name."()\n");
            printf($hFile ": ".get_extension_data($struct)."(".$dispatch."::".$dispatchCase.")\n");
            printf($hFile "{\n");
            printf($hFile "}\n");
        }
        else
        {
            my $dispatch = find_dispatch($struct);
            printf($hFile $struct_name."::".$struct_name."(".$dispatch." type)\n");
            printf($hFile ": ".get_extension_data($struct)."(type)\n");
            printf($hFile "{\n");
            printf($hFile "}\n");
        }
        $result = 1;
    }
    return $result;
}

sub print_creation_method_def_int
{
    (my $hFile, my $struct, my $dispatch)=@_;
    my $struct_name=$struct->{"name"};
    my $dispatch_case = get_dispatch_case($struct);
    if(GengineGen::has_children($struct))
    {
        for my $child (@{GengineGen::get_children($struct)})
        {
            print_creation_method_def_int($hFile, $child, $dispatch);
        }
    }
    else
    {
        printf($hFile "        case ".$dispatch."::".$dispatch_case.": return std::make_shared<".$struct_name.">();\n");
    }
}

sub print_creation_method_def
{
    (my $hFile, my $struct)=@_;
    my $struct_name=$struct->{"name"};
    if(is_dispatchable($struct))
    {
        my $dispatch = get_dispatch($struct);
        my $type_name = "type" if(GengineGen::has_children($struct));
        printf($hFile "std::shared_ptr<".$struct_name."> ".$struct_name."::Create(const std::uint32_t type)\n");
        printf($hFile "{\n");
        if(GengineGen::has_children($struct))
        {
            printf($hFile "    switch(static_cast<".$dispatch.">(type))\n");
            printf($hFile "    {\n");
            print_creation_method_def_int($hFile, $struct, $dispatch);
            printf($hFile "        default: return std::shared_ptr<".$struct_name.">();\n");
            printf($hFile "    }\n");
        }
        printf($hFile "}\n");
    }
    return 1 if(is_dispatchable($struct));
    return undef;
}

sub print_data_dependence_includes
{
    (my $hFile, my $struct,my $out_dir_name, my $intemidiate_dir)=@_;
    my $result = 0;
    my @ctypes;
    for my $field (@{$struct->{"fields"}})
    {
        my $input_type=$field->{"type"};
        if(none {$_ eq $input_type} @GengineGen::plain_types)
        {
            if($field->{"annotations"}->{"Dir"})
            {
                my $out_dir_path = GengineGen::get_output_path($field->{"annotations"}->{"Dir"}, $out_dir_name);
                my $input_type=GengineGen::get_output_path($input_type, $out_dir_path);
                push @ctypes, $input_type;
            }
            elsif($field->{"annotations"}->{"Include"})
            {
                my $out_dir_path = GengineGen::get_output_path($intemidiate_dir, $out_dir_name);
                my $input_type=GengineGen::get_output_path($field->{"annotations"}->{"Include"}, $out_dir_path);
                push @ctypes, $input_type;
            }
            elsif($field->{"annotations"}->{"InlineInclude"})
            {
                push @ctypes, $field->{"annotations"}->{"InlineInclude"};
            }
            elsif(!$field->{"annotations"}->{"F"})
            {
                my $out_dir_path = GengineGen::get_output_path($intemidiate_dir, $out_dir_name);
                my $input_type=GengineGen::get_output_path($input_type, $out_dir_path);
                push @ctypes, $input_type;
            }
        }
    }
    if(is_dispatchable($struct))
    {
        my $dispatch = get_dispatch($struct);
        my $out_dir_path = GengineGen::get_output_path($intemidiate_dir, $out_dir_name);
        my $input_type=GengineGen::get_output_path($dispatch, $out_dir_path);
        printf($hFile "#include <".$input_type.".h>\n");
        $result = 1;
    }
    my @unique_types=uniq(@ctypes);
    for my $utype (@{unique_types})
    {
        printf($hFile "#include <$utype.h>\n");
        $result = 1;
    }
    return $result;
}

sub print_data_dependence_forwards
{
    (my $hFile,my $struct, my $default_namespace)=@_;
    my $result = 0;
    my %ctypes;
    for my $field (@{$struct->{"fields"}})
    {
        my $dependence = $field->{"annotations"}->{"F"};
        if($dependence)
        {
            my $input_type=$field->{"type"};
            if(none {$_ eq $input_type} @GengineGen::plain_types)
            {
                push(@{$ctypes{$default_namespace}}, $input_type) if(!GengineGen::has_namespace($field));
                push(@{$ctypes{GengineGen::get_namespace($field)}}, $input_type) if(GengineGen::has_namespace($field));
            }
        }
    }
    foreach my $namespace (keys %ctypes) 
    {
        print($hFile "namespace ".$namespace." {\n");
        my @types = @{$ctypes{$namespace}};
        my @unique_types=uniq(@types);
        for my $utype (@{unique_types})
        {
            print($hFile "struct ".$utype.";\n");
        }
        print($hFile "}\n");
        $result = 1;
    }
    return $result;
}

sub print_data_declaration
{
    (my $hFile, my $struct)=@_;
    printf($hFile "struct ".$struct->{"name"});
    printf($hFile " final") if(!GengineGen::has_children($struct));
    if(is_extension_data($struct))
    {
        printf($hFile ": ".get_extension_data($struct));
    }
    else
    {
        my $im_first = 1;
        if(GengineGen::is_json_serializable($struct))
        {
            printf($hFile ": Gengine::JSON::IJsonSerializable");
            $im_first = 0;
        }
        if(GengineGen::is_binary_serializable($struct))
        {
            if($im_first)
            {
                printf($hFile ": ");
            }
            else
            {
                printf($hFile ", ");
            }
            printf($hFile "Gengine::Serialization::ISerializable");
        }
    }
    printf($hFile "\n");
}

sub print_data_serialization_methods_decl
{
    (my $hFile, my $struct)=@_;
    if(GengineGen::is_json_serializable($struct))
    {
        printf($hFile "    bool Serialize(Gengine::JSON::Object& serializer) const override;\n");
        printf($hFile "    bool Deserialize(const Gengine::JSON::Object& deserializer) override;\n");
    }
    if(GengineGen::is_binary_serializable($struct))
    {
        printf($hFile "    bool Serialize(Gengine::Serialization::Serializer& serializer) const override;\n");
        printf($hFile "    bool Deserialize(const Gengine::Serialization::Deserializer& deserializer) override;\n");
    }
}

sub print_data_serialization_methods_def
{
    (my $hFile, my $struct)=@_;
    my $struct_name=$struct->{"name"};
    my $isExtension = is_extension_data($struct);
    if(GengineGen::is_json_serializable($struct))
    {
        printf($hFile "bool ".$struct_name."::Serialize(JSON::Object& serializer) const\n");
        printf($hFile "{\n");
        printf($hFile "    auto result = true;\n") if(!$isExtension);
        printf($hFile "    auto result = ".get_extension_data($struct)."::Serialize(serializer);\n") if($isExtension);
        if(is_dispatchable($struct))
        {
            printf($hFile "    serializer[\"type\"] << type;\n");
        }
        for my $field (@{$struct->{"fields"}})
        {
            printf($hFile "    serializer[\"".$field->{"name"}."\"] << ".$field->{"name"}.";\n");
        }
        printf($hFile "    return result;\n");
        printf($hFile "}\n\n");
        
        printf($hFile "bool ".$struct_name."::Deserialize(const JSON::Object& deserializer)\n");
        printf($hFile "{\n");
        printf($hFile "    auto result = true;\n") if(!$isExtension);
        printf($hFile "    auto result = ".get_extension_data($struct)."::Deserialize(deserializer);\n") if($isExtension);
        if(is_dispatchable($struct))
        {
            printf($hFile "    deserializer[\"type\"] >> type;\n");
        }
        for my $field (@{$struct->{"fields"}})
        {
            printf($hFile "    deserializer[\"".$field->{"name"}."\"] >> ".$field->{"name"}.";\n");
        }
        printf($hFile "    return result;\n");
        printf($hFile "}\n");
    }
    if(GengineGen::is_binary_serializable($struct))
    {
        printf($hFile "bool ".$struct_name."::Serialize(Serialization::Serializer& serializer) const\n");
        printf($hFile "{\n");
        printf($hFile "    auto result = true;\n") if(!$isExtension);
        printf($hFile "    auto result = ".get_extension_data($struct)."::Serialize(serializer);\n") if($isExtension);
        if(is_dispatchable($struct))
        {
            printf($hFile "    serializer << type;\n");
        }
        for my $field (@{$struct->{"fields"}})
        {
            printf($hFile "    serializer << ".$field->{"name"}.";\n");
        }
        printf($hFile "    return result;\n");
        printf($hFile "}\n\n");
        
        printf($hFile "bool ".$struct_name."::Deserialize(const Serialization::Deserializer& deserializer)\n");
        printf($hFile "{\n");
        printf($hFile "    auto result = true;\n") if(!$isExtension);
        printf($hFile "    auto result = ".get_extension_data($struct)."::Deserialize(deserializer);\n") if($isExtension);
        if(is_dispatchable($struct))
        {
            printf($hFile "    deserializer >> type;\n");
        }
        for my $field (@{$struct->{"fields"}})
        {
            printf($hFile "    deserializer >> ".$field->{"name"}.";\n");
        }
        printf($hFile "    return result;\n");
        printf($hFile "}\n");
    }
}

sub print_data_operators_and_other_methods_decl
{
    (my $hFile, my $struct)=@_;
    my $struct_name=$struct->{"name"};
    printf($hFile "    ".$struct_name."& operator=(const ".$struct_name."& that);\n");
    printf($hFile "    bool operator==(const ".$struct_name."& that) const;\n");
    printf($hFile "    bool operator<(const ".$struct_name."& that) const;\n");
    printf($hFile "    bool operator!=(const ".$struct_name."& that) const;\n");
    printf($hFile "    friend std::ostream& operator<<(std::ostream& os, const ".$struct_name."& that);\n");
    printf($hFile "    friend std::ostream& operator<<(std::ostream& os, const std::shared_ptr<".$struct_name.">& that);\n");
    printf($hFile "    std::string ToString() const;\n");
}

sub print_data_operators_and_other_methods_def
{
    (my $hFile, my $struct)=@_;
    my $struct_name=$struct->{"name"};
    my $isExtension = is_extension_data($struct);
    printf($hFile $struct_name."& ".$struct_name."::operator=(const ".$struct_name."& that)\n");
    printf($hFile "{\n");
    printf($hFile "    ".get_extension_data($struct)."::operator=(that);\n") if($isExtension);
    if(is_dispatchable($struct))
    {
        printf($hFile "    type = that.type;\n");
    }
    for my $field (@{$struct->{"fields"}})
    {
        printf($hFile "    ".$field->{"name"}." = that.".$field->{"name"}.";\n");
    }
    printf($hFile "    return *this;\n");
    printf($hFile "}\n\n");
    
    printf($hFile "bool ".$struct_name."::operator==(const ".$struct_name."& that) const\n");
    printf($hFile "{\n");
    printf($hFile "    auto result = true;\n") if(!$isExtension);
    printf($hFile "    auto result = ".get_extension_data($struct)."::operator==(that);\n") if($isExtension);
    if(is_dispatchable($struct))
    {
        printf($hFile "    result &= type == that.type;\n");
    }
    for my $field (@{$struct->{"fields"}})
    {
        printf($hFile "    result &= ".$field->{"name"}." == that.".$field->{"name"}.";\n");
    }
    printf($hFile "    return result;\n");
    printf($hFile "}\n\n");

    printf($hFile "bool ".$struct_name."::operator<(const ".$struct_name."& that) const\n");
    printf($hFile "{\n");
    printf($hFile "    auto result = true;\n") if(!$isExtension);
    printf($hFile "    auto result = ".get_extension_data($struct)."::operator<(that);\n") if($isExtension);
    if(is_dispatchable($struct))
    {
        printf($hFile "    result &= type < that.type;\n");
    }
    printf($hFile "    result &= std::tie(");
    my $commaHelper = 1;
    for my $field (@{$struct->{"fields"}})
    {
        if($commaHelper eq 0)
        {
            printf($hFile ",\n        ");
        }
        $commaHelper = 0;
        printf($hFile $field->{"name"});
    }
    printf($hFile ") < std::tie(");
    $commaHelper = 1;
    for my $field (@{$struct->{"fields"}})
    {
        if($commaHelper eq 0)
        {
            printf($hFile ",\n        ");
        }
        $commaHelper = 0;
        printf($hFile "that.".$field->{"name"});
    }
    printf($hFile ");\n");
    printf($hFile "    return result;\n");
    printf($hFile "}\n\n");
    
    printf($hFile "bool ".$struct_name."::operator!=(const ".$struct_name."& that) const\n");
    printf($hFile "{\n");
    printf($hFile "    return !operator==(that);\n");
    printf($hFile "}\n\n");
    
    printf($hFile "std::ostream& operator<<(std::ostream& os, const ".$struct_name."& that)\n");
    printf($hFile "{\n");
    printf($hFile "    os << that.ToString();\n");
    printf($hFile "    return os;\n");
    printf($hFile "}\n\n");
    
    printf($hFile "std::ostream& operator<<(std::ostream& os, const std::shared_ptr<".$struct_name.">& that)\n");
    printf($hFile "{\n");
    printf($hFile "    os << that->ToString();\n");
    printf($hFile "    return os;\n");
    printf($hFile "}\n\n");
    
    printf($hFile "std::string ".$struct_name."::ToString() const\n");
    printf($hFile "{\n");
    printf($hFile "    std::string os;\n") if(!$isExtension);
    printf($hFile "    auto os = ".get_extension_data($struct)."::ToString();\n") if($isExtension);
    printf($hFile "    os += \'{\';\n");
    my $isFirst = 1;
    for my $field (@{$struct->{"fields"}})
    {
        if($isFirst == 0)
        {
            printf($hFile " os += \',\';\n");
        }
        $isFirst = 0;
        printf($hFile "    os += Gengine::ToString(".$field->{"name"}.");");
    }
    printf($hFile "\n    os += \'}\';\n");
    printf($hFile "    return os;\n");
    printf($hFile "}\n");
}

sub print_data_fields
{
    (my $hFile, my $struct)=@_;
    if(is_dispatchable($struct))
    {
        my $dispatch = get_dispatch($struct);
        printf($hFile "    ".$dispatch." type;\n");
    }
    for my $field (@{$struct->{"fields"}})
    {
        printf($hFile "    ".GengineGen::get_parameter_from_type($field)." ".$field->{"name"}.";\n");
    }
}

sub print_data_hashable
{
    (my $hFile, my $struct, my $namespace)=@_;
    print($hFile "MAKE_HASHABLE(".$namespace."::".$struct->{"name"});
    for my $field (@{$struct->{"fields"}})
    {
        printf($hFile ", t.".$field->{"name"});
    }
    print($hFile ");\n");
}
