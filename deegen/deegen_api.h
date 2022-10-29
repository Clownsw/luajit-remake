#pragma once

#include "tvalue.h"
#include "api_define_lib_function.h"
#include "deegen_preserve_lambda_helper.h"

class CodeBlock;

enum MakeCallOption
{
    NoOption,
    DontProfileInInterpreter
};

// These names are hardcoded for our LLVM IR processor to locate
//
extern "C" void NO_RETURN DeegenImpl_ReturnValue(TValue value);
extern "C" void NO_RETURN DeegenImpl_ReturnNone();
extern "C" void NO_RETURN DeegenImpl_ReturnValueAndBranch(TValue value);
extern "C" void NO_RETURN DeegenImpl_ReturnNoneAndBranch();
extern "C" void NO_RETURN DeegenImpl_MakeCall_ReportContinuationAfterCall(void* handler, void* func);
extern "C" void* WARN_UNUSED DeegenImpl_MakeCall_ReportParam(void* handler, TValue arg);
extern "C" void* WARN_UNUSED DeegenImpl_MakeCall_ReportParamList(void* handler, const TValue* argBegin, size_t numArgs);
extern "C" void* WARN_UNUSED DeegenImpl_MakeCall_ReportTarget(void* handler, uint64_t target);
extern "C" void* WARN_UNUSED DeegenImpl_MakeCall_ReportOption(void* handler, size_t option);
extern "C" void* WARN_UNUSED DeegenImpl_StartMakeCallInfo();
extern "C" void* WARN_UNUSED DeegenImpl_StartMakeCallPassingVariadicResInfo();
extern "C" void* WARN_UNUSED DeegenImpl_StartMakeInPlaceCallInfo();
extern "C" void* WARN_UNUSED DeegenImpl_StartMakeInPlaceCallPassingVariadicResInfo();
extern "C" void* WARN_UNUSED DeegenImpl_StartMakeTailCallInfo();
extern "C" void* WARN_UNUSED DeegenImpl_StartMakeTailCallPassingVariadicResInfo();
extern "C" void* WARN_UNUSED DeegenImpl_StartMakeInPlaceTailCallInfo();
extern "C" void* WARN_UNUSED DeegenImpl_StartMakeInPlaceTailCallPassingVariadicResInfo();
extern "C" void NO_RETURN DeegenImpl_ThrowErrorTValue(TValue value);
extern "C" void NO_RETURN DeegenImpl_ThrowErrorCString(const char* value);
extern "C" HeapPtr<TableObject> WARN_UNUSED DeegenImpl_GetFEnvGlobalObject();
extern "C" void NO_RETURN DeegenImpl_GuestLanguageFunctionReturn_NoValue();
extern "C" void NO_RETURN DeegenImpl_GuestLanguageFunctionReturn(TValue* retStart, size_t numRets);
extern "C" void NO_RETURN DeegenImpl_GuestLanguageFunctionReturnAppendingVariadicResults(TValue* retStart, size_t numRets);
extern "C" HeapPtr<FunctionObject> WARN_UNUSED DeegenImpl_CreateNewClosure(CodeBlock* cb);
TValue WARN_UNUSED DeegenImpl_UpvalueAccessor_Get(size_t ord);
void DeegenImpl_UpvalueAccessor_Put(size_t ord, TValue valueToPut);
void DeegenImpl_UpvalueAccessor_Close(const TValue* limit);
extern "C" TValue* WARN_UNUSED DeegenImpl_GetVarArgsStart();
extern "C" size_t WARN_UNUSED DeegenImpl_GetNumVarArgs();
extern "C" void DeegenImpl_StoreVarArgsAsVariadicResults();
extern "C" TValue* WARN_UNUSED DeegenImpl_GetVariadicResultsStart();
extern "C" size_t WARN_UNUSED DeegenImpl_GetNumVariadicResults();
extern "C" void NO_RETURN DeegenImpl_EnterSlowPathLambda(const void* cp, const void* fpp);

// Return zero or one value as the result of the operation
//
inline void ALWAYS_INLINE NO_RETURN Return(TValue value)
{
    DeegenImpl_ReturnValue(value);
}

inline void ALWAYS_INLINE NO_RETURN Return()
{
    DeegenImpl_ReturnNone();
}

// Return the result of the current bytecode, and additionally informs that control flow should not fallthrough to
// the next bytecode, but redirected to the conditional branch target of this bytecode
//
inline void ALWAYS_INLINE NO_RETURN ReturnAndBranch(TValue value)
{
    DeegenImpl_ReturnValueAndBranch(value);
}

