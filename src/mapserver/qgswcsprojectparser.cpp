/***************************************************************************
                              qgswcsprojectparser.cpp
                              -----------------------
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

#include "qgswcsprojectparser.h"

QgsWCSProjectParser::QgsWCSProjectParser( QDomDocument* xmlDoc, const QString& filePath ): mProjectParser( xmlDoc, filePath )
{
}

QgsWCSProjectParser::~QgsWCSProjectParser()
{
}
