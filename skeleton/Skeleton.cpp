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
        std::vector<Type *> paramTypes = { Type::getInt8PtrTy(Ctx), Type::getInt8PtrTy(Ctx)};
        Type *retType = Type::getVoidTy(Ctx);
        FunctionType *logFuncType = FunctionType::get(retType, paramTypes, false);       
        M->getOrInsertFunction("wrapper", logFuncType);
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

    virtual bool runOnFunction(Function &F) {
      declareFuncs(F);

      Module *M = F.getParent();

      FunctionCallee logFunc = M->getFunction("logop");
      FunctionCallee readTimeFunc = M->getFunction("read_time");
      FunctionCallee wrapper = M->getFunction("wrapper");

      std::vector<string> funcs = { "printf" };
      std::vector<string> calc_func = {"add", "sub", "mul", "divide"};

      bool modified = false;

      if (find(calc_func, F.getName()) != calc_func.end())
      {
        BasicBlock &entryBlock = F.getEntryBlock();
        Instruction *firstInst = entryBlock.getFirstNonPHI();
        IRBuilder<> IRB(firstInst);

        errs() << "Function " << wrapper.getCallee()->getName();
        errs() << " is inserted at the front of " << F.getName() << "\n";

        // Insert *after* `op`.
        IRB.SetInsertPoint(firstInst);

        // Insert a call to our function.
        Value *callee;
        Value *func;
        Value *args[] = { callee, func };
        IRB.CreateCall(wrapper);
        // IRB.CreateLoad(Ptr, )

        modified |= true;
      }

      for (auto &B : F)
      {
        for (auto &I : B) {
          branches(I);
          if (auto *CI = dyn_cast<CallInst>(&I)) {
            if (auto *f = CI->getCalledFunction()) {
              if (find(funcs, f->getName()) != funcs.end())
              {
                errs() << "Function " << readTimeFunc.getCallee()->getName();
                errs() << " is inserted before " << f->getName() << "\n";

                // Insert *after* `op`.
                IRBuilder<> IRB(CI);
                // builder.SetInsertPoint(&B, builder.GetInsertPoint());
                IRB.SetInsertPoint(&I);

                // Insert a call to our function.
                // Value* args[] = {op};

                IRB.CreateCall(readTimeFunc);

                modified |= true;
              }
            }
          }
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
