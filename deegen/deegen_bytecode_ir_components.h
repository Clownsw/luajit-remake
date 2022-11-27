#pragma once

#include "misc_llvm_helper.h"
#include "deegen_bytecode_operand.h"

namespace dast {

enum class BytecodeIrComponentKind
{
    // This is the main function of the bytecode variant
    //
    Main,
    // This is a return continuation
    // The return continuation additionally has access to all the return values of the call
    //
    ReturnContinuation,
    // This is a quickened bytecode slow path
    //
    QuickeningSlowPath,
    // This is a slow path created by EnterSlowPath API
    //
    SlowPath,
    // This is an affliated bytecode created by FuseICIntoInterpreterOpcode() API
    //
    FusedInInlineCacheEffect
};

// Describes a subcomponent of a bytecode (the main function, a slow path, a quickening variant, etc)
//
struct BytecodeIrComponent
{
    // The type of this component
    //
    BytecodeIrComponentKind m_processKind;

    // The owning bytecode variant
    //
    BytecodeVariantDefinition* m_bytecodeDef;

    // The LLVM module, and the function in th module for this component
    //
    std::unique_ptr<llvm::Module> m_module;
    llvm::Function* m_impl;

    // The desired name of the resulted function after processing
    //
    std::string m_resultFuncName;

    // Construct a bytecode IR component
    // This function clones the module, so the original module is untouched.
    // The cloned module is owned by this class.
    //
    BytecodeIrComponent(BytecodeVariantDefinition* bytecodeDef, llvm::Function* impl, BytecodeIrComponentKind processKind);

    struct ProcessFusedInIcEffectTag { };

    // Construct a 'FusedIC' bytecode IR component
    // Need a special function because it takes an extra 'icEffectOrd' as argument
    //
    BytecodeIrComponent(ProcessFusedInIcEffectTag, BytecodeVariantDefinition* bytecodeDef, llvm::Function* impl, size_t icEffectOrd);

    // Run the deegen-level optimization passes
    //
    void DoOptimization();
};

// The class that holds all the LLVM IR information about one bytecode
//
struct BytecodeIrInfo
{
    // The bytecode variant of this bytecode
    //
    BytecodeVariantDefinition* m_bytecodeDef;

    // The main implementation function
    //
    std::unique_ptr<BytecodeIrComponent> m_mainComponent;

    // All the return continuation functions
    //
    std::vector<std::unique_ptr<BytecodeIrComponent>> m_allRetConts;

    // If the bytecode is a quickening one, this holds the slow path
    // Nullptr otherwise
    //
    std::unique_ptr<BytecodeIrComponent> m_quickeningSlowPath;

    // Holds all the slow paths generated by EnterSlowPath API
    // Note that the m_resultFuncName of these submodules may not be unique across translational units.
    // We will rename them after the Main processor links all the submodules together.
    //
    std::vector<std::unique_ptr<BytecodeIrComponent>> m_slowPaths;

    // This holds all the fused IC effect specializations, if any
    // Only useful for interpreter
    //
    std::vector<std::unique_ptr<BytecodeIrComponent>> m_fusedICs;

    // This stores all the additional affliated bytecodes functions, which should be
    // assigned opcode ordinal immediately following the main bytecode function in sequential order
    // Only useful for interpreter
    //
    std::vector<std::string> m_affliatedBytecodeFnNames;

    // This stores the function name of all the IC body
    // These functions are always defined in the main component
    //
    std::vector<std::string> m_icBodyNames;

    static BytecodeIrInfo WARN_UNUSED Create(BytecodeVariantDefinition* bytecodeDef, llvm::Function* mainImpl);

    // Some misc naming-related utilities
    //
    static std::string WARN_UNUSED ToInterpreterName(const std::string& genericName)
    {
        ReleaseAssert(genericName.starts_with("__deegen_bytecode_"));
        return "__deegen_interpreter_op_" + genericName.substr(strlen("__deegen_bytecode_"));
    }

    static std::string WARN_UNUSED GetBaseName(BytecodeVariantDefinition* bytecodeDef)
    {
        return std::string("__deegen_bytecode_") + bytecodeDef->m_bytecodeName + "_" + std::to_string(bytecodeDef->m_variantOrd);
    }

    static std::string WARN_UNUSED GetRetContFuncName(BytecodeVariantDefinition* bytecodeDef, size_t rcOrd)
    {
        return GetBaseName(bytecodeDef) + "_retcont_" + std::to_string(rcOrd);
    }

    static std::string GetQuickeningSlowPathFuncName(BytecodeVariantDefinition* bytecodeDef)
    {
        return GetBaseName(bytecodeDef) + "_quickening_slowpath";
    }
};

// A utility helper to find all the return continuations and slow paths in a bytecode definition
//
struct ReturnContinuationAndSlowPathFinder
{
    ReturnContinuationAndSlowPathFinder(llvm::Function* from, bool ignoreSlowPaths);

    bool m_ignoreSlowPaths;

    std::unordered_set<llvm::Function*> m_returnContinuations;
    std::unordered_set<llvm::Function*> m_slowPaths;

    // This is simply m_slowPaths + m_returnContinuations
    //
    std::unordered_set<llvm::Function*> m_allResults;

private:
    void FindCalls(llvm::Function* from);
    void FindSlowPaths(llvm::Function* from);
    void dfs(llvm::Function* cur, bool isCall);
};

constexpr const char* x_get_bytecode_metadata_ptr_placeholder_fn_name = "__DeegenImpl_GetInterpreterBytecodeMetadataPtrPlaceholder";

}   // namespace dast
