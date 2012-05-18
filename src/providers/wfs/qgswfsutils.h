/***************************************************************************
    qgswfsutils.h
    ---------------------
    begin                : January 2012
    copyright            : (C) 2012 by Martin Dobias
    email                : wonder.sk at gmail.com
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

class QgsExpression;
class QDomDocument;

class QgsWFSUtils
{
  public:

    //! Creates ogc filter xml document. Supports minimum standard filter according to the OGC filter specs (=,!=,<,>,<=,>=,AND,OR,NOT)
    //! @return true in case of success. False if string contains something that goes beyond the minimum standard filter
    static bool expressionToOGCFilter( QgsExpression& exp, QDomDocument& doc );
};

#endif // QGSWFSUTILS_H
