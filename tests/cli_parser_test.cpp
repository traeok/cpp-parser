#include <cassert>
#include <iostream>
#include <sstream>
#include "../include/lexer.hpp"
#include "../include/parser.hpp"

// Helper function to parse a command line using pparser
parser::ParseResult parse_command_line(const std::string &input, parser::ArgumentParser &parser)
{
    return parser.parse(input);
}

// Test basic flag parsing
void testBasicFlags()
{
    std::cout << "\nTesting basic flags...\n";
    parser::ArgumentParser parser("test", "Test CLI parser");
    
    parser::Command &root = parser.get_root_command();
    root.add_keyword_arg("verbose", parser::make_aliases("-v", "--verbose"), "enable verbose output", parser::ArgType_Flag);
    root.add_keyword_arg("debug", parser::make_aliases("-d", "--debug"), "enable debug mode", parser::ArgType_Flag);
    
    // Test short flags
    parser::ParseResult result = parse_command_line("-v -d", parser);
    assert(result.status == parser::ParseResult::ParserStatus_Success);
    assert(result.find_kw_arg_bool("verbose") == true);
    assert(result.find_kw_arg_bool("debug") == true);
    
    // Test long flags
    result = parse_command_line("--verbose --debug", parser);
    assert(result.status == parser::ParseResult::ParserStatus_Success);
    assert(result.find_kw_arg_bool("verbose") == true);
    assert(result.find_kw_arg_bool("debug") == true);
    
    // Test combined short flags
    result = parse_command_line("-vd", parser);
    assert(result.status == parser::ParseResult::ParserStatus_Success);
    assert(result.find_kw_arg_bool("verbose") == true);
    assert(result.find_kw_arg_bool("debug") == true);
    
    std::cout << "Basic flags test passed!\n";
}

// Test flags with values
void testFlagsWithValues()
{
    std::cout << "\nTesting flags with values...\n";
    parser::ArgumentParser parser("test", "Test CLI parser");
    
    parser::Command &root = parser.get_root_command();
    root.add_keyword_arg("output", parser::make_aliases("-o", "--output"), "output file", parser::ArgType_Single);
    root.add_keyword_arg("count", parser::make_aliases("-c", "--count"), "count value", parser::ArgType_Single);
    
    // Test string value
    parser::ParseResult result = parse_command_line("-o test.txt", parser);
    assert(result.status == parser::ParseResult::ParserStatus_Success);
    assert(result.find_kw_arg_string("output") == "test.txt");
    
    // Test integer value
    result = parse_command_line("--count 42", parser);
    assert(result.status == parser::ParseResult::ParserStatus_Success);
    assert(result.find_kw_arg_int("count") == 42);
    
    // Test both together
    result = parse_command_line("-o result.txt -c 100", parser);
    assert(result.status == parser::ParseResult::ParserStatus_Success);
    assert(result.find_kw_arg_string("output") == "result.txt");
    assert(result.find_kw_arg_int("count") == 100);
    
    std::cout << "Flags with values test passed!\n";
}

// Test positional arguments
void testPositionalArguments()
{
    std::cout << "\nTesting positional arguments...\n";
    parser::ArgumentParser parser("test", "Test CLI parser");
    
    parser::Command &root = parser.get_root_command();
    root.add_positional_arg("input", "input file", parser::ArgType_Single, true);
    root.add_positional_arg("output", "output file", parser::ArgType_Single, false, parser::ArgValue(std::string("default.out")));
    
    // Test with both arguments
    parser::ParseResult result = parse_command_line("input.txt output.txt", parser);
    assert(result.status == parser::ParseResult::ParserStatus_Success);
    assert(result.find_pos_arg_string("input") == "input.txt");
    assert(result.find_pos_arg_string("output") == "output.txt");
    
    // Test with only required argument (optional should get default)
    result = parse_command_line("input.txt", parser);
    assert(result.status == parser::ParseResult::ParserStatus_Success);
    assert(result.find_pos_arg_string("input") == "input.txt");
    assert(result.find_pos_arg_string("output") == "default.out");
    
    std::cout << "Positional arguments test passed!\n";
}

// Test multiple values
void testMultipleValues()
{
    std::cout << "\nTesting multiple values...\n";
    
    // Test multiple keyword values
    {
        parser::ArgumentParser parser("test", "Test CLI parser");
        parser::Command &root = parser.get_root_command();
        root.add_keyword_arg("files", parser::make_aliases("-f", "--files"), "input files", parser::ArgType_Multiple);
        
        parser::ParseResult result = parse_command_line("-f file1.txt file2.txt file3.txt", parser);
        assert(result.status == parser::ParseResult::ParserStatus_Success);
        std::vector<std::string> files = result.find_kw_arg_list("files");
        assert(files.size() == 3);
        assert(files[0] == "file1.txt");
        assert(files[1] == "file2.txt");
        assert(files[2] == "file3.txt");
    }
    
    // Test multiple positional values
    {
        parser::ArgumentParser parser("test", "Test CLI parser");
        parser::Command &root = parser.get_root_command();
        root.add_positional_arg("sources", "source files", parser::ArgType_Multiple, true);
        
        parser::ParseResult result = parse_command_line("src1.cpp src2.cpp src3.cpp", parser);
        assert(result.status == parser::ParseResult::ParserStatus_Success);
        std::vector<std::string> sources = result.find_pos_arg_list("sources");
        assert(sources.size() == 3);
        assert(sources[0] == "src1.cpp");
        assert(sources[1] == "src2.cpp");
        assert(sources[2] == "src3.cpp");
    }
    
    std::cout << "Multiple values test passed!\n";
}

