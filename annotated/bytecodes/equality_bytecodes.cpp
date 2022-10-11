#include "bytecode_definition_utils.h"
#include "deegen_api.h"

#include "runtime_utils.h"

namespace {

template<bool compareForNotEqual, bool shouldBranch>
void NO_RETURN EqualityOperationMetamethodCallContinuation(TValue /*lhs*/, TValue /*rhs*/)
{
    TValue retValue = GetReturnValue(0);
    bool isTruthy = retValue.IsTruthy();
    bool result = isTruthy ^ compareForNotEqual;
    if constexpr(shouldBranch)
    {
        if (result) { ReturnAndBranch(); } else { Return(); }
    }
    else
    {
        Return(TValue::Create<tBool>(result));
    }
}

// When 'compareForNotEqual' is true, it computes !(lhs == rhs). Otherwise it computes lhs == rhs
// When 'shouldBranch' is true, it performs a branch if the computation is true. Otherwise it simply returns the computation result.
//
template<bool compareForNotEqual, bool shouldBranch>
void NO_RETURN EqualityOperationImpl(TValue lhs, TValue rhs)
{
    bool result;
    // This is slightly tricky. We check 'rhs' first because the variants are specializing on 'rhs'.
    // Note that in theory, we could have made our TValue typecheck elimination supported "bitwise-equal" comparsion
    // (to optimize the "lhs.m_value == rhs.m_value" check a few lines below based on known type information).
    // If we had implemented that, then it would no longer matter whether we check 'lhs' first or 'rhs' first,
    // as our TValue typecheck elimination would give us the same optimal result in both cases.
    // However, we haven't implemented it yet, and even though it is not hard to implement it, for now let's just stay simple.
    //
    if (likely(rhs.Is<tDouble>()))
    {
        if (likely(lhs.Is<tDouble>()))
        {
            bool isEqualAsDouble = UnsafeFloatEqual(lhs.As<tDouble>(), rhs.As<tDouble>());
            result = isEqualAsDouble ^ compareForNotEqual;
            goto end;
        }
        else
        {
            goto not_equal;
        }
    }

    if (lhs.m_value == rhs.m_value)
    {
        result = true ^ compareForNotEqual;
        goto end;
    }

    assert(!lhs.Is<tInt32>() && "unimplemented");
    assert(!rhs.Is<tInt32>() && "unimplemented");

    if (likely(lhs.Is<tTable>() && rhs.Is<tTable>()))
    {
        // Consider metamethod call
        //
        HeapPtr<TableObject> lhsMetatable;
        {
            HeapPtr<TableObject> tableObj = lhs.As<tTable>();
            TableObject::GetMetatableResult gmr = TableObject::GetMetatable(tableObj);
            if (gmr.m_result.m_value == 0)
            {
                goto not_equal;
            }
            lhsMetatable = gmr.m_result.As<TableObject>();
        }

        HeapPtr<TableObject> rhsMetatable;
        {
            HeapPtr<TableObject> tableObj = rhs.As<tTable>();
            TableObject::GetMetatableResult gmr = TableObject::GetMetatable(tableObj);
            if (gmr.m_result.m_value == 0)
            {
                goto not_equal;
            }
            rhsMetatable = gmr.m_result.As<TableObject>();
        }

        TValue metamethod = GetMetamethodFromMetatableForComparisonOperation<true /*supportsQuicklyRuleOutMM*/>(lhsMetatable, rhsMetatable, LuaMetamethodKind::Eq);
        if (metamethod.Is<tNil>())
        {
            goto not_equal;
        }

        GetCallTargetConsideringMetatableResult callTarget = GetCallTargetConsideringMetatable(metamethod);
        if (callTarget.m_target.m_value == 0)
        {
            ThrowError(MakeErrorMessageForUnableToCall(metamethod));
        }

        if (unlikely(callTarget.m_invokedThroughMetatable))
        {
            MakeCall(callTarget.m_target.As(), metamethod, lhs, rhs, EqualityOperationMetamethodCallContinuation<compareForNotEqual, shouldBranch>);
        }
        else
        {
            MakeCall(callTarget.m_target.As(), lhs, rhs, EqualityOperationMetamethodCallContinuation<compareForNotEqual, shouldBranch>);
        }
    }

not_equal:
    result = false ^ compareForNotEqual;

end:
    if constexpr(shouldBranch)
    {
        if (result) { ReturnAndBranch(); } else { Return(); }
    }
    else
    {
        Return(TValue::Create<tBool>(result));
    }
}

}   // anonymous namespace

DEEGEN_DEFINE_BYTECODE_TEMPLATE(EqualityOperation, bool compareForNotEqual, bool shouldBranch)
{
    Operands(
        BytecodeSlotOrConstant("lhs"),
        BytecodeSlotOrConstant("rhs")
    );
    Result(shouldBranch ? ConditionalBranch : BytecodeValue);
    Implementation(EqualityOperationImpl<compareForNotEqual, shouldBranch>);
    Variant(
        Op("lhs").IsBytecodeSlot(),
        Op("rhs").IsBytecodeSlot()
    );
    Variant(
        Op("lhs").IsBytecodeSlot(),
        Op("rhs").IsConstant<tDouble>()
    );
    Variant(
        Op("lhs").IsBytecodeSlot(),
        Op("rhs").IsConstant<tString>()
    );
    Variant(
        Op("lhs").IsBytecodeSlot(),
        Op("rhs").IsConstant<tNil>()
    );
    Variant(
        Op("lhs").IsBytecodeSlot(),
        Op("rhs").IsConstant<tBool>()
    );
}

DEEGEN_DEFINE_BYTECODE_BY_TEMPLATE_INSTANTIATION(BranchIfEq, EqualityOperation, false /*compareForNotEqual*/, true /*shouldBranch*/);
DEEGEN_DEFINE_BYTECODE_BY_TEMPLATE_INSTANTIATION(BranchIfNotEq, EqualityOperation, true /*compareForNotEqual*/, true /*shouldBranch*/);

DEEGEN_END_BYTECODE_DEFINITIONS
