/*
 * main.cpp
 *
 *  Created on: 12/gen/2011
 *      Author: gian
 */

#include <cstdio>
#include <iostream>

#include <falcon/vm.h>
#include <falcon/syntree.h>
#include <falcon/symbol.h>
#include <falcon/expression.h>
#include <falcon/statement.h>

#include <falcon/synfunc.h>
#include <falcon/application.h>
#include <falcon/psteps/exprsym.h>

#include <falcon/psteps/stmtautoexpr.h>
#include <falcon/psteps/stmtreturn.h>
#include <falcon/psteps/stmtwhile.h>
#include <falcon/psteps/exprvalue.h>
#include <falcon/psteps/exprmath.h>
#include <falcon/psteps/exprcompare.h>

#include <falcon/psteps/exprassign.h>

class LoopApp: public Falcon::Application
{

public:
void go()
{
   Falcon::SynFunc fmain( "__main__" );
   // create a program:
   // count = 0
   // while count < 5
   //   count = count + 1
   // end

   Falcon::Symbol* count = new Falcon::Symbol("count", Falcon::Symbol::e_st_local, 0);
   Falcon::SynTree* assign = new Falcon::SynTree;
   assign->append(
         new Falcon::StmtAutoexpr(
               new Falcon::ExprAssign( new Falcon::ExprSymbol(count),
                     new Falcon::ExprPlus( new Falcon::ExprSymbol(count), new Falcon::ExprValue(1) )
         )));


   Falcon::SynTree* program = &fmain.syntree();
   (*program)
      .append( new Falcon::StmtAutoexpr(new Falcon::ExprAssign( new Falcon::ExprSymbol(count), new Falcon::ExprValue(0) ) ) )
      .append( new Falcon::StmtWhile(
                     new Falcon::ExprLT( new Falcon::ExprSymbol(count), new Falcon::ExprValue(50000000) ),
                     assign ) );


   std::cout << program->describe().c_ize() << std::endl;

   // And now, run the code.
   Falcon::VMachine vm;
   vm.currentContext()->call(&fmain,0);
   vm.currentContext()->pushData(Falcon::Item());  // create an item -- local 0
   vm.run();

   std::cout << "Top: " << vm.regA().describe().c_ize() << std::endl;
}

};

// This is just a test.
int main( int , char* [] )
{
   std::cout << "Hello world" << std::endl;

   LoopApp loop;
   loop.go();
   return 0;
}