// Test subcommands
void testSubcommands()
{
    std::cout << "\nTesting subcommands...\n";
    parser::ArgumentParser parser("git", "Git version control");
    
    parser::Command &root = parser.get_root_command();
    
    // Create subcommands
#if defined(PARSER_USE_TR1_SHARED_PTR)
    parser::command_ptr add_cmd(new parser::Command("add", "Add files to staging"));
    parser::command_ptr commit_cmd(new parser::Command("commit", "Commit changes"));
#else
    parser::command_ptr add_cmd = std::make_shared<parser::Command>("add", "Add files to staging");
    parser::command_ptr commit_cmd = std::make_shared<parser::Command>("commit", "Commit changes");
#endif
    
    add_cmd->add_positional_arg("files", "files to add", parser::ArgType_Multiple, true);
    commit_cmd->add_keyword_arg("message", parser::make_aliases("-m", "--message"), "commit message", parser::ArgType_Single, true);
    commit_cmd->add_alias("ci"); // Add alias for commit
    
    root.add_command(add_cmd);
    root.add_command(commit_cmd);
    
    // Test add subcommand
    parser::ParseResult result = parse_command_line("add file1.txt file2.txt", parser);
    assert(result.status == parser::ParseResult::ParserStatus_Success);
    assert(result.command_path == "git add");
    std::vector<std::string> files = result.find_pos_arg_list("files");
    assert(files.size() == 2);
    assert(files[0] == "file1.txt");
    assert(files[1] == "file2.txt");
    
    // Test commit subcommand
    result = parse_command_line("commit -m \"Initial commit\"", parser);
    assert(result.status == parser::ParseResult::ParserStatus_Success);
    assert(result.command_path == "git commit");
    assert(result.find_kw_arg_string("message") == "Initial commit");
    
    // Test commit alias
    result = parse_command_line("ci -m \"Using alias\"", parser);
    assert(result.status == parser::ParseResult::ParserStatus_Success);
    assert(result.command_path == "git commit");
    assert(result.find_kw_arg_string("message") == "Using alias");
    
    std::cout << "Subcommands test passed!\n";
}

// Test quoted arguments with parentheses (existing test)
void testQuotedParenthesesArgument()
{
    std::cout << "\nTesting quoted argument with parentheses...\n";
    parser::ArgumentParser parser("ds", "Test CLI parser");

    // root command: ds
    parser::Command &root = parser.get_root_command();
    // add positional argument: view
    root.add_positional_arg("cmd", "subcommand", parser::ArgType_Single, true);
    // add positional argument: data
    root.add_positional_arg("data", "data argument", parser::ArgType_Single, true);

    std::string input = "view \"MY.DATA(MEMBER)\"";
    parser::ParseResult result = parse_command_line(input, parser);

    assert(result.status == parser::ParseResult::ParserStatus_Success);
    assert(result.find_pos_arg_string("cmd") == "view");
    assert(result.find_pos_arg_string("data") == "MY.DATA(MEMBER)");
    
    std::cout << "Quoted parentheses argument test passed!\n";
}

// Test error cases
void testErrorCases()
{
    std::cout << "\nTesting error cases...\n";
    parser::ArgumentParser parser("test", "Test CLI parser");
    
    parser::Command &root = parser.get_root_command();
    root.add_keyword_arg("required", parser::make_aliases("-r", "--required"), "required flag", parser::ArgType_Single, true);
    root.add_positional_arg("input", "input file", parser::ArgType_Single, true);
    
    // Test missing required flag
    parser::ParseResult result = parse_command_line("input.txt", parser);
    assert(result.status == parser::ParseResult::ParserStatus_ParseError);
    assert(result.error_message.find("required") != std::string::npos);
    
    // Test missing required positional
    result = parse_command_line("-r value", parser);
    assert(result.status == parser::ParseResult::ParserStatus_ParseError);
    assert(result.error_message.find("input") != std::string::npos);
    
    // Test unknown flag
    result = parse_command_line("--unknown input.txt", parser);
    assert(result.status == parser::ParseResult::ParserStatus_ParseError);
    assert(result.error_message.find("unknown") != std::string::npos);
    
    std::cout << "Error cases test passed!\n";
}

