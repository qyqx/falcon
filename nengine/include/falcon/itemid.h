/*
   FALCON - The Falcon Programming Language.
   FILE: itemid.h

   List of item ids
   -------------------------------------------------------------------
   Author: Giancarlo Niccolai
   Begin: ven ott 15 2004

   -------------------------------------------------------------------
   (C) Copyright 2004: the FALCON developers (see list in AUTHORS file)

   See LICENSE file for licensing details.
*/

#ifndef FALCON_ITEMID_H
#define FALCON_ITEMID_H

/* First, the flat items */

#define FLC_ITEM_NIL          0
#define FLC_ITEM_BOOL         1
#define FLC_ITEM_INT          2
#define FLC_ITEM_NUM          3

/* User items are ungarbaged items.
 */
#define FLC_ITEM_USER         4

/* Framing items. They are used as special markers in the item stack. */
#define FLC_ITEM_FRAMING      5

/* This marks the last flat item */

#define FLC_ITEM_FLAT         5

/*
 Theese are hybrid flat-deep items.
 They are flat with regards to the item system, that is, they are
 copied flat, while they are deep with regards to the GC, that is,
 they require ad-hoc marking.

 Function and methods keep their mark token inside their own structure,
 and declare themselves to the GC in case of need.
*/
#define FLC_ITEM_FUNC         6
#define FLC_ITEM_METHOD       7
#define FLC_ITEM_BASEMETHOD   8

/* From this point on, we have deep items -- garbage controlled */
#define FLC_ITEM_DEEP         9

#define FLC_ITEM_COUNT       10

// Theese are the class IDs, used to get the typeID of this item.
#define FLC_CLASS_ID_STRING   8
#define FLC_CLASS_ID_FUNCTION 9
#define FLC_CLASS_ID_ARRAY    10
#define FLC_CLASS_ID_DICT     11
#define FLC_CLASS_ID_RANGE    12
#define FLC_CLASS_ID_CLASS    13

// Other generic object
#define FLC_CLASS_ID_OBJECT   20


#define FLC_ITEM_INVALID      99

#endif

/* end of itemid.h */