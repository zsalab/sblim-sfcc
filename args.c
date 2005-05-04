/*!
  \file args.c
  \brief Native CMPIArgs implementation.

  This is the native CMPIArgs implementation as used for remote
  providers. It reflects the well-defined interface of a regular
  CMPIArgs object, however, it works independently from the management broker.
  
  It is part of a native broker implementation that simulates CMPI data
  types rather than interacting with the entities in a full-grown CIMOM.

  (C) Copyright IBM Corp. 2003
 
  THIS FILE IS PROVIDED UNDER THE TERMS OF THE COMMON PUBLIC LICENSE 
  ("AGREEMENT"). ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS FILE 
  CONSTITUTES RECIPIENTS ACCEPTANCE OF THE AGREEMENT.
 
  You can obtain a current copy of the Common Public License from
  http://oss.software.ibm.com/developerworks/opensource/license-cpl.html

  \author Frank Scheffler
  $Revision: 1.1 $
*/

#include <stdio.h>
#include "cmcidt.h"
#include "cmcift.h"
#include "cmcimacs.h"
#include "tool.h"
#include "native.h"


//! Native extension of the CMPIArgs data type.
/*!
  This structure stores the information needed to represent arguments for
  CMPI providers, i.e. within invokeMethod() calls.
 */
struct native_args {
	CMPIArgs args;	        /*!< the inheriting data structure  */
	int mem_state;		/*!< states, whether this object is
				  registered within the memory mangagement or
				  represents a cloned object */

	struct native_property * data;	/*!< argument content */
};


static struct native_args * __new_empty_args ( int, CMPIStatus * );

/****************************************************************************/


static CMPIStatus __aft_release ( CMPIArgs * args )
{
	struct native_args * a = (struct native_args *) args;

	if ( a->mem_state == TOOL_MM_NO_ADD ) {

		tool_mm_add ( a );
		a->mem_state = TOOL_MM_ADD;
		propertyFT.release ( a->data );

		CMReturn ( CMPI_RC_OK );
	}

	CMReturn ( CMPI_RC_ERR_FAILED );
}


static CMPIArgs * __aft_clone ( CMPIArgs * args, CMPIStatus * rc )
{
	struct native_args * a   = (struct native_args *) args;
	struct native_args * new = __new_empty_args ( TOOL_MM_NO_ADD, rc );

	if ( rc->rc == CMPI_RC_OK ) {
		new->data = propertyFT.clone ( a->data, rc );
	}

	return (CMPIArgs *) new;
}


static CMPIStatus __aft_addArg ( CMPIArgs * args,
				 char * name,
				 CMPIValue * value,
				 CMPIType type )
{
	struct native_args * a = (struct native_args *) args;

	CMReturn ( ( propertyFT.addProperty ( &a->data,
					      a->mem_state,
					      name,
					      type,
					      0,
					      value ) )?
		   CMPI_RC_ERR_ALREADY_EXISTS: 
		   CMPI_RC_OK );
}


static CMPIData __aft_getArg ( CMPIArgs * args,
			       char * name,
			       CMPIStatus * rc )
{
	struct native_args * a = (struct native_args *) args;

	return propertyFT.getDataProperty ( a->data, name, rc );
}


static CMPIData __aft_getArgAt ( CMPIArgs * args, 
				 unsigned int index,
				 CMPIString ** name,
				 CMPIStatus * rc )
{
	struct native_args * a = (struct native_args *) args;

	return propertyFT.getDataPropertyAt ( a->data, index, name, rc );
}


static unsigned int __aft_getArgCount ( CMPIArgs * args, CMPIStatus * rc )
{
	struct native_args * a = (struct native_args *) args;
  
	return propertyFT.getPropertyCount ( a->data, rc );
}


static struct native_args * __new_empty_args ( int mm_add, CMPIStatus * rc )
{
	static CMPIArgsFT aft = {
		NATIVE_FT_VERSION,
		__aft_release,
		__aft_clone,
		__aft_addArg,
		__aft_getArg,
		__aft_getArgAt,
		__aft_getArgCount
	};
	static CMPIArgs a = {
		"CMPIArgs",
		&aft
	};

	struct native_args * args =
		(struct native_args *) 
		tool_mm_alloc ( mm_add, sizeof ( struct native_args ) );

	args->args      = a;
	args->mem_state = mm_add;

	if ( rc ) CMSetStatus ( rc, CMPI_RC_OK );
	return args;
}


CMPIArgs * native_new_CMPIArgs ( CMPIStatus * rc )
{
	return (CMPIArgs *) __new_empty_args ( TOOL_MM_ADD, rc );
}

/*****************************************************************************/

/*** Local Variables:  ***/
/*** mode: C           ***/
/*** c-basic-offset: 8 ***/
/*** End:              ***/
