/*
   FALCON - The Falcon Programming Language.
   FILE: classstatement.cpp

   Base class for statement PStep handlers.
   -------------------------------------------------------------------
   Author: Giancarlo Niccolai
   Begin: Tue, 27 Dec 2011 21:39:56 +0100

   -------------------------------------------------------------------
   (C) Copyright 2011: the FALCON developers (see list in AUTHORS file)

   See LICENSE file for licensing details.
*/


#include <falcon/setup.h>
#include <falcon/string.h>
#include <falcon/classes/classstatement.h>
#include <falcon/classes/classtreestep.h>

namespace Falcon {

ClassStatement::ClassStatement( ClassTreeStep* parent ):
   DerivedFrom( parent, "Statement" )
{}
   
ClassStatement::~ClassStatement(){}


}

/* end of classstatement.cpp */