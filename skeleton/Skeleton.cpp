#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/IR/Module.h"

#include <string>
#include <vector>

using namespace llvm;
using namespace std;

namespace
{
  struct SkeletonPass : public FunctionPass {
    static char ID;
    SkeletonPass() : FunctionPass(ID) {  }

    void declareFuncs(Function &F) {
      // Get the function to call from our runtime library.
      LLVMContext &Ctx = F.getContext();
      Module *M = F.getParent();
      
      {
        std::vector<Type *> paramTypes = {Type::getInt32Ty(Ctx)};
        Type *retType = Type::getVoidTy(Ctx);
        FunctionType *logFuncType = FunctionType::get(retType, paramTypes, false);       
        M->getOrInsertFunction("logop", logFuncType);
      }
      {
        Type *retType = Type::getVoidTy(Ctx);
        FunctionType *logFuncType = FunctionType::get(retType, false);       
        M->getOrInsertFunction("bcs_init", logFuncType);
      }
      {
        Type *retType = Type::getVoidTy(Ctx);
        FunctionType *logFuncType = FunctionType::get(retType, false);       
        M->getOrInsertFunction("bcs_checking", logFuncType);
      }
      {
        Type *retType = Type::getVoidTy(Ctx);
        FunctionType *readTimeFuncType = FunctionType::get(retType, false);
        M->getOrInsertFunction("read_time", readTimeFuncType);
      }
    }

    void branches(Instruction &I){
      if (CallInst *CI = dyn_cast<CallInst>(&I)) {
        // Print out the function name
        if(Function *f = CI->getCalledFunction()) {
          outs () << " |-getCalledFunction=" << f->getName() << "\n";
        }
      }
      if(InvokeInst *II = dyn_cast<InvokeInst>(&I)) {
        // Print out the function name
        outs () << " |-getNumDestinations=" << II->getCalledFunction()->getName() << "\n";
      }
      if(IndirectBrInst *IBI = dyn_cast<IndirectBrInst>(&I)) {
        // Print out the function name
        outs () << " |-getNumDestinations=" << IBI->getNumDestinations() << "\n";
      }
    }

    void insert_func(Function &F, FunctionCallee callee) {
        BasicBlock &entryBlock = F.getEntryBlock();
        Instruction *firstInst = entryBlock.getFirstNonPHI();
        IRBuilder<> IRB(firstInst);

        errs() << "Function " << callee.getCallee()->getName();
        errs() << " is inserted at the front of " << F.getName() << "\n";

        // Insert *after* `op`.
        IRB.SetInsertPoint(firstInst);

        // Insert a call to our function.
        IRB.CreateCall(callee);
    }

    bool checking_before_call(Instruction &I, FunctionCallee callee, std::vector<string> &funcs) {
      bool modified = false;
      // is a CallInst
      if (auto *CI = dyn_cast<CallInst>(&I)) {
        // has function called
        if (auto *f = CI->getCalledFunction()) {
          // is to be inserted before
          if (find(funcs, f->getName()) != funcs.end()) {

            errs() << "Function " << callee.getCallee()->getName();
            errs() << " is inserted before " << f->getName() << "\n";

            // Insert *after* `op`.
            IRBuilder<> IRB(CI);
            IRB.SetInsertPoint(&I);

            // Insert a call to our function.
            // Value* args[] = {op};

            IRB.CreateCall(callee);

            modified |= true;
          }
        }
      }

      return modified;
    }

    virtual bool runOnFunction(Function &F) {
      declareFuncs(F);

      Module *M = F.getParent();

      bool modified = false;

      // call bcs_init before execution of main
      if (F.getName() == "main") {
        insert_func(F, M->getFunction("bcs_init"));
        modified |= true;
      }

      // call bcs_checking before execution of calc_func
      std::vector<string> calc_func = {"add", "sub", "mul", "divide"};
      if (find(calc_func, F.getName()) != calc_func.end()) {
        insert_func(F, M->getFunction("bcs_checking"));
        modified |= true;
      }
 
      // call read_time before printf
      std::vector<string> funcs = { "printf" };
      for (auto &B : F) {
        for (auto &I : B) {
          branches(I);
          checking_before_call(I, M->getFunction("read_time"), funcs);
        }
      }

      return modified;
    }
  };
}

char SkeletonPass::ID = 0;

// Automatically enable the pass.
// http://adriansampson.net/blog/clangpass.html
static void registerSkeletonPass(const PassManagerBuilder &,
                         legacy::PassManagerBase &PM) {
  PM.add(new SkeletonPass());
}
static RegisterStandardPasses
  RegisterMyPass(PassManagerBuilder::EP_EarlyAsPossible,
                 registerSkeletonPass);
