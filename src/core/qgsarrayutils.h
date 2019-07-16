/***************************************************************************
                                  qgsarrayutils.h
                              ---------------------
    begin                : July 2019
    copyright            : (C) 2019 by David Signer
    email                : david at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSARRAYUTILS_H
#define QGSARRAYUTILS_H

#include "qgis_core.h"
#include "qgis.h"
#include "qlist.h"

#ifdef SIP_RUN
% ModuleHeaderCode
#include "qgsarrayutils.h"
% End
#endif

/**
 * \ingroup core
 * The QgsArrayUtils namespace provides functions to handle postgres array like formatted lists in strings.
 * \since QGIS 3.4
 */
namespace QgsArrayUtils
{

  /**
   * Returns a QVariantList created out of a string containing an array in postgres array format {1,2,3} or {"a","b","c"}
   * \param string The formatted list in a string
   * \since QGIS 3.8
   */
  CORE_EXPORT QVariantList parse( const QString &string );

  /**
   * Build a hstore-formatted string from a QVariantMap.
   * \param map The map to format as a string
   * \since QGIS 3.8
   */
  CORE_EXPORT QString build( const QVariantList &list );

};

#endif //QGSARRAYUTILS_H
