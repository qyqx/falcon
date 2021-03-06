/*
 *  Falcon DataMatrix - Extension
 */

#include "dmtx_ext.h"
#include "dmtx_mod.h"
//#include "dmtx_srv.h"
//#include "dmtx_st.h"

#include <falcon/engine.h>
#include <falcon/vm.h>

#include <stdio.h>//debug..

//extern Falcon::DataMatrixService theDataMatrixService;

/*#
    @beginmodule dmtx
 */

namespace Falcon
{
namespace Ext
{

/*#
    @class DataMatrixError
    @brief Error generated DataMatrix operations.
    @optparam code The error code
    @optparam desc The description for the error code
    @optparam extra Extra information specifying the error conditions.
    @from Error( code, desc, extra )
*/
FALCON_FUNC DataMatrixError_init( VMachine* vm )
{
    CoreObject *einst = vm->self().asObject();

    if ( einst->getUserData() == 0 )
        einst->setUserData( new DataMatrixError );

    ::Falcon::core::Error_init( vm );
}


/*#
    @class DataMatrix
    @brief DataMatrix codec

    @prop module_size
    @prop margin_size
    @prop gap_size
    @prop scheme
    @prop shape
    @prop timeout
    @prop shrink
    @prop deviation
    @prop threshold
    @prop min_edge
    @prop max_edge
    @prop corrections
    @prop max_count
 */
#if 0
FALCON_FUNC DataMatrix_init( VMachine* vm )
{
    Dmtx::DataMatrix* self = static_cast<Dmtx::DataMatrix*>( vm->self().asObjectSafe() );
    vm->retval( self );
}
#endif


/*#
    @method encode DataMatrix
    @param data A string or membuf
    @param context A context object
    @return true on success
 */
FALCON_FUNC DataMatrix_encode( VMachine* vm )
{
    Item* i_data = vm->param( 0 );
    Item* i_ctxt = vm->param( 1 );

    if ( !i_data || !( i_data->isString() || i_data->isMemBuf() )
        || !i_ctxt || !i_ctxt->isObject() )
    {
        throw new ParamError( ErrorParam( e_inv_params, __LINE__ )
                .extra( "S|M,O" ) );
    }

    Dmtx::DataMatrix* self = static_cast<Dmtx::DataMatrix*>( vm->self().asObjectSafe() );
    vm->retval( self->encode( *i_data, *i_ctxt ) );
}


/*#
    @method decode DataMatrix
    @param data A string or membuf
    @param width Width of scanned image
    @param height Height of scanned image
    @return An array of results, or nil on error
 */
FALCON_FUNC DataMatrix_decode( VMachine* vm )
{
    Item* i_data = vm->param( 0 );
    Item* i_w = vm->param( 1 );
    Item* i_h = vm->param( 2 );

    if ( !i_data || !( i_data->isString() || i_data->isMemBuf() )
        || !i_w || !i_w->isInteger()
        || !i_h || !i_h->isInteger() )
    {
        throw new ParamError( ErrorParam( e_inv_params, __LINE__ )
                .extra( "S|M,I,I" ) );
    }

    Dmtx::DataMatrix* self = static_cast<Dmtx::DataMatrix*>( vm->self().asObjectSafe() );
    CoreArray* res;
    if ( self->decode( *i_data, i_w->asInteger(), i_h->asInteger(), &res ) )
        vm->retval( res );
    else
        vm->retnil();
}


/*#
    @method resetOptions
    @brief Reset the options (properties).
 */
FALCON_FUNC DataMatrix_resetOptions( VMachine* vm )
{
    Dmtx::DataMatrix* self = static_cast<Dmtx::DataMatrix*>( vm->self().asObjectSafe() );
    self->resetOptions();
}

} /* !namespace Ext */
} /* !namespace Falcon */
