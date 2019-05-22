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
#include <vector>
using namespace llvm;
namespace
{
struct CountOp : public FunctionPass
{
  Instruction *bottom;
  void pushForwardTop(Instruction *inst)
  {
    int operandCount = inst->getNumOperands();
    for (int j = 0; j < operandCount; j++)
    {
      Value *v = inst->getOperand(j);
      pushForward(v, inst);
    }
  }
  //std::vector<Instruction *> pushed_load;
  void pushForward(Value *v, Instruction *parent)
  {
    Instruction *inst = dyn_cast<Instruction>(v);
    if (inst == NULL)
    {
      return;
    }
    if (parent->getParent() != inst->getParent())
    {
      return;
    }
    inst->moveBefore(bottom);
    bottom = inst;
    const char *instOpcodeName = inst->getOpcodeName();
    int isLoad = (std::string(instOpcodeName).find("load") != std::string::npos);
    errs() << instOpcodeName << '\n';
    if (isLoad)
    {
      // If the instruction is load, we must also push associated store forward.
      // This is a bit hacky, if there are multiple stores associated with that addr,
      // this may not work.

      // Look for associated store instruction in the basicblock
      for (auto &I : *inst->getParent())
      {
        Instruction *inst2 = &I;
        const char *instOpcodeName2 = inst2->getOpcodeName();
        int isStore = (std::string(instOpcodeName2).find("store") != std::string::npos);
        if(!isStore){
          continue;
        }
        // If the load and store address is match, move the store before the load.
        // Get first operand of the load instruction
        Value *loadAddr = inst->getOperand(0);
        // Get second operand of the store instruction
        Value *storeAddr = inst2->getOperand(1);
        if (loadAddr == storeAddr)
        {
          inst2->moveBefore(inst);
          bottom=inst2;
          pushForwardTop(inst2);
          break;
        }
      }
    }

    
    pushForwardTop(inst);
  }
  std::vector<Instruction *> pending_load;
  std::map<std::string, int> opCounter;
  static char ID;
  CountOp() : FunctionPass(ID) {}
  virtual bool runOnFunction(Function &F)
  {
    int opt_id = 0;
    errs() << "Function " << F.getName() << '\n';
    //to do, change to basic block iterator
    for (auto &B : F)
    {
      pending_load.clear();
      if (F.size() > 1)
      {
        break;
      }
      for (auto &I : B)
      {
        Instruction *inst = &I;
        if (CallInst *c1 = dyn_cast<CallInst>(inst))
        {

          Function *calledFunc = c1->getCalledFunction();
          if (calledFunc)
          {
            // errs() << calledFunc->getName() << '\n';
            if (calledFunc->getName().find("NO_OPT") != std::string::npos)
            {
              errs() << "No OPT function----------------------\n";
              inst->eraseFromParent();
              return false;
            }
            if (calledFunc->getName().find("flush_caches") != std::string::npos)
            {

              errs() << "CALL: " << calledFunc->getName() << '\n';
              bottom = &*inst_begin(F);
              Instruction *injection_point = bottom; //->getNextNode();
              //pushForwardTop(inst);
              //Just need to insert OPT ADDR now.
              //get the address of the flush instruction
              Value *v1 = inst->getOperand(0);
              errs() << "v1 type:";
              v1->getType()->print(errs());
              errs() << '\n';
              //get the size of the flush instruction
              Value *v2 = inst->getOperand(1);
              errs() << "v2 type:";
              v2->getType()->print(errs());
              errs() << '\n';
              Module *M = F.getParent();
              ConstantInt *opt_id_const = ConstantInt::get(M->getContext(), APInt(32, opt_id));
              Constant *c = M->getOrInsertFunction("OPT_ADDR_AUTO",
                                                   Type::getVoidTy(F.getContext()),
                                                   v1->getType(),
                                                   v2->getType(),
                                                   opt_id_const->getType());
              Function *opt_addr_func = cast<Function>(c);
              //ConstantInt *opt_id_const = ConstantInt::get(M->getContext(), APInt(64, opt_id));
              //ConstantInt *thread_id_const = ConstantInt::get(M->getContext(), APInt(8, 0));
              //insert the opt before the first instruction
              IRBuilder<> builder(inst);

              Instruction *injectedInst = builder.CreateCall(opt_addr_func, {v1, v2, opt_id_const});
              injectedInst->moveAfter(B.getFirstNonPHI());
              opt_id++;
              pushForwardTop(inst);
              //OPT_ADDR((void*)idx, 0, addr, size);
            }
            else
            {
              if ((calledFunc->getName().find("memcpy") != std::string::npos))
              {
                // Dest
                Value *v1 = inst->getOperand(0);
                errs() << "v1 type:";
                v1->getType()->print(errs());
                errs() << '\n';
                // Src
                Value *v2 = inst->getOperand(1);
                errs() << "v2 type:";
                v2->getType()->print(errs());
                errs() << '\n';
                // Size
                Value *v3 = inst->getOperand(3);
                errs() << "v3 type:";
                v3->getType()->print(errs());
                errs() << '\n';
                // Get the ancestor of the Src.
                Instruction *src_inst = dyn_cast<Instruction>(v2);
                Instruction *target = inst;
                int isSelf = 0;
                if (src_inst != NULL)
                {
                  target = src_inst;
                  isSelf = 1;
                }

                Module *M = F.getParent();
                ConstantInt *opt_id_const = ConstantInt::get(M->getContext(), APInt(32, opt_id));
                Constant *c = M->getOrInsertFunction("OPT_AUTO",
                                                     Type::getVoidTy(F.getContext()),
                                                     v1->getType(),
                                                     v2->getType(),
                                                     v3->getType(),
                                                     opt_id_const->getType());
                Function *opt_func = cast<Function>(c);
                //ConstantInt *thread_id_const = ConstantInt::get(M->getContext(), APInt(8, 0));
                //insert the opt before the first instruction
                IRBuilder<> builder(inst);

                Instruction *injected_inst = builder.CreateCall(opt_func, {v1, v2, v3, opt_id_const});

                if (isSelf)
                {
                  injected_inst->moveAfter(inst);
                }
                else
                {
                  injected_inst->moveAfter(inst);
                }

                opt_id++;
              }
            }
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
      /*
      for (auto &P : pending_load)
      {
        Instruction *inst = &I;
      }
      */
    }
    errs() << "done\n";
    return false;
  }
};
} // namespace
char CountOp::ID = 0;
static RegisterPass<CountOp> X("optAddr", "Counts opcodes per functions");
