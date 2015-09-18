/***************************************************************************
                              qgswfsprojectparser.sip
                              ------------------------
  begin                : May 2, 2015
  copyright            : (C) 2015 by Alessandro Pasotti
  email                : a dot pasotti at itopen dot it
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


class QgsWFSProjectParser
{
%TypeHeaderCode
#include "qgswfsprojectparser.h"

%End

  public:
    QgsWFSProjectParser( const QString& filePath );
    ~QgsWFSProjectParser();

    void serviceCapabilities( QDomElement& parentElement, QDomDocument& doc ) const;
    QString serviceUrl() const;
    QString wfsServiceUrl() const;
    void featureTypeList( QDomElement& parentElement, QDomDocument& doc ) const;

    void describeFeatureType( const QString& aTypeName, QDomElement& parentElement, QDomDocument& doc ) const;

    QStringList wfsLayers() const;
    QSet<QString> wfsLayerSet() const;
    int wfsLayerPrecision( const QString& aLayerId ) const;

    QList<QgsMapLayer*> mapLayerFromTypeName( const QString& aTypeName, bool useCache = true ) const;

    QSet<QString> wfstUpdateLayers() const;
    QSet<QString> wfstInsertLayers() const;
    QSet<QString> wfstDeleteLayers() const;

};
