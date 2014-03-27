/***************************************************************************
                              qgswfsprojectparser.h
                              ---------------------
  begin                : March 25, 2014
  copyright            : (C) 2014 by Marco Hugentobler
  email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSWFSPROJECTPARSER_H
#define QGSWFSPROJECTPARSER_H

#include "qgsserverprojectparser.h"

class QgsWFSProjectParser
{
  public:
    QgsWFSProjectParser( QDomDocument* xmlDoc, const QString& filePath );
    ~QgsWFSProjectParser();

  private:
    QgsServerProjectParser mProjectParser;
};

#endif // QGSWFSPROJECTPARSER_H
