/****************************************************************************
** $Id: dl_exception.h 163 2003-07-01 15:51:48Z andrew $
**
** Copyright (C) 2001-2003 RibbonSoft. All rights reserved.
** Copyright (C) 2001 Robert J. Campbell Jr.
**
** This file is part of the dxflib project.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
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

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/**
 * Used for exception handling.
 */
class DL_Exception {}
;

/**
 * Used for exception handling.
 */
class DL_NullStrExc : public DL_Exception {}
;

/**
 * Used for exception handling.
 */
class DL_GroupCodeExc : public DL_Exception
{
    DL_GroupCodeExc( int gc = 0 ) : groupCode( gc ) {}
    int groupCode;
};
#endif

