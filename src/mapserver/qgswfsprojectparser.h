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

    void serviceCapabilities( QDomElement& parentElement, QDomDocument& doc ) const;
    QString serviceUrl() const;
    QString wfsServiceUrl() const;
    void featureTypeList( QDomElement& parentElement, QDomDocument& doc ) const;

    void describeFeatureType( const QString& aTypeName, QDomElement& parentElement, QDomDocument& doc ) const;

    QStringList wfsLayers() const;
    int wfsLayerPrecision( const QString& aLayerId ) const;

    QList<QgsMapLayer*> mapLayerFromTypeName( const QString& aTypeName, bool useCache = true ) const;

  private:
    QgsServerProjectParser mProjectParser;

    QStringList wfstUpdateLayers() const;
    QStringList wfstInsertLayers() const;
    QStringList wfstDeleteLayers() const;
};

#endif // QGSWFSPROJECTPARSER_H
