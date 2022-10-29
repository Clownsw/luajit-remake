#pragma once

#include "misc_llvm_helper.h"

namespace dast {

struct InterpreterFunctionInterface
{
public:
    static llvm::FunctionType* WARN_UNUSED GetType(llvm::LLVMContext& ctx);
    static llvm::Function* WARN_UNUSED CreateFunction(llvm::Module* module, const std::string& name);
    static void CreateDispatchToBytecode(llvm::Value* target, llvm::Value* coroutineCtx, llvm::Value* stackbase, llvm::Value* bytecodePtr, llvm::Value* codeBlock, llvm::Instruction* insertBefore);
    static void CreateDispatchToReturnContinuation(llvm::Value* target, llvm::Value* coroutineCtx, llvm::Value* stackbase, llvm::Value* retStart, llvm::Value* numRets, llvm::Instruction* insertBefore);
    static void CreateDispatchToCallee(llvm::Value* codePointer, llvm::Value* coroutineCtx, llvm::Value* preFixupStackBase, llvm::Value* calleeCodeBlockHeapPtr, llvm::Value* numArgs, llvm::Value* isMustTail64, llvm::Instruction* insertBefore);

    // Return the list of avaiable GPR/FPR that can be used to pass additional arguments to the interpreter bytecode function
    // The result is represented as a vector of argument ordinals.
    //
    static std::vector<uint64_t> WARN_UNUSED GetAvaiableGPRListForBytecode();
    static std::vector<uint64_t> WARN_UNUSED GetAvaiableFPRListForBytecode();
};

}   // namespace dast
