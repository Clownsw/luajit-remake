#pragma once

#include "json_utils.h"
#include "misc_llvm_helper.h"

namespace dast {

struct ProcessBytecodeDefinitionForInterpreterResult
{
    std::unique_ptr<llvm::Module> m_processedModule;
    std::vector<std::string> m_generatedClassNames;
    // Has same length as m_generatedClassNames, each subvector holding the names of all the variants, in the same order as the opcode used by the builder
    //
    std::vector<std::vector<std::string>> m_allExternCDeclarations;
    std::vector<std::pair<std::string /*auditFileName*/, std::string /*content*/>> m_auditFiles;
    std::string m_generatedHeaderFile;
    json m_bytecodeInfoJson;
};

ProcessBytecodeDefinitionForInterpreterResult WARN_UNUSED ProcessBytecodeDefinitionForInterpreter(std::unique_ptr<llvm::Module> module);

}   // namespace dast
