/***************************************************************************
    qgsxmlutils.h
    ---------------------
    begin                : December 2013
    copyright            : (C) 2013 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSXMLUTILS_H
#define QGSXMLUTILS_H

class QDomDocument;
class QDomElement;

class QgsRectangle;

#include "qgis.h"

/** \ingroup core
 * Assorted helper methods for reading and writing chunks of XML
 */
class CORE_EXPORT QgsXmlUtils
{
  public:

    /* reading */

    static QGis::UnitType readMapUnits( const QDomElement& element );
    static QgsRectangle readRectangle( const QDomElement& element );

    /* writing */

    static QDomElement writeMapUnits( QGis::UnitType units, QDomDocument& doc );
    static QDomElement writeRectangle( const QgsRectangle& rect, QDomDocument& doc );
};


#endif // QGSXMLUTILS_H
