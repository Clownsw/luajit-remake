#include "bytecode_definition_utils.h"
#include "deegen_api.h"

#include "bytecode.h"

static void NO_RETURN ArithmeticOperationMetamethodCallContinuation(TValue /*lhs*/, TValue /*rhs*/)
{
    Return(GetReturnValue(0));
}

template<LuaMetamethodKind opKind>
static void NO_RETURN ArithmeticOperationImpl(TValue lhs, TValue rhs)
{
    if (likely(lhs.Is<tDouble>() && rhs.Is<tDouble>()))
    {
        double ld = lhs.As<tDouble>();
        double rd = rhs.As<tDouble>();
        double res;
        if constexpr(opKind == LuaMetamethodKind::Add)
        {
            res = ld + rd;
        }
        else if constexpr(opKind == LuaMetamethodKind::Sub)
        {
            res = ld - rd;
        }
        else if constexpr(opKind == LuaMetamethodKind::Mul)
        {
            res = ld * rd;
        }
        else if constexpr(opKind == LuaMetamethodKind::Div)
        {
            res = ld / rd;
        }
        else if constexpr(opKind == LuaMetamethodKind::Mod)
        {
            res = ModulusWithLuaSemantics(ld, rd);
        }
        else
        {
            static_assert(opKind == LuaMetamethodKind::Pow, "unexpected opKind");
            res = pow(ld, rd);
        }
        Return(TValue::Create<tDouble>(res));
    }
    else
    {
        TValue metamethod;

        if (likely(lhs.Is<tTable>()))
        {
            HeapPtr<TableObject> tableObj = lhs.As<tTable>();
            TableObject::GetMetatableResult result = TableObject::GetMetatable(tableObj);
            if (result.m_result.m_value != 0)
            {
                HeapPtr<TableObject> metatable = result.m_result.As<TableObject>();
                GetByIdICInfo icInfo;
                TableObject::PrepareGetById(metatable, VM_GetStringNameForMetatableKind(opKind), icInfo /*out*/);
                metamethod = TableObject::GetById(metatable, VM_GetStringNameForMetatableKind(opKind).As<void>(), icInfo);
                if (likely(!metamethod.IsNil()))
                {
                    goto do_metamethod_call;
                }
            }
        }

        // Handle case that lhs/rhs are number or string that can be converted to number
        //
        {
            std::optional<double> res = TryDoBinaryOperationConsideringStringConversion(lhs, rhs, opKind);
            if (res)
            {
                Return(TValue::CreateDouble(res.value()));
            }
        }

        // Now we know we will need to call metamethod, determine the metamethod to call
        //
        // TODO: this could have been better since we already know lhs is not a table with metatable
        // TODO: I don't think this should be a templated function..
        //
        metamethod = GetMetamethodForBinaryArithmeticOperation(lhs, rhs, opKind);
        if (metamethod.IsNil())
        {
            // TODO: make this error consistent with Lua
            //
            ThrowError("Invalid types for arithmetic operation");
        }

do_metamethod_call:
        {
            GetCallTargetConsideringMetatableResult callTarget = GetCallTargetConsideringMetatable(metamethod);
            if (callTarget.m_target.m_value == 0)
            {
                ThrowError(MakeErrorMessageForUnableToCall(metamethod));
            }

            if (unlikely(callTarget.m_invokedThroughMetatable))
            {
                MakeCall(callTarget.m_target.As(), metamethod, lhs, rhs, ArithmeticOperationMetamethodCallContinuation);
            }
            else
            {
                MakeCall(callTarget.m_target.As(), lhs, rhs, ArithmeticOperationMetamethodCallContinuation);
            }
        }
    }
}

DEEGEN_DEFINE_BYTECODE_TEMPLATE(ArithmeticOperation, LuaMetamethodKind opKind)
{
    Operands(
        BytecodeSlotOrConstant("lhs"),
        BytecodeSlotOrConstant("rhs")
    );
    Result(BytecodeValue);
    Implementation(ArithmeticOperationImpl<opKind>);
    Variant(
        Op("lhs").IsBytecodeSlot(),
        Op("rhs").IsBytecodeSlot()
    );
    Variant(
        Op("lhs").IsConstant<tDouble>(),
        Op("rhs").IsBytecodeSlot()
    );
    Variant(
        Op("lhs").IsBytecodeSlot(),
        Op("rhs").IsConstant<tDouble>()
    );
}

DEEGEN_DEFINE_BYTECODE_BY_TEMPLATE_INSTANTIATION(Add, ArithmeticOperation, LuaMetamethodKind::Add);
DEEGEN_DEFINE_BYTECODE_BY_TEMPLATE_INSTANTIATION(Sub, ArithmeticOperation, LuaMetamethodKind::Sub);
DEEGEN_DEFINE_BYTECODE_BY_TEMPLATE_INSTANTIATION(Mul, ArithmeticOperation, LuaMetamethodKind::Mul);
DEEGEN_DEFINE_BYTECODE_BY_TEMPLATE_INSTANTIATION(Div, ArithmeticOperation, LuaMetamethodKind::Div);
DEEGEN_DEFINE_BYTECODE_BY_TEMPLATE_INSTANTIATION(Mod, ArithmeticOperation, LuaMetamethodKind::Mod);
DEEGEN_DEFINE_BYTECODE_BY_TEMPLATE_INSTANTIATION(Pow, ArithmeticOperation, LuaMetamethodKind::Pow);

DEEGEN_END_BYTECODE_DEFINITIONS
