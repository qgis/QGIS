/***************************************************************************
                              qgswcsprojectparser.sip
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


class QgsWCSProjectParser
{
%TypeHeaderCode
#include "qgswcsprojectparser.h"

%End
  public:
    QgsWCSProjectParser( const QString& filePath );
    ~QgsWCSProjectParser();

    void serviceCapabilities( QDomElement& parentElement, QDomDocument& doc ) const;
    QString wcsServiceUrl() const;
    QString serviceUrl() const;
    void wcsContentMetadata( QDomElement& parentElement, QDomDocument& doc ) const;
    QStringList wcsLayers() const;
    void describeCoverage( const QString& aCoveName, QDomElement& parentElement, QDomDocument& doc ) const;
    QList<QgsMapLayer*> mapLayerFromCoverage( const QString& cName, bool useCache = true ) const;

};

