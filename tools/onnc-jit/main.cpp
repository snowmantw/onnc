//===- main.cpp -----------------------------------------------------------===//
//
//                             The ONNC Project
//
// See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "ONNCJITApp.h"
#include <onnc/ADT/Color.h>
#include <onnc/Support/Host.h>
#include <onnc/Support/IOStream.h>
#include <onnc/Option/CommandLine.h>
#include <onnc/Config/AboutData.h>

using namespace onnc;

static AboutData g_About("onnc-jit",
                         "onnc-jit",
                         AboutLicense::kPrivate,
                         "ONNC-JIT is the just-in-time interpreter of ONNC");

static cl::opt<Path> OptInput("input", cl::kPositional, cl::kOptional,
    cl::kValueRequired,
    cl::desc("The input file"), cl::about(g_About));

static cl::opt<std::string> OptOutput("o", cl::kShort, cl::kOptional,
    cl::kValueRequired,
    cl::desc("The output file"),
    cl::about(g_About));

static cl::opt<bool> OptHelp("help", cl::kLong, cl::kOptional,
    cl::kValueDisallowed, cl::init(false),
    cl::desc("Show this manual."),
    cl::about(g_About));

static cl::opt<bool> OptVersion("version", cl::kLong, cl::kOptional,
    cl::kValueDisallowed, cl::init(false),
    cl::desc("Show the version string."),
    cl::about(g_About));

static cl::alias HelpAliasH("h", cl::kShort, cl::trueopt(OptHelp));
static cl::alias HelpAliasQ("?", cl::kShort, cl::trueopt(OptHelp));

static cl::opt<unsigned int>
OptVerbose("verbose",
    cl::kLong,
    cl::kZeroOrMore,
    cl::kValueRequired,
    cl::kEqualSeparated,
    cl::desc("Set verbose level to <number> (default is 1)."),
    cl::init(1),
    cl::about(g_About));

static cl::opt<bool>
OptV("v", cl::kShort, cl::kZeroOrMore, cl::kValueDisallowed, cl::init(false),
    cl::desc("One -v increases one verbose level."),
    cl::about(g_About));

static cl::opt<bool>
OptQuiet("quiet", cl::kLong, cl::kOptional, cl::kValueDisallowed,
    cl::init(false),
    cl::desc("Set verbose level to 0."),
    cl::about(g_About));

static cl::opt<std::string> OptQuadruple("mquadruple", cl::kShort, cl::kOptional,
    cl::kValueRequired, cl::desc("target quadruple"), cl::about(g_About));
    
static cl::opt<std::string> OptMArch("march", cl::kShort, cl::kOptional,
    cl::kValueRequired, cl::desc("target architecture"), cl::about(g_About));

//===----------------------------------------------------------------------===//
// Main Procedure
//===----------------------------------------------------------------------===//
int main(int pArgc, char* pArgv[])
{
  ONNCJITApp jit(pArgc, pArgv);

  // -verbose=level
  if (OptVerbose.hasOccurrence())
    jit.options().setVerbose(OptVerbose);

  // -v
  if (OptV.hasOccurrence())
    jit.options().setVerbose(OptV.getNumOccurrence());

  // --quiet
  if (OptQuiet)
    jit.options().setVerbose(0);

  // --help
  if (OptHelp) {
    g_About.print(outs(), ONNCJITConfig::kNormal < jit.options().verbose());
    return EXIT_SUCCESS;
  }

  // --version
  if (OptVersion) {
    outs() << g_About.version() << "\n";
    return EXIT_SUCCESS;
  }

  // check inputs
  if (!exists(OptInput)) {
    errs() << Color::MAGENTA << "Fatal" << Color::RESET
           << ": input file not found: " << OptInput << std::endl;
    return EXIT_FAILURE;
  }
  if (!is_regular(OptInput)) {
    errs() << Color::MAGENTA << "Fatal" << Color::RESET
           << ": input file is not a regular file: " << OptInput << std::endl;
    return EXIT_FAILURE;
  }
  jit.options().setInput(OptInput);

  // check output
  if (OptOutput.hasOccurrence())
    jit.options().setOutput(OptOutput);
  else
    jit.options().setOutput(ONNCJITConfig::DefaultOutputName);

  // Set quadruple. We shall check target instance at compilation time.
  if (!OptQuadruple.hasOccurrence() && ! OptMArch.hasOccurrence()) {
    jit.options().setQuadruple(sys::GetHostQuadruple());
  }
  else {
    if (OptQuadruple.hasOccurrence())
      jit.options().setQuadruple(OptQuadruple);

    if (OptMArch.hasOccurrence())
      jit.options().setArchName(OptMArch);
  }

  return jit.run();
}
