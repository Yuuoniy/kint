#include "Diagnostic.h"
#include "SMTSolver.h"
#include <llvm/IR/DebugInfo.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Function.h>
#include <llvm/ADT/SmallString.h>
#include <llvm/ADT/SmallString.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/IR/Metadata.h>

using namespace llvm;

Diagnostic::Diagnostic() : OS(errs()) {}

static void getPath(SmallVectorImpl<char> &Path, const DIScope *SC)
{
	StringRef Filename = SC->getFilename();
	if (sys::path::is_absolute(Filename))
		Path.append(Filename.begin(), Filename.end());
	else
		sys::path::append(Path, SC->getDirectory(), Filename);
}

void Diagnostic::backtrace(Instruction *I)
{
	const char *Prefix = " - ";
	// MDNode *MD = I->getDebugLoc().getAsMDNode(I->getContext());
	// if (!MD)
	// 	return;
	// const DebugLoc &location = I->getDebugLoc();
	DILocation *Loc = (I->getDebugLoc().get());
	// if (!Loc)
	// 	return;
	// DILocation Loc(MD);
	OS << "stack: \n";
	for (;;)
	{
		SmallString<64> Path;
		getPath(Path, (Loc->getScope()));
		OS << Prefix << Path
		   << ':' << Loc->getLine()
		   << ':' << Loc->getColumn() << '\n';
		Loc = Loc->getInlinedAt();
		if (!Loc)
			break;
	}
}

void Diagnostic::bug(const Twine &Str)
{
	OS << "---\n"
	   << "bug: " << Str << "\n";
}

void Diagnostic::classify(Value *V)
{
	Instruction *I = dyn_cast<Instruction>(V);
	if (!I)
		return;

	if (MDNode *MD = I->getMetadata("taint"))
	{
		StringRef s = dyn_cast<MDString>(MD->getOperand(0))->getString();
		OS << "taint: " << s << "\n";
	}
	if (MDNode *MD = I->getMetadata("sink"))
	{
		StringRef s = dyn_cast<MDString>(MD->getOperand(0))->getString();
		OS << "sink: " << s << "\n";
	}
}

void Diagnostic::status(int Status)
{
	const char *Str;
	switch (Status)
	{
	case SMT_UNDEF:
		Str = "undef";
		break;
	case SMT_UNSAT:
		Str = "unsat";
		break;
	case SMT_SAT:
		Str = "sat";
		break;
	default:
		Str = "timeout";
		break;
	}
	OS << "status: " << Str << "\n";
}
