/***************************************************************************
    extraitemutils.h
    ---------------------
    begin                : 2025/11/05
    copyright            : (C) 2025 by Julien Cabieces
    email                : julien dot cabieces at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSEXTRAITEMUTILS_H
#define QGSEXTRAITEMUTILS_H

#define SIP_NO_FILE

#include "qgis.h"
#include "qgis_core.h"

#include <QPair>

class QgsRenderContext;

/**
 * \ingroup core
 * \brief Helper class to manipulate extra items
 * \since QGIS 4.2
*/
class CORE_EXPORT QgsExtraItemUtils
{
  public:

    typedef QList<std::tuple<double, double, double>> ExtraItems;

    /**
       * Parse extra items string representation \a strExtraItems
       *
       * Extra items format is expected to be in the form "2.5 4.5 8.1,2 7 0, 4 8 12" where each
       * triplet represent respectively X, Y, Angle where X,Y is the extra item position expressed in its layer CRS unit,
       * and Angle is the item rotation angle in degree
       *
       * Returns a list of extra items
       */
    static ExtraItems parseExtraItems( const QString &strExtraItems, QString &error );

  private:

};

#endif
