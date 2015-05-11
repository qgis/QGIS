/****************************************************************************
** Copyright (C) 2001-2013 RibbonSoft, GmbH. All rights reserved.
** Copyright (C) 2001 Robert J. Campbell Jr.
**
** This file is part of the dxflib project.
**
** This file is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** Licensees holding valid dxflib Professional Edition licenses may use 
** this file in accordance with the dxflib Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.ribbonsoft.com for further details.
**
** Contact info@ribbonsoft.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef DL_EXCEPTION_H
#define DL_EXCEPTION_H

#include "dl_global.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/**
 * Used for exception handling.
 */
class DXFLIB_EXPORT DL_Exception {}
;

/**
 * Used for exception handling.
 */
class DXFLIB_EXPORT DL_NullStrExc : public DL_Exception {}
;

/**
 * Used for exception handling.
 */
class DXFLIB_EXPORT DL_GroupCodeExc : public DL_Exception {
    DL_GroupCodeExc(int gc=0) : groupCode(gc) {}
    int groupCode;
};
#endif

