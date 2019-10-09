/***************************************************************************
    qgswfsutils.h
    ---------------------
    begin                : March 2016
    copyright            : (C) 2016 by Even Rouault
    email                : even.rouault at spatialys.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSWFSUTILS_H
#define QGSWFSUTILS_H

#include <QString>

/**
 * Utility class */
class QgsWFSUtils
{
  public:
    //! Removes a possible namespace prefix from a typename
    static QString removeNamespacePrefix( const QString &tname );
    //! Returns namespace prefix (or an empty string if there is no prefix)
    static QString nameSpacePrefix( const QString &tname );
};

#endif // QGSWFSUTILS_H