// Test default values
void testDefaultValues()
{
    std::cout << "\nTesting default values...\n";
    parser::ArgumentParser parser("test", "Test CLI parser");
    
    parser::Command &root = parser.get_root_command();
    root.add_keyword_arg("count", parser::make_aliases("-c", "--count"), "count value", parser::ArgType_Single, false, parser::ArgValue((long long)10));
    root.add_keyword_arg("enabled", parser::make_aliases("-e", "--enabled"), "enable feature", parser::ArgType_Flag, false, parser::ArgValue(true));
    
    // Test without providing arguments (should use defaults)
    parser::ParseResult result = parse_command_line("", parser);
    assert(result.status == parser::ParseResult::ParserStatus_Success);
    assert(result.find_kw_arg_int("count") == 10);
    assert(result.find_kw_arg_bool("enabled") == true);
    
    // Test overriding defaults
    result = parse_command_line("-c 25 --no-enabled", parser);
    assert(result.status == parser::ParseResult::ParserStatus_Success);
    assert(result.find_kw_arg_int("count") == 25);
    // The automatic negation flag creates a separate no_enabled flag
    // When --no-enabled is used, we check that flag instead
    assert(result.find_kw_arg_bool("no_enabled") == true);
    
    std::cout << "Default values test passed!\n";
}

// Test help functionality
void testHelpFunctionality()
{
    std::cout << "\nTesting help functionality...\n";
    parser::ArgumentParser parser("test", "Test CLI parser");
    
    parser::Command &root = parser.get_root_command();
    root.add_keyword_arg("verbose", parser::make_aliases("-v", "--verbose"), "enable verbose output", parser::ArgType_Flag);
    root.add_positional_arg("input", "input file", parser::ArgType_Single, true);
    
    // Test help flag
    parser::ParseResult result = parse_command_line("--help", parser);
    assert(result.status == parser::ParseResult::ParserStatus_HelpRequested);
    assert(result.exit_code == 0);
    
    // Test short help flag
    result = parse_command_line("-h", parser);
    assert(result.status == parser::ParseResult::ParserStatus_HelpRequested);
    assert(result.exit_code == 0);
    
    std::cout << "Help functionality test passed!\n";
}

// Test flag combinations that should fail
void testInvalidFlagCombinations()
{
    std::cout << "\nTesting invalid flag combinations...\n";
    parser::ArgumentParser parser("test", "Test CLI parser");
    
    parser::Command &root = parser.get_root_command();
    root.add_keyword_arg("output", parser::make_aliases("-o", "--output"), "output file", parser::ArgType_Single);
    root.add_keyword_arg("verbose", parser::make_aliases("-v", "--verbose"), "verbose mode", parser::ArgType_Flag);
    
    // Test combining non-flag with flags (should fail)
    parser::ParseResult result = parse_command_line("-ov", parser);
    assert(result.status == parser::ParseResult::ParserStatus_ParseError);
    assert(result.error_message.find("requires a value") != std::string::npos);
    
    std::cout << "Invalid flag combinations test passed!\n";
}

// Test mixed arguments
void testMixedArguments()
{
    std::cout << "\nTesting mixed arguments...\n";
    parser::ArgumentParser parser("compiler", "C++ Compiler");
    
    parser::Command &root = parser.get_root_command();
    root.add_keyword_arg("optimize", parser::make_aliases("-O", "--optimize"), "optimization level", parser::ArgType_Single);
    root.add_keyword_arg("debug", parser::make_aliases("-g", "--debug"), "debug info", parser::ArgType_Flag);
    root.add_keyword_arg("include", parser::make_aliases("-I", "--include"), "include paths", parser::ArgType_Multiple);
    root.add_positional_arg("source", "source file", parser::ArgType_Single, true);
    root.add_positional_arg("output", "output file", parser::ArgType_Single, false, parser::ArgValue(std::string("a.out")));
    
    // Put positional arguments first to avoid them being consumed by multiple-value flags
    std::string input = "main.cpp program -O 2 -g -I /usr/include -I /opt/include";
    
    parser::ParseResult result = parse_command_line(input, parser);
    
    assert(result.status == parser::ParseResult::ParserStatus_Success);
    assert(result.find_kw_arg_int("optimize") == 2);
    assert(result.find_kw_arg_bool("debug") == true);
    std::vector<std::string> includes = result.find_kw_arg_list("include");
    assert(includes.size() == 2);
    assert(includes[0] == "/usr/include");
    assert(includes[1] == "/opt/include");
    assert(result.find_pos_arg_string("source") == "main.cpp");
    assert(result.find_pos_arg_string("output") == "program");
    
    std::cout << "Mixed arguments test passed!\n";
}

int main()
{
    try
    {
        std::cout << "Running CLI parser tests...\n";
        
        testBasicFlags();
        testFlagsWithValues();
        testPositionalArguments();
        testMultipleValues();
        testSubcommands();
        testQuotedParenthesesArgument();
        testErrorCases();
        testDefaultValues();
        testHelpFunctionality();
        testInvalidFlagCombinations();
        testMixedArguments();
        
        std::cout << "\nAll tests passed!\n";
        return 0;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Test failed: " << e.what() << "\n";
        return 1;
    }
}
