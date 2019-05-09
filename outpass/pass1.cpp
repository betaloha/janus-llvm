//===- Hello.cpp - Example code from "Writing an LLVM Pass" ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements two versions of the LLVM "Hello World" pass described
// in docs/WritingAnLLVMPass.html
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "opCounter"
#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Value.h"
#include <map>
using namespace llvm;
namespace
{
struct CountOp : public FunctionPass
{
  std::map<std::string, int> opCounter;
  static char ID;
  CountOp() : FunctionPass(ID) {}
  virtual bool runOnFunction(Function &F)
  {
    errs() << "Function " << F.getName() << '\n';
    for (inst_iterator I = inst_begin(F), E= inst_end(F); I != E; ++I){
    Instruction *inst = &*I;
    //check if the instruction is foo
    if(CallInst *c1 = dyn_cast<CallInst> (inst)){
      errs() << "This is a call instruction" << '\n';
      Function *calledFunc = c1->getCalledFunction();
      if(calledFunc){
        errs() << calledFunc->getName() << '\n';
        if(calledFunc->getName()=="foo"){
          continue;
        }
        //if the instruction is foo, inject laa
        Module *M = F.getParent();

          Constant *c = M->getOrInsertFunction("laa",
              IntegerType::get(F.getContext(),32), NULL);
          Function *laa = cast<Function>(c);

          //ConstantInt *a = ConstantInt::get(M->getContext(), APInt(32, 9437184));

          IRBuilder<> builder(inst);
          //Value *strPtr = builder.CreateGlobalStringPtr("/dev/mem", ".str");

          //ConstantInt *a = builder.getInt32(9437184);
          //CallInst *openRet = builder.CreateCall2(open, strPtr, a, "open");

          builder.CreateCall(laa);

      }


    }
    
    /*
    for (Function::iterator bb = F.begin(), e = F.end(); bb != e; ++bb)
    {
      for (BasicBlock::iterator i = bb->begin(), e = bb->end(); i != e; ++i)
      {
        errs() << i->getOpcodeName() << '\n';
        int operandCount = i->getNumOperands();
        for(int k=0;k<operandCount;k++){
          errs() << "Operand: " << k << ' ' << i->getOperand(k) << '\n';
        }
      }
    }
    */
    }
    return false;
  }
};
} // namespace
char CountOp::ID = 0;
static RegisterPass<CountOp> X("opCounter", "Counts opcodes per functions");