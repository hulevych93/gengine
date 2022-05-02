#!/usr/bin/perl

package GengineGen;

use strict;
use warnings;

use Parse::RecDescent;
use List::Util qw(any none);

our $AUTOGEN_WARNING_COMMENT=<<COMMENT;
/**
\@file
Automatically generated file. Do not edit - all changes will be lost.
Modify .rdl files and run rpcgen.pl to perform modifications.
*/
COMMENT

$::RD_HINT=1;
$::RD_ERRORS=1;

#$::RD_TRACE = 1 ;
our @plain_types = (
    "uint8", 
    "uint16", 
    "uint32", 
    "uint64", 
    "int8", 
    "int16", 
    "int32", 
    "int64", 
    "bool", 
    "String", 
    "WString", 
    "BLOBOBJ",
    "Ptr"
);

our @int_types = (
    "uint8", 
    "uint16", 
    "uint32", 
    "uint64", 
    "int8", 
    "int16", 
    "int32", 
    "int64", 
    "bool",
    "Ptr"
);

our $GRAMMAR ='
       Process: PROJECT
       PROJECT: ANNOTATION(s?) "namespace" IDENTIFIER "{" ENUM(s?) DATA_STRUCTURE(s?) INTERFACE(s?) "};"
                    {
                        my %project;
                        $project{"name"}=$item{"IDENTIFIER"};
                        $project{"interfaces"}=\@{$item{"INTERFACE(s?)"}};
                        $project{"structs"}=\@{$item{"DATA_STRUCTURE(s?)"}};
                        $project{"enums"}=\@{$item{"ENUM(s?)"}};
                        my $annotations=$item{"ANNOTATION(s?)"};
                        my %annotations;
                        for my $annotation(@$annotations)
                        {
                            $annotations{$annotation->{"type"} }=$annotation->{"value"};
                        }
                        $project{"annotations"}=\%annotations;
                        return \%project;
                    }
       ANNOTATION: "[" IDENTIFIER ANNOTATION_VALUE(0..1) "]"
                    {
                        my $annType=$item[2];
                        my $annVal=$item[3][0];
                        if(not $annVal)
                        {
                            $annVal="1";
                        }
                        $annVal=~s/[\"\(\)\:]*//g;
                        my %annotation=("type"=>$annType,"value"=>$annVal);
                        $return=\%annotation;
                    }
       IDENTIFIER: /[a-zA-Z0-9_]+/
       ANNOTATION_VALUE: /\(\"?:?[a-zA-Z0-9\.\,_\/\-?]+\"?\)/
       INTERFACE: ANNOTATION(s?) "interface" IDENTIFIER "{" METHOD(s) "};"
                    {
                        my %interface;
                        $interface{"name"}=$item{"IDENTIFIER"};
                        my $annotations=$item{"ANNOTATION(s?)"};
                        my %annotations;
                        for my $annotation(@$annotations)
                        {
                            $annotations{$annotation->{"type"} }=$annotation->{"value"};
                        }
                        $interface{"annotations"}=\%annotations;
                        $interface{"methods"}=$item{"METHOD(s)"};
                        $return=\%interface;
                    }
       ENUM: ANNOTATION(s?) "enum" IDENTIFIER "{" FIELD(s) "};"
                    {
                        my %enum;
                        $enum{"name"}=$item{"IDENTIFIER"};
                        my $annotations=$item{"ANNOTATION(s?)"};
                        my %annotations;
                        for my $annotation(@$annotations)
                        {
                            $annotations{$annotation->{"type"} }=$annotation->{"value"};
                        }
                        $enum{"annotations"}=\%annotations;
                        $enum{"fields"}=$item{"FIELD(s)"};
                        $return=\%enum;
                    }
       FIELD: IDENTIFIER ";"
                    {
                        my %field;
                        $field{"name"}=$item{"IDENTIFIER"};
                        $return=\%field;
                    }
       METHOD: IDENTIFIER "(" ARGUMENT(s? /[,]*/) ")" ";"
                    {
                        my %method;
                        $method{"name"}=$item{"IDENTIFIER"};
                        $method{"arguments"}=$item[3];
                        $return=\%method;
                    }
       ARGUMENT: ANNOTATION(s?) ARGUMENT_TYPE /\*?/ IDENTIFIER
                    {
                        my %argument;
                        $argument{"name"}=$item{"IDENTIFIER"};
                        my $annotations=$item{"ANNOTATION(s?)"};
                        my %annotations;
                        for my $annotation(@$annotations)
                        {
                            $annotations{$annotation->{"type"} }=$annotation->{"value"};
                        }
                        $argument{"annotations"}=\%annotations;
                        $argument{"type"}=$item{"ARGUMENT_TYPE"};
                        $return=\%argument;
                    }
       ARGUMENT_TYPE: /[a-zA-Z0-9_]+/
       DATA_STRUCTURE: ANNOTATION(s?) "struct" IDENTIFIER "{" DATA(s) "};"
                    {
                        my %structure;
                        $structure{"name"}=$item{"IDENTIFIER"};
                        my $annotations=$item{"ANNOTATION(s?)"};
                        my %annotations;
                        for my $annotation(@$annotations)
                        {
                            $annotations{$annotation->{"type"} }=$annotation->{"value"};
                        }
                        $structure{"annotations"}=\%annotations;
                        $structure{"fields"}=$item{"DATA(s)"};
                        $return=\%structure;
                    }
       DATA: ARGUMENT_TYPE IDENTIFIER ";" ANNOTATION(s?) 
                    {
                        my %data;
                        $data{"name"}=$item{"IDENTIFIER"};
                        my $annotations=$item{"ANNOTATION(s?)"};
                        my %annotations;
                        for my $annotation(@$annotations)
                        {
                            $annotations{$annotation->{"type"} }=$annotation->{"value"};
                        }
                        $data{"annotations"}=\%annotations;
                        $data{"type"}=$item{"ARGUMENT_TYPE"};
                        $return=\%data;
                    }
    ';

our $PARSER = new Parse::RecDescent($GRAMMAR);

sub print_usage
{
    print("Usage: rpcgen.pl -idl_file: <idl file to parse>  \n");    
}

sub get_output_path
{
    (my $fileName, my $outPath)=@_;
    if($outPath ne "")
    {
        return $outPath."/".$fileName;
    }
    return $fileName;
}

sub get_rpc_decl_file_name
{
    my $file_name=shift;
    $file_name=~s/^_?I//;
    $file_name="I".$file_name.".h";
    return $file_name;
}

sub get_rpc_enum_file_name
{
    my $file_name=shift;
    $file_name=~s/^_?I//;
    $file_name="".$file_name.".h";
    return $file_name;
}

sub get_rpc_data_file_name
{
    my $file_name=shift;
    $file_name=~s/^_?I//;
    $file_name="".$file_name.".h";
    return $file_name;
}

sub get_rpc_data_def_file_name
{
    my $file_name=shift;
    $file_name=~s/^_?I//;
    $file_name="".$file_name.".cpp";
    return $file_name;
}

sub parse_file
{
    (my $fileName)=@_;
    my $f;
    open($f,$fileName) or die("Failed open $fileName");
    undef $/;
    my $IDL_CONTENT=<$f>;
    close($f);

    my $project=$PARSER->Process($IDL_CONTENT);
    if(not $project)
    {
        die "Failed parse $fileName";
    }
    return $project;
}

sub print_method_parameters_C_new
{
    (my $hFile,my $method)=@_;
    my $result="";
    for my $arg (@{$method->{"arguments"}})
    {
        my $input_type=$arg->{"type"};

        my $is_wrappred_parameter = is_wrappred_parameter($arg);
        my $is_input = is_input_parameter($arg);
        my $is_output = is_output_parameter($arg);
        my $is_not_primary_type = none {$_ eq $input_type} @GengineGen::int_types;

        if($is_input and ($is_wrappred_parameter or $is_not_primary_type))
        {
            $result.="const ";
        }

        $result.=get_parameter_from_type($arg);

        if($is_input and ($is_wrappred_parameter or $is_not_primary_type))
        {
            $result.="&";
        }

        if($is_input)
        {
            $result.=" ";
        }
        elsif($is_output)
        {
            $result.="* ";
        }

        $result.=$arg->{"name"}.", ";
    }

    $result=~s/, $//;
    printf($hFile $result);
}

sub print_comment_if_any
{
    (my $hFile,my $arg)=@_;
    if($arg->{"annotations"}->{"Comment"})
    {
        print($hFile "//".$arg->{"annotations"}->{"Comment"});
    }
}

sub is_input_parameter
{
    (my $arg)=@_;
    return 1 if($arg->{"annotations"}->{"in"});
    return undef;
}

sub is_output_parameter
{
    (my $arg)=@_;
    return 1 if($arg->{"annotations"}->{"out"});
    return undef;
}

sub is_shared_ptr_parameter
{
    (my $arg)=@_;
    return 1 if($arg->{"annotations"}->{"P"});
    return undef;
}

sub is_unique_ptr_parameter
{
    (my $arg)=@_;
    return 1 if($arg->{"annotations"}->{"UP"});
    return undef;
}

sub is_vector_parameter
{
    (my $arg)=@_;
    return 1 if($arg->{"annotations"}->{"V"});
    return undef;
}

sub is_deque_parameter
{
    (my $arg)=@_;
    return 1 if($arg->{"annotations"}->{"D"});
    return undef;
}

sub is_list_parameter
{
    (my $arg)=@_;
    return 1 if($arg->{"annotations"}->{"L"});
    return undef;
}

sub is_optional_parameter
{
    (my $arg)=@_;
    return 1 if($arg->{"annotations"}->{"O"});
    return undef;
}

sub is_set_parameter
{
    (my $arg)=@_;
    return 1 if($arg->{"annotations"}->{"S"});
    return undef;
}

sub is_hash_set_parameter
{
    (my $arg)=@_;
    return 1 if($arg->{"annotations"}->{"HS"});
    return undef;
}

sub is_binary_serializable
{
    (my $arg)=@_;
    return 1 if($arg->{"annotations"}->{"B"});
    return undef;
}

sub is_json_serializable
{
    (my $arg)=@_;
    return 1 if($arg->{"annotations"}->{"J"});
    return undef;
}

sub is_serializable
{
    (my $arg)=@_;
    return is_json_serializable($arg) || is_binary_serializable($arg);
}

sub is_map_parameter
{
    (my $arg)=@_;
    return 1 if($arg->{"annotations"}->{"M"});
    return undef;
}

sub is_hash_map_parameter
{
    (my $arg)=@_;
    return 1 if($arg->{"annotations"}->{"HM"});
    return undef;
}

sub has_namespace
{
    (my $arg)=@_;
    return 1 if($arg->{"annotations"}->{"namespace"});
    return undef;
}

sub has_children
{
    (my $struct)=@_;
    return 1 if($struct->{"children"});
    return undef;
}

sub get_children
{
    (my $struct)=@_;
    return $struct->{"children"};
}

sub get_namespace
{
    (my $arg)=@_;
    return $arg->{"annotations"}->{"namespace"};
}

sub get_output_directory
{
    (my $arg)=@_;
    return $arg->{"annotations"}->{"Dir"};
}

sub get_map_key_parameter
{
    (my $arg)=@_;
    my $result =  get_parameter_from_type_internal($arg->{"annotations"}->{"M"});
    #$result = get_parameter_from_type_with_namespace($arg, $result);
    return $result;
}

sub get_hash_map_key_parameter
{
    (my $arg)=@_;
    my $result =  get_parameter_from_type_internal($arg->{"annotations"}->{"HM"});
    #$result = get_parameter_from_type_with_namespace($arg, $result);
    return $result;
}

sub is_supported_type
{
    (my $arg)=@_;
    my $input_type=$arg->{"type"};
    return 1 if(any {$_ eq $input_type} @GengineGen::plain_types);
    return 1 if(is_binary_serializable($arg));
    return 1 if(is_json_serializable($arg));
    return undef;
}

sub is_wrappred_parameter
{
    (my $arg)=@_;
    return 1 if(is_vector_parameter($arg));
    return 1 if(is_deque_parameter($arg));
    return 1 if(is_list_parameter($arg));
    return 1 if(is_set_parameter($arg));
    return 1 if(is_hash_set_parameter($arg));
    return 1 if(is_map_parameter($arg));
    return 1 if(is_hash_map_parameter($arg));
    return undef;
}

sub get_parameter_from_type
{
    (my $arg)=@_;
    my $is_containeer = is_wrappred_parameter($arg);
    my $type=get_parameter_from_type_internal_wrapped($arg);
    return "std::vector<".$type.">" if(is_vector_parameter($arg));
    return "std::deque<".$type.">" if(is_deque_parameter($arg));
    return "std::list<".$type.">" if(is_list_parameter($arg));
    return "std::set<".$type.">" if(is_set_parameter($arg));
    return "std::unordered_set<".$type.">" if(is_hash_set_parameter($arg));
    return "std::map<".get_map_key_parameter($arg).", ".$type.">" if(is_map_parameter($arg));
    return "std::unordered_map<".get_hash_map_key_parameter($arg).", ".$type.">" if(is_hash_map_parameter($arg));
    return $type;
}

sub get_parameter_from_type_internal_wrapped
{
    (my $arg)=@_;
    my $input_type=$arg->{"type"};
    my $result = get_parameter_from_type_internal($input_type);
    $result = get_parameter_from_type_with_namespace($arg, $result);
    return "std::shared_ptr<".$result.">" if(is_shared_ptr_parameter($arg));
    return "std::unique_ptr<".$result.">" if(is_unique_ptr_parameter($arg));
    return "boost::optional<".$result.">" if(is_optional_parameter($arg));
    return $result;
}

sub get_parameter_from_type_internal
{
    (my $input_type)=@_;
    my $result = $input_type;
    $result ="std::int8_t"   if($input_type eq "int8");
    $result ="std::int16_t"  if($input_type eq "int16");
    $result ="std::int32_t"  if($input_type eq "int32");
    $result ="std::int64_t"  if($input_type eq "int64");
    $result ="std::uint8_t"  if($input_type eq "uint8");
    $result ="std::uint16_t" if($input_type eq "uint16");
    $result ="std::uint32_t" if($input_type eq "uint32");
    $result ="std::uint64_t" if($input_type eq "uint64");
    $result ="bool"          if($input_type eq "bool");
    $result ="std::wstring"  if($input_type eq "WString");
    $result ="std::string"   if($input_type eq "String");
    $result ="Gengine::Blob" if($input_type eq "BLOBOBJ");
    $result ="void*" if($input_type eq "Ptr");
    return $result;
}

sub get_parameter_from_type_with_namespace
{
    (my $arg, my $result)=@_;
    $result = get_namespace($arg)."::".$result if(has_namespace($arg));
    return $result;
}

sub get_parameter_enum_from_type
{
    (my $arg)=@_;
    my $input_type=$arg->{"type"};
    return "ParametersTypes::Container" if(is_vector_parameter($arg));
    return "ParametersTypes::Container" if(is_deque_parameter($arg));
    return "ParametersTypes::Container" if(is_list_parameter($arg));
    return "ParametersTypes::Container" if(is_set_parameter($arg));
    return "ParametersTypes::Container" if(is_hash_set_parameter($arg));
    return "ParametersTypes::Map" if(is_map_parameter($arg));
    return "ParametersTypes::Map" if(is_hash_map_parameter($arg));
    return "ParametersTypes::Map" if(is_hash_map_parameter($arg));
    return "ParametersTypes::BinarySerializable" if(is_binary_serializable($arg));
    return "ParametersTypes::JsonSerializable" if(is_json_serializable($arg));
    return get_parameter_enum_from_type_internal($input_type);
}

sub get_parameter_enum_from_type_internal
{
    (my $input_type)=@_;
    my $result = $input_type;
    $result ="ParametersTypes::Int8"     if($input_type eq "int8");
    $result ="ParametersTypes::Int16"    if($input_type eq "int16");
    $result ="ParametersTypes::Int32"    if($input_type eq "int32");
    $result ="ParametersTypes::Int64"    if($input_type eq "int64");
    $result ="ParametersTypes::UInt8"    if($input_type eq "uint8");
    $result ="ParametersTypes::UInt16"   if($input_type eq "uint16");
    $result ="ParametersTypes::UInt32"   if($input_type eq "uint32");
    $result ="ParametersTypes::UInt64"   if($input_type eq "uint64");
    $result ="ParametersTypes::Boolean"     if($input_type eq "bool");
    $result ="ParametersTypes::WideString"  if($input_type eq "WString");
    $result ="ParametersTypes::String"   if($input_type eq "String");
    $result ="ParametersTypes::Blob"     if($input_type eq "BLOBOBJ");
    $result ="ParametersTypes::RawPtr"     if($input_type eq "Ptr");
    return $result;
}

sub create_file 
{
    my($filePath) = $_[1];
    open($_[0],'>',$filePath) or die ("Failed create file");
}
