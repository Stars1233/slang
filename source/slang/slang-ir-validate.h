// slang-ir-validate.h
#pragma once

namespace Slang
{
struct CodeGenContext;
class CompileRequestBase;
class DiagnosticSink;
class TargetRequest;
struct IRModule;
struct IRInst;

// Validate that an IR module obeys the invariants we need to enforce.
// For example:
//
// * Confirm that linked lists for children and for use-def chains are consistent
//   (e.g., x.next.prev == x)
//
// * Confirm that parent/child relationships are correct (e.g., if is `x` is in
//   `y.children`, then `x.parent == y`
//
// * Confirm that every operand of an instruction is valid to reference (i.e., it
//   must either be defined earlier in the same block, in a different block that
//   dominates the current one, or in a parent instruction of the block.
//
// * Confirm that every block ends with a terminator, and there are no terminators
//   elsewhere in a block.
//
// * Confirm that all the parameters of a block come before any "ordinary" instructions.
void validateIRModule(IRModule* module, DiagnosticSink* sink);
void validateIRInst(IRInst* inst);

// A wrapper that calls `validateIRModule` only when IR validation is enabled
// for the given compile request.
void validateIRModuleIfEnabled(CompileRequestBase* compileRequest, IRModule* module);

void validateIRModuleIfEnabled(CodeGenContext* codeGenContext, IRModule* module);

// RAII class to manage IR validation state in an exception-safe manner
class [[nodiscard]] IRValidationScope
{
public:
    // Constructor saves current state and sets new state
    explicit IRValidationScope(bool enableValidation);

    // Destructor automatically restores previous state
    ~IRValidationScope();

    // Non-copyable to prevent accidental copies
    IRValidationScope(const IRValidationScope&) = delete;
    IRValidationScope& operator=(const IRValidationScope&) = delete;

    // Non-movable to keep it simple
    IRValidationScope(IRValidationScope&&) = delete;
    IRValidationScope& operator=(IRValidationScope&&) = delete;

private:
    bool m_previousState;
};

// Convenience functions to create scoped guards
[[nodiscard]] inline IRValidationScope enableIRValidationScope()
{
    return IRValidationScope(true);
}

[[nodiscard]] inline IRValidationScope disableIRValidationScope()
{
    return IRValidationScope(false);
}

// Validate that the destination of an atomic operation is appropriate, meaning it's
// either 'groupshared' or in a device buffer.
// Note that validation of atomic operations should be done after address space
// specialization for targets (e.g. SPIR-V and Metal) which support this kind of use-case:
//   void atomicOp(inout int array){ InterlockedAdd(array, 1);}
//   groupshared int gArray;
//   [numthreads(1, 1, 1)] void main() { atomicOp(gArray); }
// If 'skipFuncParamValidation' is true, then the validation allows destinations that
// lead back to in/inout parameters that we can't validate.
void validateAtomicOperations(bool skipFuncParamValidation, DiagnosticSink* sink, IRInst* inst);

void validateVectorsAndMatrices(
    IRModule* module,
    DiagnosticSink* sink,
    TargetRequest* targetRequest);

} // namespace Slang