inline void ALWAYS_INLINE NO_RETURN ReturnAndBranch()
{
    DeegenImpl_ReturnNoneAndBranch();
}

inline void ALWAYS_INLINE NO_RETURN ThrowError(TValue value)
{
    DeegenImpl_ThrowErrorTValue(value);
}

inline void ALWAYS_INLINE NO_RETURN ThrowError(const char* value)
{
    DeegenImpl_ThrowErrorCString(value);
}

// Get the global object captured by the current function
//
inline HeapPtr<TableObject> WARN_UNUSED ALWAYS_INLINE GetFEnvGlobalObject()
{
    return DeegenImpl_GetFEnvGlobalObject();
}

inline void ALWAYS_INLINE NO_RETURN GuestLanguageFunctionReturn()
{
    DeegenImpl_GuestLanguageFunctionReturn_NoValue();
}

inline void ALWAYS_INLINE NO_RETURN GuestLanguageFunctionReturn(TValue* retStart, size_t numRets)
{
    DeegenImpl_GuestLanguageFunctionReturn(retStart, numRets);
}

inline void ALWAYS_INLINE NO_RETURN GuestLanguageFunctionReturnAppendingVariadicResults(TValue* retStart, size_t numRets)
{
    DeegenImpl_GuestLanguageFunctionReturnAppendingVariadicResults(retStart, numRets);
}

inline HeapPtr<FunctionObject> WARN_UNUSED ALWAYS_INLINE CreateNewClosure(CodeBlock* cb)
{
    return DeegenImpl_CreateNewClosure(cb);
}

struct UpvalueAccessor
{
    static TValue WARN_UNUSED ALWAYS_INLINE Get(size_t ord)
    {
        return DeegenImpl_UpvalueAccessor_Get(ord);
    }

    static void ALWAYS_INLINE Put(size_t ord, TValue valueToPut)
    {
        DeegenImpl_UpvalueAccessor_Put(ord, valueToPut);
    }

    // Close all upvalues >= limit
    //
    static void ALWAYS_INLINE Close(const TValue* limit)
    {
        DeegenImpl_UpvalueAccessor_Close(limit);
    }
};

struct VarArgsAccessor
{
    static TValue* WARN_UNUSED ALWAYS_INLINE GetPtr()
    {
        return DeegenImpl_GetVarArgsStart();
    }

    static size_t WARN_UNUSED ALWAYS_INLINE GetNum()
    {
        return DeegenImpl_GetNumVarArgs();
    }

    static void ALWAYS_INLINE StoreAllVarArgsAsVariadicResults()
    {
        return DeegenImpl_StoreVarArgsAsVariadicResults();
    }
};

struct VariadicResultsAccessor
{
    static TValue* WARN_UNUSED ALWAYS_INLINE GetPtr()
    {
        return DeegenImpl_GetVariadicResultsStart();
    }

    static size_t WARN_UNUSED ALWAYS_INLINE GetNum()
    {
        return DeegenImpl_GetNumVariadicResults();
    }
};

namespace detail {

template<typename... Args>
struct MakeCallArgHandlerImpl;

template<>
struct MakeCallArgHandlerImpl<std::nullptr_t>
{
    static void NO_RETURN ALWAYS_INLINE handle(void* handler, std::nullptr_t /*nullptr*/)
    {
        DeegenImpl_MakeCall_ReportContinuationAfterCall(handler, nullptr);
    }
};

template<typename... ContinuationFnArgs>
struct MakeCallArgHandlerImpl<void(*)(ContinuationFnArgs...)>
{
    static void NO_RETURN ALWAYS_INLINE handle(void* handler, void(*func)(ContinuationFnArgs...))
    {
        DeegenImpl_MakeCall_ReportContinuationAfterCall(handler, reinterpret_cast<void*>(func));
    }
};

template<typename... ContinuationFnArgs>
struct MakeCallArgHandlerImpl<NO_RETURN void(*)(ContinuationFnArgs...)>
{
    static void NO_RETURN ALWAYS_INLINE handle(void* handler, void(*func)(ContinuationFnArgs...))
    {
        DeegenImpl_MakeCall_ReportContinuationAfterCall(handler, reinterpret_cast<void*>(func));
    }
};

template<typename... Args>
struct MakeCallArgHandlerImpl<TValue, Args...>
{
    static void NO_RETURN ALWAYS_INLINE handle(void* handler, TValue arg, Args... remainingArgs)
    {
        MakeCallArgHandlerImpl<Args...>::handle(DeegenImpl_MakeCall_ReportParam(handler, arg), remainingArgs...);
    }
};

template<typename T, typename... Args>
struct MakeCallArgHandlerImpl<TValue*, T, Args...>
{
    static_assert(std::is_integral_v<T> && !std::is_signed_v<T> && !std::is_same_v<T, bool>);

