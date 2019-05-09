#include "llvm/IR/Instructions.h"
#include "llvm/Pass.h"
#include "llvm/Support/InstIterator.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/GlobalValue.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Module.h"

using namespace llvm;
namespace{

struct fun_insert : public FunctionPass{
  static char ID;

  fun_insert():FunctionPass(ID){}

  virtual bool runOnFunction(Function &F){
    if (F.getName() == "main"){
      for (inst_iterator I = inst_begin(F), E= inst_end(F); I != E; ++I){               
        Instruction *inst = &*I;
        if(dyn_cast<AllocaInst> (inst)){
          errs() << "test" << "\n";
          GlobalVariable *virtAddr = new GlobalVariable(*F.getParent(),
              Type::getInt8PtrTy(F.getContext()), false,
              GlobalValue::ExternalLinkage, 0, "virt_addr");
          virtAddr->setAlignment(4);                    

          Module *M = F.getParent();

          Constant *c = M->getOrInsertFunction("open",
              IntegerType::get(F.getContext(),32),
              PointerType::get(Type::getInt8PtrTy(F.getContext(), 8),8),
              IntegerType::get(F.getContext(),32), NULL);
          Function *open = cast<Function>(c);

          //ConstantInt *a = ConstantInt::get(M->getContext(), APInt(32, 9437184));

          IRBuilder<> builder(inst);
          Value *strPtr = builder.CreateGlobalStringPtr("/dev/mem", ".str");

          ConstantInt *a = builder.getInt32(9437184);
          //CallInst *openRet = builder.CreateCall2(open, strPtr, a, "open");

          builder.CreateCall2(open, strPtr, a, "open");
        }
        errs() << *inst <<"\n"; 
      }
    }   
    return false;
  }
};

}



char fun_insert::ID=0;
static RegisterPass<fun_insert> 
X("fun_insert", "Insert Function Test", false, false);