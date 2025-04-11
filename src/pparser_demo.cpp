#include "pparser.hpp"
#include <iostream>
#include <string>
#include <vector>

// Handler for the 'add' subcommand
int handleAddCommand(const pparser::ParseResult &result) {
  std::cout << "Executing command: " << result.commandPath << std::endl;

  const std::vector<std::string> *files = result.getPositionalArgStringVector(
      0); // 'files' is the first positional arg
  const bool *force =
      result.getKeywordArgBool("force"); // Optional '--force' flag

  if (files) {
    std::cout << "  Files to add: ";
    for (size_t i = 0; i < files->size(); ++i) {
      std::cout << (*files)[i] << (i == files->size() - 1 ? "" : ", ");
    }
    std::cout << std::endl;
  }

  std::cout << "  Force flag is " + std::string(*force ? "set." : "not set.")
            << std::endl;

  return 0;
}

// Handler for the 'commit' subcommand
int handleCommitCommand(const pparser::ParseResult &result) {
  std::cout << "Executing command: " << result.commandPath << std::endl;

  const std::string *message =
      result.getKeywordArgString("message"); // Required '-m' option
  const bool *amend =
      result.getKeywordArgBool("amend"); // Optional '--amend' flag
  const bool *verbose =
      result.getKeywordArgBool("verbose"); // Optional '--verbose' flag
  if (!message) {
    std::cerr << "Error: Required '-m' option not provided." << std::endl;
    return 1;
  }

  std::cout << "  Commit message: " << *message << std::endl;
  std::cout << "  Amend flag is " + std::string(*amend ? "set." : "not set.")
            << std::endl;
  std::cout << "  Verbose flag is " +
                   std::string(*verbose ? "set." : "not set.")
            << std::endl;

  return 0;
}

// --- Main Demo ---

int main(int argc, char *argv[]) {
  pparser::ArgumentParser parser("git_demo",
                                 "A simple git-like command line demo.");

  parser.getRootCommand().addKeywordArg("verbose", "-v", "--verbose",
                                        "Enable verbose output",
                                        pparser::ARGTYPE_FLAG);

  std::shared_ptr<pparser::Command> addCmd = std::make_shared<pparser::Command>(
      "add", "Add file contents to the index");
  addCmd
      ->addPositionalArg("files", "Files to add", pparser::ARGTYPE_MULTIPLE,
                         true) // Required, multiple values
      .addKeywordArg("force", "-f", "--force", "Allow adding ignored files",
                     pparser::ARGTYPE_FLAG, false)
      .setHandler(handleAddCommand);

  std::shared_ptr<pparser::Command> commitCmd =
      std::make_shared<pparser::Command>("commit",
                                         "Record changes to the repository");
  commitCmd
      ->addKeywordArg("message", "-m", "--message", "Commit message",
                      pparser::ARGTYPE_SINGLE, true) // Required, single value
      .addKeywordArg("amend", "-a", "--amend", "Amend the previous commit",
                     pparser::ARGTYPE_FLAG, false)
      .addKeywordArg("verbose", "-v", "--verbose", "Enable verbose output",
                     pparser::ARGTYPE_FLAG, false)
      .setHandler(handleCommitCommand);

  parser.getRootCommand().addSubcommand(addCmd);
  parser.getRootCommand().addSubcommand(commitCmd);

  pparser::ParseResult result;

  if (argc < 2) {
    std::cout << "----------------------------------------" << std::endl;
    std::cout << "pparser demo (git-like CLI)" << std::endl;
    std::cout << "Enter commands, e.g.:" << std::endl;
    std::cout << "\tadd file1.txt -f" << std::endl;
    std::cout << "\tcommit -m \"Initial\"" << std::endl;
    std::cout << "\thelp" << std::endl;
    std::cout << "\t--help" << std::endl;
    std::cout << "\tadd --help" << std::endl;
    std::cout << "Type 'exit' or 'quit' to quit." << std::endl;
    std::cout << "----------------------------------------" << std::endl;

    std::string line;
    while (true) {
      std::cout << "> ";
      std::getline(std::cin, line);

      if (line.empty())
        continue;
      if (line == "exit" || line == "quit")
        break;

      std::cout << "--- [input]: " << line << " ---" << std::endl;
      std::cout << "--- [output] ---" << std::endl;
      result = parser.parse(line);
      std::cout << "----------------------------------------" << std::endl;
    }
  } else {
    std::cout << "--- [output] ---" << std::endl;
    result = parser.parse(argc, argv);
    std::cout << "----------------------------------------" << std::endl;
  }

  return (result.status == pparser::ParseResult::PPARSER_STATUS_PARSE_ERROR)
             ? 1
             : 0;
}