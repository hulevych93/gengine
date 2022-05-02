#!/usr/bin/perl
package RPCGenerator;

use strict;
use warnings;
use Digest::MD5 qw(md5 md5_hex md5_base64);
use File::Path;
use File::Basename;
use Cwd 'abs_path';
use List::MoreUtils qw(uniq);
use List::Util qw(none any);

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
    my $client = 2;
    my $num_args=$#ARGV+1;
    my $idl_file_name="";
    my $out_dir_path="";
    my $iface_name="";
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
        if($opt_name eq "-iface:")
        {
            $iface_name=$opt_val;
            $i++;#skip next arg
            next;
        }
        if($opt_name eq "-client")
        {
            $client = 1;
            $i++;#skip next arg
            next;
        }
        if($opt_name eq "-server")
        {
            $client = 0;
            $i++;#skip next arg
            next;
        }
    }
    if(not $idl_file_name or not $iface_name)
    {
        print_usage();
        die("Not all command line arguments are set properly");
    }

    parse($idl_file_name, $out_dir_path, $client, $iface_name);
}

sub parse
{
    (my $idl_file_name, my $out_dir_path, my $client, my $iface_name)=@_;
    my $project=GengineGen::parse_file($idl_file_name);
    
    my $intemidiate_dir = GengineGen::get_output_directory($project);
    
    for my $interface(@{$project->{"interfaces"}})
    {
        if($interface->{"name"} eq $iface_name)
        {
            if($client eq 1)
            {
                output_client_implementation($interface, $project->{"name"}, $out_dir_path, $intemidiate_dir);
                output_client_dummy_implementation($interface, $project->{"name"}, $out_dir_path, $intemidiate_dir);
            }
            elsif($client eq 0)
            {
                output_executer_implementation($interface, $project->{"name"}, $out_dir_path, $intemidiate_dir);
            }
        }
    }
}

sub get_method_index
{
    (my $interface, my $method)=@_;
    my $index=0;
    for my $thatMethod (@{$interface->{"methods"}})
    {
        if($method->{"name"} eq $thatMethod->{"name"})
        {
            my @methodAguments = @{$method->{"arguments"}};
            my @thatArguments = @{$thatMethod->{"arguments"}};
            if(scalar(@methodAguments) eq scalar(@thatArguments))
            {
                my $matched = 1;
                for (my $i = 0; $i < scalar(@methodAguments) ; $i++) 
                {
                    if(($methodAguments[$i]->{"name"} ne $thatArguments[$i]->{"name"}) or
                       ($methodAguments[$i]->{"type"} ne $thatArguments[$i]->{"type"}) or
                       ($methodAguments[$i]->{"annotations"} ne $thatArguments[$i]->{"annotations"}))
                    {
                        $matched = 0;
                        last;
                    }
                }
                if($matched)
                {
                    return $index;
                }
            }
        }
        $index++;
    }
    return undef;
}