    static void NO_RETURN ALWAYS_INLINE handle(void* handler, TValue* argBegin, T numArgs, Args... remainingArgs)
    {
        MakeCallArgHandlerImpl<Args...>::handle(DeegenImpl_MakeCall_ReportParamList(handler, argBegin, static_cast<size_t>(numArgs)), remainingArgs...);
    }
};

template<typename T, typename... Args>
struct MakeCallArgHandlerImpl<const TValue*, T, Args...>
{
    static_assert(std::is_integral_v<T> && !std::is_signed_v<T> && !std::is_same_v<T, bool>);

    static void NO_RETURN ALWAYS_INLINE handle(void* handler, const TValue* argBegin, T numArgs, Args... remainingArgs)
    {
        MakeCallArgHandlerImpl<Args...>::handle(DeegenImpl_MakeCall_ReportParamList(handler, argBegin, static_cast<size_t>(numArgs)), remainingArgs...);
    }
};

template<typename... Args>
struct MakeCallHandlerImpl;

template<typename... Args>
struct MakeCallHandlerImpl<HeapPtr<FunctionObject>, Args...>
{
    static void NO_RETURN ALWAYS_INLINE handle(void* handler, HeapPtr<FunctionObject> target, Args... args)
    {
        MakeCallArgHandlerImpl<Args...>::handle(DeegenImpl_MakeCall_ReportTarget(handler, reinterpret_cast<uint64_t>(target)), args...);
    }
};

template<typename... Args>
struct MakeCallHandlerImpl<MakeCallOption, HeapPtr<FunctionObject>, Args...>
{
    static void NO_RETURN ALWAYS_INLINE handle(void* handler, MakeCallOption option, HeapPtr<FunctionObject> target, Args... args)
    {
        MakeCallHandlerImpl<HeapPtr<FunctionObject>, Args...>::handle(DeegenImpl_MakeCall_ReportOption(handler, static_cast<size_t>(option)), target, args...);
    }
};

constexpr size_t x_stackFrameHeaderSlots = 4;

template<typename... ContinuationFnArgs>
void NO_RETURN ALWAYS_INLINE ReportInfoForInPlaceCall(void* handler, TValue* argsBegin, size_t numArgs, void(*continuationFn)(ContinuationFnArgs...))
{
    handler = DeegenImpl_MakeCall_ReportTarget(handler, (argsBegin - x_stackFrameHeaderSlots)->m_value);
    handler = DeegenImpl_MakeCall_ReportParamList(handler, argsBegin, numArgs);
    DeegenImpl_MakeCall_ReportContinuationAfterCall(handler, reinterpret_cast<void*>(continuationFn));
}

inline void NO_RETURN ALWAYS_INLINE ReportInfoForInPlaceTailCall(void* handler, TValue* argsBegin, size_t numArgs)
{
    handler = DeegenImpl_MakeCall_ReportTarget(handler, (argsBegin - x_stackFrameHeaderSlots)->m_value);
    handler = DeegenImpl_MakeCall_ReportParamList(handler, argsBegin, numArgs);
    DeegenImpl_MakeCall_ReportContinuationAfterCall(handler, nullptr);
}

}   // namespace detail

// Make a call to a guest language function, with pre-layouted frame holding the target function and the arguments.
// Specifically, a StackFrameHeader must sit right before argsBegin (i.e., in memory region argsBegin[-stackFrameHdrSize, 0) )
// with the target HeapPtr<FunctionObject> already filled, and argsBegin[0, numArgs) must hold all the arguments.
// Note that this also implies that everything >= argsBegin - stackFrameHdrSize are invalidated after the call
//
template<typename... ContinuationFnArgs>
void NO_RETURN ALWAYS_INLINE MakeInPlaceCall(TValue* argsBegin, size_t numArgs, void(*continuationFn)(ContinuationFnArgs...))
{
    detail::ReportInfoForInPlaceCall(DeegenImpl_StartMakeInPlaceCallInfo(), argsBegin, numArgs, continuationFn);
}

