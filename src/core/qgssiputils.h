/***************************************************************************
                          qgssiputils.h

                             -------------------
    begin                : May 2025
    copyright            : (C) 2025 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSIPUTILS_H
#define QGSSIPUTILS_H
#include "qgis_core.h"
#include "qgis_sip.h"

/**
 * \brief Contains utilities for working with SIP Python objects.
 *
 * \ingroup core
 * \since QGIS 3.44
 */
class CORE_EXPORT QgsSipUtils
{
  public:

#ifdef SIP_RUN
    /**
     * Returns TRUE if an object is currently owned by Python.
     *
     * If FALSE is returned, then the object is currently owned by another
     * object (e.g. a c++ class).
     */
    static bool isPyOwned( SIP_PYOBJECT SIP_GETWRAPPER );
    % MethodCode
    if ( sipIsOwnedByPython( ( sipSimpleWrapper * )a0Wrapper ) )
    {
      sipRes = true;
    }
    else
    {
      sipRes = false;
    }
    % End
#endif

};

#endif //QGSSIPUTILS_H