sub output_interface_method_enumeration
{
    (my $hFile, my $interface)=@_;
    my $interface_name=$interface->{"name"};
    
    printf($hFile "enum  class ".$interface_name."IDs : std::uint8_t\n");
    printf($hFile "{\n");
    my $index=0;
    for my $method (@{$interface->{"methods"}})
    {
        my $member= "ID_".$method->{"name"}.$index."=".$index;
        printf($hFile "    $member");
        if($index==$#{$interface->{"methods"}})
        {#last member - issue just line break
            printf($hFile "\n");
        }
        else
        {#not a last member - issue comma with line break
            printf($hFile ",\n");
        }
        $index++;
    }
    printf($hFile "};\n");
}

sub output_client_dummy_implementation
{
    (my $interface, my $namespace, my $out_dir_path, my $intemidiate_dir)=@_;
    my $out_dir_path_2 = GengineGen::get_output_path($intemidiate_dir, $out_dir_path);
    my $interface_name=$interface->{"name"};
    my $file_name=get_stub_dummy_class_file_name($interface_name);
    my $decl_file_name=GengineGen::get_rpc_decl_file_name($interface_name);
    $decl_file_name=GengineGen::get_output_path($decl_file_name, $out_dir_path_2);
    my $hFile;
    GengineGen::create_file($hFile,$file_name);
    
    my $class_name=get_stub_dummy_class_name($interface_name);
    #header
    printf($hFile "#include <core/AbstractFactory.h>\n");
    printf($hFile "#include <brokers/ServiceBroker.h>\n");
    printf($hFile "#include <core/Logger.h>\n\n");
    printf($hFile "#include \"${decl_file_name}\"\n");
    print_custom_types_includes($hFile,$interface->{"methods"}, $out_dir_path, $intemidiate_dir);
    printf($hFile "\n");
    printf($hFile "using namespace Gengine;\n");
    printf($hFile "using namespace InterprocessCommunication;\n");
    printf($hFile "using namespace Services;\n\n");
    printf($hFile "namespace ".$namespace."{\n");
    printf($hFile "class ${class_name}: public ".$interface_name."\n");
    printf($hFile "{\n");
    printf($hFile "public:\n");
    #interface implementation
    for my $method(@{$interface->{"methods"}})
    {
        my $index = get_method_index($interface, $method);
        printf($hFile "\n");
        my @input_parameters=get_parameters_by_method_new($method,"in");
        my @output_parameters=get_parameters_by_method_new($method,"out");
        printf($hFile "bool ".$method->{"name"}."(");
        GengineGen::print_method_parameters_C_new($hFile,$method);
        printf($hFile ")\n");
        printf($hFile "{
    GLOG_WARNING_INTERNAL(\"Method not implemented\");
    return false;\n}\n");
    }
    printf($hFile "};\n\n");
    printf($hFile "class ${class_name}Creator: public IAbstractCreator<IMicroService, const interface_key&>\n");
    printf($hFile "{\n");
    printf($hFile "public:\n");
    printf($hFile "std::shared_ptr<IMicroService>  Create(const interface_key&) const override\n");
    printf($hFile "{\n");
    printf($hFile "    return std::make_shared<${class_name}>();\n");
    printf($hFile "}\n");
    printf($hFile "};\n\n");
    printf($hFile "REGISTER_SERVICE(\"$interface_name\", ");
    printf($hFile " (std::make_shared<${class_name}Creator>()), ${class_name}, ServiceType::Null) \n");
    printf($hFile "\n}");
    close($hFile);
}

sub output_client_implementation
{
    (my $interface, my $namespace, my $out_dir_path, my $intemidiate_dir)=@_;
    my $out_dir_path_2  = GengineGen::get_output_path($intemidiate_dir, $out_dir_path);
    my $interface_name=$interface->{"name"};
    my $file_name=get_stub_class_file_name($interface_name);
    my $decl_file_name=GengineGen::get_rpc_decl_file_name($interface_name);
    $decl_file_name=GengineGen::get_output_path($decl_file_name, $out_dir_path_2);
    my $hFile;
    GengineGen::create_file($hFile,$file_name);
    
    my $class_name=get_stub_class_name($interface_name);
    #header
    printf($hFile "#include <core/AbstractFactory.h>\n");
    printf($hFile "#include <brokers/ServiceBroker.h>\n");
    printf($hFile "#include <interprocess-communication/InputParameters.h>\n");
    printf($hFile "#include <interprocess-communication/OutputParameters.h>\n");
    printf($hFile "#include <interprocess-communication/InterprocessClient.h>\n");
    printf($hFile "#include <serialization/Serializer.h>\n");
    printf($hFile "#include <serialization/Deserializer.h>\n");
    printf($hFile "#include <core/Logger.h>\n\n");
    printf($hFile "#include <${decl_file_name}>\n");
    print_custom_types_includes($hFile,$interface->{"methods"}, $out_dir_path, $intemidiate_dir);
    printf($hFile "\n");
    printf($hFile "using namespace Gengine;\n");
    printf($hFile "using namespace InterprocessCommunication;\n");
    printf($hFile "using namespace Serialization;\n");
    printf($hFile "using namespace Services;\n\n");
    printf($hFile "namespace ".$namespace."{\n");
    output_interface_method_enumeration($hFile,$interface);
    printf($hFile "class ${class_name}: public ".$interface_name.",\n");
    printf($hFile "    public InterprocessClient\n");
    printf($hFile "{\n");
    printf($hFile "public:\n");
    printf($hFile "${class_name}(const interface_key& key)\n");
    printf($hFile ": m_key(key)\n");
    printf($hFile "{}\n");
    #interface implementation
    for my $method(@{$interface->{"methods"}})
    {
        my $index = get_method_index($interface, $method);
        printf($hFile "\n");
        my @input_parameters=get_parameters_by_method_new($method,"in");
        my @output_parameters=get_parameters_by_method_new($method,"out");
        printf($hFile "bool ".$method->{"name"}."(");
        GengineGen::print_method_parameters_C_new($hFile,$method);
        printf($hFile ")\n");
        printf($hFile "{
    OutputParameters arguments;
    const auto uiFunctionID=static_cast<std::uint8_t>(".$interface_name."IDs::ID_".$method->{"name"}.$index.");\n");

        print_append_input_parameters($hFile, @input_parameters);

        my $output_parameters_count=scalar(@output_parameters);
        if($output_parameters_count > 0)
        {
            printf($hFile "    InputParameters results;
    const std::uint32_t uiOutputParametersCount=".$output_parameters_count.";
    if(!SendRequest(m_key, uiFunctionID, results, arguments))
    {
        GLOG_WARNING_INTERNAL(\"Failed send request\");
        return false;
    }
    if(results.GetParametersCount()!=uiOutputParametersCount)
    {
        assert(0 && \"Invalid output parameters count\");
        return false;
    }\n");

          print_get_output_parameters($hFile, @output_parameters);
      }
      else
      {
          printf($hFile "    if(!SendEvent(m_key, uiFunctionID, arguments))
    {
        GLOG_WARNING_INTERNAL(\"Failed send event\");
        return false;
    }\n");
      }
      printf($hFile "    return true;\n");
      printf($hFile "}\n");
    }
    printf($hFile "\nprivate:\n");
    printf($hFile "    interface_key m_key;\n");
    printf($hFile "};\n\n");
    printf($hFile "class ${class_name}Creator: public IAbstractCreator<IMicroService, const interface_key&>\n");
    printf($hFile "{\n");
    printf($hFile "public:\n");
    printf($hFile "std::shared_ptr<IMicroService>  Create(const interface_key& key) const override\n");
    printf($hFile "{\n");
    printf($hFile "    return std::make_shared<${class_name}>(key);\n");
    printf($hFile "}\n");
    printf($hFile "};\n\n");
    printf($hFile "REGISTER_SERVICE(\"$interface_name\", ");
    printf($hFile " (std::make_shared<${class_name}Creator>()), ${class_name}, ServiceType::Remote) \n");
    if(!is_any_out_parameters_exist($interface))
    {
        my $class_name=get_stub_composite_class_name($interface_name);
        printf($hFile "\n\n");
        printf($hFile "class ${class_name}: public ".$namespace."::".$interface_name.",\n");
        printf($hFile "    public ICompositeClient\n");
        printf($hFile "{\n");
        printf($hFile "public:\n");
        printf($hFile "void RegisterService(const TService& service) override\n{\n");
        printf($hFile "    std::lock_guard<std::mutex> locker(m_mutex);\n");
        printf($hFile "    m_clients.emplace_back(std::dynamic_pointer_cast<".$interface_name.">(service));\n}\n");
        printf($hFile "void UnregisterService(const TService& service) override\n{\n");
        printf($hFile "    std::lock_guard<std::mutex> locker(m_mutex);\n");
        printf($hFile "    auto clientIter = std::find_if(m_clients.begin(), m_clients.end(), [service](const std::shared_ptr<".$interface_name.">& item) { return item.get() == service.get(); });\n");
        printf($hFile "    if(clientIter != m_clients.end()) m_clients.erase(clientIter);\n}\n");
        printf($hFile "std::size_t Count() override\n{\n");
        printf($hFile "    std::lock_guard<std::mutex> locker(m_mutex);\n");
        printf($hFile "    return m_clients.size();\n}\n");
        #interface implementation
        for my $method(@{$interface->{"methods"}})
        {
            my $index = get_method_index($interface, $method);
            printf($hFile "\n");
            my @input_parameters=get_parameters_by_method_new($method,"in");
            my @output_parameters=get_parameters_by_method_new($method,"out");
            printf($hFile "bool ".$method->{"name"}."(");
            GengineGen::print_method_parameters_C_new($hFile,$method);
            printf($hFile ") override\n");
            printf($hFile "{\n");
            printf($hFile "    std::lock_guard<std::mutex> locker(m_mutex);\n");
            printf($hFile "    auto result = false;\n");
            printf($hFile "    for(auto& client: m_clients)\n");
            printf($hFile "        result |= client->".$method->{"name"}."(");
            print_executer_call($method->{"arguments"},$hFile);
            printf($hFile ");\n");
            printf($hFile "    return result;\n}\n");
        }
        printf($hFile "\nprivate:\n");
        printf($hFile "    std::mutex m_mutex;\n");
        printf($hFile "    std::vector<std::shared_ptr<".$namespace."::".$interface_name.">> m_clients;\n");
        printf($hFile "};\n\n");
        printf($hFile "class ${class_name}Creator: public IAbstractCreator<IMicroService, const interface_key&>\n");
        printf($hFile "{\n");
        printf($hFile "public:\n");
        printf($hFile "std::shared_ptr<IMicroService>  Create(const interface_key&) const override\n");
        printf($hFile "{\n");
        printf($hFile "    return std::make_shared<${class_name}>();\n");
        printf($hFile "}\n");
        printf($hFile "};\n\n");
        printf($hFile "REGISTER_SERVICE(\"$interface_name\", ");
        printf($hFile " (std::make_shared<${class_name}Creator>()), ${class_name}, ServiceType::Composite) \n");
    }
    printf($hFile "\n}");
    close($hFile);
}

sub output_executer_implementation
{
    (my $interface, my $namespace, my $out_dir_path, my $intemidiate_dir)=@_;
    my $out_dir_path_2   = GengineGen::get_output_path($intemidiate_dir, $out_dir_path);
    my $interface_name=$interface->{"name"};
    my $file_name=get_executer_class_file_name($interface_name);
    my $decl_file_name=GengineGen::get_rpc_decl_file_name($interface_name);
    $decl_file_name=GengineGen::get_output_path($decl_file_name, $out_dir_path_2);
    my $hFile;
    GengineGen::create_file($hFile,$file_name);
    
    my $class_name=get_executer_class_name($interface_name);
    #header
    printf($hFile "#include <${decl_file_name}>\n");
    printf($hFile "#include <core/AbstractFactory.h>\n");
    printf($hFile "#include <interprocess-communication/InterfaceExecutor.h>\n");
    printf($hFile "#include <interprocess-communication/InputParameters.h>\n");
    printf($hFile "#include <interprocess-communication/OutputParameters.h>\n");
    printf($hFile "#include <brokers/ExecutorBroker.h>\n");
    printf($hFile "#include <serialization/Serializer.h>\n");
    printf($hFile "#include <serialization/Deserializer.h>\n");
    printf($hFile "#include <core/Logger.h>\n");
    print_custom_types_includes($hFile,$interface->{"methods"}, $out_dir_path, $intemidiate_dir);  printf($hFile "\n");
    printf($hFile "using namespace Gengine;\n");
    printf($hFile "using namespace InterprocessCommunication;\n");
    printf($hFile "using namespace Serialization;\n");
    printf($hFile "using namespace Services;\n");
    printf($hFile "namespace ".$namespace." {\n");
    output_interface_method_enumeration($hFile,$interface);
    my $isExecutor = is_any_out_parameters_exist($interface);
    if($isExecutor)
    {
        printf($hFile "class ${class_name}: public InterfaceExecutor\n");
    }
    else
    {
        printf($hFile "class ${class_name}: public InterfaceListener\n");
    }
    printf($hFile "{\n");
    printf($hFile "public:\n");
    printf($hFile "${class_name}(const interface_key& key, IMicroService& service)\n");
    printf($hFile ": m_interface(dynamic_cast<${namespace}::${interface_name}&>(service))\n");
    printf($hFile ", m_key(key)\n");
    printf($hFile "{}\n\n");
    printf($hFile "interface_key GetInterface() const override\n");
    printf($hFile "{\n");
    printf($hFile "    return m_key;\n");
    printf($hFile "}\n\n");
    if($isExecutor)
    {
        print_executer_request_handler($hFile, $interface);
        print_executer_request_handlers_implementations($hFile, $interface);
    }
    {
        print_executer_event_handler($hFile, $interface);
        print_executer_event_handlers_implementations($hFile, $interface);
    }
    printf($hFile "\nprivate:\n");
    printf($hFile "    ${namespace}::${interface_name}& m_interface;\n");
    printf($hFile "    interface_key m_key;\n");
    printf($hFile "};\n\n");
    printf($hFile "class ${class_name}Creator: public IAbstractCreator<InterfaceImpl, const interface_key&, IMicroService&>\n");
    printf($hFile "{\n");
    printf($hFile "public:\n");
    printf($hFile "std::shared_ptr<InterfaceImpl>  Create(const interface_key& key, IMicroService& service) const override\n");
    printf($hFile "{\n");
    printf($hFile "    return std::make_shared<${class_name}>(key, service);\n");
    printf($hFile "}\n");
    printf($hFile "};\n\n");
    printf($hFile "REGISTER_EXECUTOR(\"$interface_name\", ");
    printf($hFile "(std::make_shared<${class_name}Creator>()), ${class_name}) \n}");
    close($hFile);
}

sub get_all_event_methods
{
    (my $interface)=@_;
    my $method;
    my @result_methods;
    for $method(@{$interface->{"methods"}})
    {
        my @output_parameters=get_parameters_by_method_new($method,"out");
        my $output_parameters_count=scalar(@output_parameters);
        if($output_parameters_count == 0)
        {
            push(@result_methods, $method);
        }
    }
    return @result_methods;
}

sub get_all_request_methods
{
    (my $interface)=@_;
    my $method;
    my @result_methods;
    for $method(@{$interface->{"methods"}})
    {
        my @output_parameters=get_parameters_by_method_new($method,"out");
        my $output_parameters_count=scalar(@output_parameters);
        if($output_parameters_count > 0)
        {
            push(@result_methods,$method);
        }
    }
    return @result_methods;
}

sub is_any_out_parameters_exist
{
    (my $interface)=@_;
    my $method;
    for $method(@{$interface->{"methods"}})
    {
        my @output_parameters=get_parameters_by_method_new($method,"out");
        my $output_parameters_count=scalar(@output_parameters);
        if($output_parameters_count > 0)
        {
             return 1;
        }
    }
    return undef;
}

sub get_parameters_by_method_new
{
    (my $method,my $parameters_req_method)=@_;
    my @result_parameters;
    if(not($parameters_req_method eq "in" or $parameters_req_method eq "out"))
    {
        die("Unknown parameters type");
    }
    for my $arg(@{$method->{"arguments"}})
    {
        if($arg->{"annotations"}->{$parameters_req_method})
        {
            push(@result_parameters,$arg);
        }
    }
    return @result_parameters;
}

sub print_append_input_parameters
{
    (my $hFile,my @input_parameters)=@_;
    my $input_parameter;
    foreach $input_parameter(@input_parameters)
    {
        my $parameter_name=$input_parameter->{"name"};
        if(GengineGen::is_supported_type($input_parameter))
        {
            printf($hFile "    arguments.Append(".$parameter_name.");");
        }
        else
        {
            printf "Unsupported argument type...\n";
            die;
        }
        printf($hFile "\n");
    }
}

sub print_append_output_parameters
{
    (my $hFile,my @output_parameters)=@_;
    my $output_parameter;
    foreach $output_parameter(@output_parameters)
    {
        my $parameter_name=$output_parameter->{"name"};
        if(GengineGen::is_supported_type($output_parameter))
        {
            printf($hFile "        outputs->Append(".$parameter_name.");");
        }
        else
        {
            printf "Unsupported argument type...\n";
            die;
        }
        printf($hFile "\n");
    }
}

sub print_declare_output_parameters
{
    (my $hFile,my @output_parameters)=@_;
    my $output_parameter;
    foreach $output_parameter(@output_parameters)
    {
        printf($hFile "    ");
        my $parameter_type=$output_parameter->{"type"};
        my $parameter_name=$output_parameter->{"name"};
        printf($hFile GengineGen::get_parameter_from_type($output_parameter)." ".$parameter_name.";");
        printf($hFile "\n");
    }
}

sub print_get_input_parameters
{
    (my $hFile,my @input_parameters)=@_;
    my $input_parameter;
    foreach $input_parameter(@input_parameters)
    {
        printf($hFile "    ");
        my $parameter_type=$input_parameter->{"type"};
        my $parameter_name=$input_parameter->{"name"};
        my $type = GengineGen::get_parameter_from_type($input_parameter);
        my $type_enum = GengineGen::get_parameter_enum_from_type($input_parameter);

        if(GengineGen::is_supported_type($input_parameter))
        {
            printf($hFile $type ." ". $parameter_name. ";
    if(inputs->GetParameterHeader(iParameterCounter)->parameterType!=".$type_enum.")
    {
        GLOG_WARNING_INTERNAL(\"Argument expected to be ".$type."\");
        return ResponseCodes::ParametersMismatch;
    }
    inputs->Get(iParameterCounter, ".$parameter_name.");
    iParameterCounter++;");
        }
        else
        {
            printf "Unsupported argument type...\n";
            die;
        }
        printf($hFile "\n");
    }
}

sub print_interface_method_enumeration
{
    (my $hFile, my $interface)=@_;
    my $interface_name=$interface->{"name"};
    my $enumName=$interface_name."IDs";
    printf($hFile "enum  class ".$interface_name."IDs : std::uint8_t\n");
    printf($hFile "{\n");
    for my $method (@{$interface->{"methods"}})
    {
        my $index = get_method_index($interface, $method);
        my $member= $interface_name."IDs::ID_".$method->{"name"}."=".$index;
        printf($hFile "    $member");
        if($index==$#{$interface->{"methods"}})
        {#last member - issue just line break
            printf($hFile "\n");
        }
        else
        {#not a last member - issue comma with line break
            printf($hFile ",\n");
        }
    }
    printf($hFile "};\n");
}

sub print_get_output_parameters
{
    (my $hFile,my @output_parameters)=@_;
    my $output_parameter;
    foreach $output_parameter(@output_parameters)
    {
        printf($hFile "    int iCurrentOutArgument=0;");
        my $parameter_type=$output_parameter->{"type"};
        my $parameter_name=$output_parameter->{"name"};
        my $type = GengineGen::get_parameter_from_type($output_parameter);
        my $type_enum = GengineGen::get_parameter_enum_from_type($output_parameter);

        if(GengineGen::is_supported_type($output_parameter))
        {
            printf($hFile "if(results.GetParameterHeader(iCurrentOutArgument)->parameterType!="."$type_enum".")
    {
        assert(0 && \"Argument is not ".$type."\");
        return false;
    }
    results.Get(iCurrentOutArgument, *".$parameter_name.");
    iCurrentOutArgument++;");
        }
        else
        {
            printf "Unsupported argument type...\n";
            die;
        }
        printf($hFile "\n");
    }
}

sub print_executer_request_handler
{
    (my $hFile,my $interface)=@_;
    my $interface_name=$interface->{"name"};
    printf($hFile "ResponseCodes HandleRequest(std::uint8_t functionCode,
                    const std::shared_ptr<const InputParameters>& inputs,
                    const std::shared_ptr<OutputParameters>& outputs) override
{
    switch(functionCode)
    {\n");
    #handler routine implementation
    my $method;
    for $method(get_all_request_methods($interface))
    {
        my $index = get_method_index($interface, $method);
        printf($hFile "    case (int)".$interface_name."IDs::ID_".$method->{"name"}.$index.":
        return Handle".$method->{"name"}.$index."(inputs,outputs);\n");
    }
    #handler routine end
    printf($hFile "    default:
        //unknown request
        GLOG_ERROR(\"Unknown request %%08X\",functionCode);
        return ResponseCodes::UnknownFunction;
    }
}\n\n");
}

sub print_executer_request_handlers_implementations
{
    (my $hFile,my $interface)=@_;

    my $method;
    for $method(get_all_request_methods($interface))
    {
        my $index = get_method_index($interface, $method);
        my @input_parameters=get_parameters_by_method_new($method,"in");
        my $input_parameters_count=scalar(@input_parameters);
        printf($hFile "ResponseCodes Handle".$method->{"name"}.$index."(const std::shared_ptr<const InputParameters>& inputs,
                const std::shared_ptr<OutputParameters>& outputs)
{
    if(inputs->GetParametersCount()!=$input_parameters_count)
    {
        GLOG_WARNING_INTERNAL(\"$input_parameters_count parameters expected;\");
        assert(0 && \"Input parameters count mismatch!\");
        return ResponseCodes::ParametersMismatch;
    }
    int iParameterCounter=0;\n");
        #get input parameters
        print_get_input_parameters($hFile, @input_parameters);
        my @output_parameters=get_parameters_by_method_new($method,"out");
        #declare output parameters
        print_declare_output_parameters($hFile, @output_parameters);
        #execute
        print($hFile "    auto result = m_interface.".$method->{"name"}."(");
        print_executer_call($method->{"arguments"},$hFile);
        print($hFile ");\n");
        print($hFile "    if(result)
    {\n");
        #obtain output parameters
        print_append_output_parameters($hFile, @output_parameters);
        print($hFile "    }\n");
        #done
        printf($hFile "    return result ? ResponseCodes::Ok : ResponseCodes::RequestError;\n}\n\n");
    }
}

sub print_executer_event_handler
{
    (my $hFile,my $interface)=@_;
    my $interface_name=$interface->{"name"};

    #handler routine implementation
    my $method;
    my @event_methods = get_all_event_methods($interface);
    my $event_methods_count=scalar(@event_methods);
    if($event_methods_count > 0)
    {
        printf($hFile "ResponseCodes HandleEvent(std::uint8_t functionCode,
                    const std::shared_ptr<const InputParameters>& inputs) override
{
    switch(functionCode)
    {\n");
        for $method(get_all_event_methods($interface))
        {
            my $index = get_method_index($interface, $method);
            printf($hFile "    case (int)".$interface_name."IDs::ID_".$method->{"name"}.$index.":
        return Handle".$method->{"name"}.$index."(inputs);\n");
        }
            #handler routine end
    printf($hFile "    default:
        //unknown request
        GLOG_ERROR(\"Unknown event %%08X\",functionCode);
        return ResponseCodes::UnknownFunction;
    }
}\n\n");
    }
    else
    {
        printf($hFile "ResponseCodes HandleEvent(std::uint8_t functionCode,
                    const std::shared_ptr<const InputParameters>& inputs) override
{
    GLOG_ERROR(\"Unknown event %%08X\",functionCode);
    return ResponseCodes::UnknownFunction;
}\n\n");
    }
}

sub print_executer_event_handlers_implementations
{
    (my $hFile,my $interface)=@_;

    my $method;
    for $method(get_all_event_methods($interface))
    {
        my $index = get_method_index($interface, $method);
        my @input_parameters=get_parameters_by_method_new($method,"in");
        my $input_parameters_count=scalar(@input_parameters);
        printf($hFile "ResponseCodes Handle".$method->{"name"}.$index."(const std::shared_ptr<const InputParameters>& inputs)
{
    if(inputs->GetParametersCount()!=$input_parameters_count)
    {
        GLOG_WARNING_INTERNAL(\"$input_parameters_count parameters expected;\");
        assert(0 && \"Input parameters count mismatch!\");
        return ResponseCodes::ParametersMismatch;
    }
    int iParameterCounter=0;\n");

        print_get_input_parameters($hFile, @input_parameters);
        #execute
        print($hFile "    auto result = m_interface.".$method->{"name"}."(");
        print_executer_call($method->{"arguments"},$hFile);
        print($hFile ");\n");
        #done
        printf($hFile "    return result ? ResponseCodes::Ok : ResponseCodes::RequestError;\n}\n\n");
    }
}

sub print_executer_call
{
    (my $parameters,my $hFile)=@_;
    my $result="";
    for my $arg(@{$parameters})
    {
        my $parameter_name=$arg->{"name"};
        if($arg->{"annotations"}->{"in"})
        {#input parameter
            $result.=$arg->{"name"};
        }
        elsif($arg->{"annotations"}->{"out"})
        {#output parameter
            $result.="&".$parameter_name;
        }
        else
        {
            die("Unknown parameter method for ".$parameter_name);
        }
        $result.=", ";
    }
    $result=~s/, $//;
    print $hFile $result;
}

sub print_custom_types_includes
{
    (my $hFile,my $methods, my $out_dir_name, my $intemidiate_dir)=@_;
    my $result="";
    my @ctypes;
    for my $method (@{$methods})
    {
        for my $arg (@{$method->{"arguments"}})
        {
            my $intemidiate_dir_2 = $intemidiate_dir;
            $intemidiate_dir_2 = $arg->{"annotations"}->{"Include"} if($arg->{"annotations"}->{"Include"});
            my $out_dir_path = GengineGen::get_output_path($intemidiate_dir_2, $out_dir_name);
            my $input_type=$arg->{"type"};
            if(none {$_ eq $input_type} @GengineGen::plain_types)
            {
                $input_type=GengineGen::get_output_path($input_type, $out_dir_path);
                push @ctypes, $input_type;
            }
        }
    }
    my @unique_types=uniq(@ctypes);
    for my $utype (@{unique_types})
    {
        $result.="#include <$utype.h>\n";
    }
    printf($hFile $result);
}

sub get_stub_class_name
{
    my $class_name=shift;
    $class_name=~s/^_?I//;
    $class_name=$class_name."Client";
    return $class_name;
}

sub get_stub_dummy_class_name
{
    my $class_name=shift;
    $class_name=~s/^_?I//;
    $class_name=$class_name."DummyClient";
    return $class_name;
}

sub get_stub_composite_class_name
{
    my $class_name=shift;
    $class_name=~s/^_?I//;
    $class_name=$class_name."CompositeClient";
    return $class_name;
}

sub get_stub_class_file_name
{
    my $file_name=shift;
    $file_name=~s/^_?I//;
    $file_name=$file_name."Client.cpp";
    return $file_name;
}

sub get_stub_dummy_class_file_name
{
    my $file_name=shift;
    $file_name=~s/^_?I//;
    $file_name=$file_name."DummyClient.cpp";
    return $file_name;
}

sub get_executer_class_name
{
    my $class_name=shift;
    $class_name=~s/^_?I//;
    $class_name=$class_name."RequestHandler";
    return $class_name;
}

sub get_executer_class_file_name
{
    my $file_name=shift;
    $file_name=~s/^_?I//;
    $file_name=$file_name."RequestHandler.cpp";
    return $file_name;
}