// Same as above, except that the variadic results from the immediate preceding opcode are appended to the end of the argument list
//
template<typename... ContinuationFnArgs>
void NO_RETURN ALWAYS_INLINE MakeInPlaceCallPassingVariadicRes(TValue* argsBegin, size_t numArgs, void(*continuationFn)(ContinuationFnArgs...))
{
    detail::ReportInfoForInPlaceCall(DeegenImpl_StartMakeInPlaceCallPassingVariadicResInfo(), argsBegin, numArgs, continuationFn);
}

// The tail call versions
//
inline void NO_RETURN ALWAYS_INLINE MakeInPlaceTailCall(TValue* argsBegin, size_t numArgs)
{
    detail::ReportInfoForInPlaceTailCall(DeegenImpl_StartMakeInPlaceTailCallInfo(), argsBegin, numArgs);
}

inline void NO_RETURN ALWAYS_INLINE MakeInPlaceTailCallPassingVariadicRes(TValue* argsBegin, size_t numArgs)
{
    detail::ReportInfoForInPlaceTailCall(DeegenImpl_StartMakeInPlaceTailCallPassingVariadicResInfo(), argsBegin, numArgs);
}

// Make a call to a guest language function.
// The new call frame is set up at the end of the current function's stack frame.
// This requires copying, so it's less efficient, but also more flexible.
//
// The parameters are (in listed order):
// 1. An optional Option flag
// 2. The target function (a HeapPtr<FunctionObject> value)
// 3. The arguments, described as a list of TValue and (TValue*, size_t)
// 4. The continuation function
//
template<typename... Args>
void NO_RETURN ALWAYS_INLINE MakeCall(Args... args)
{
    detail::MakeCallHandlerImpl<Args...>::handle(DeegenImpl_StartMakeCallInfo(), args...);
}

// Same as above, except additionally passing varret
//
template<typename... Args>
void NO_RETURN ALWAYS_INLINE MakeCallPassingVariadicRes(Args... args)
{
    detail::MakeCallHandlerImpl<Args...>::handle(DeegenImpl_StartMakeCallPassingVariadicResInfo(), args...);
}

// The tail call versions
//
template<typename... Args>
void NO_RETURN ALWAYS_INLINE MakeTailCall(Args... args)
{
    detail::MakeCallHandlerImpl<Args..., std::nullptr_t>::handle(DeegenImpl_StartMakeTailCallInfo(), args..., nullptr);
}

template<typename... Args>
void NO_RETURN ALWAYS_INLINE MakeTailCallPassingVariadicRes(Args... args)
{
    detail::MakeCallHandlerImpl<Args..., std::nullptr_t>::handle(DeegenImpl_StartMakeTailCallPassingVariadicResInfo(), args..., nullptr);
}

// These names are hardcoded for our LLVM IR processor to locate
//
TValue DeegenImpl_GetReturnValueAtOrd(size_t ord);
size_t DeegenImpl_GetNumReturnValues();
void DeegenImpl_StoreReturnValuesTo(TValue* dst, size_t numToStore);
void DeegenImpl_StoreReturnValuesAsVariadicResults();

// APIs for accessing return values
//
inline TValue WARN_UNUSED ALWAYS_INLINE GetReturnValue(size_t ord)
{
    return DeegenImpl_GetReturnValueAtOrd(ord);
}

inline size_t WARN_UNUSED ALWAYS_INLINE GetNumReturnValues()
{
    return DeegenImpl_GetNumReturnValues();
}

// Store the first 'numToStore' return values to the destination address, padding nil as needed
//
inline void ALWAYS_INLINE StoreReturnValuesTo(TValue* dst, size_t numToStore)
{
    DeegenImpl_StoreReturnValuesTo(dst, numToStore);
}

inline void ALWAYS_INLINE StoreReturnValuesAsVariadicResults()
{
    DeegenImpl_StoreReturnValuesAsVariadicResults();
}

// Creates a slow path. The logic in the slow path will be separated out into a dedicated slow path function.
// This helps improve code locality, and can also slightly improve the fast path code by reducing unnecessary
// register shuffling, spilling and stack pointer adjustments.
//
// Note that the slow path will be executed as a tail call, so the lifetime of all the local variables in the
// interpreter function has ended when the slow path lambda is executed. That is, it is illegal for the lambda
// to capture or use any variable defined in the interpreter function by reference.
//
template<typename Lambda>
void ALWAYS_INLINE NO_RETURN EnterSlowPath(const Lambda& lambda)
{
    static_assert(std::is_same_v<void, std::invoke_result_t<Lambda>>, "The lambda must take no arguments and return void");
    DeegenImpl_EnterSlowPathLambda(DeegenGetLambdaClosureAddr(lambda), DeegenGetLambdaFunctorPP(lambda));
}
