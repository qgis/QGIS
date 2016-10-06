/***************************************************************************
                              qgswcsprojectparser.h
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

#ifndef QGSWCSPROJECTPARSER_H
#define QGSWCSPROJECTPARSER_H

#include "qgsserverprojectparser.h"

#ifdef HAVE_SERVER_PYTHON_PLUGINS
class QgsAccessControl;
#endif

class SERVER_EXPORT QgsWCSProjectParser
{
  public:
    QgsWCSProjectParser(
      const QString& filePath
#ifdef HAVE_SERVER_PYTHON_PLUGINS
      , const QgsAccessControl* ac
#endif
    );
    ~QgsWCSProjectParser();

    void serviceCapabilities( QDomElement& parentElement, QDomDocument& doc ) const;
    QString wcsServiceUrl() const;
    QString serviceUrl() const;
    void wcsContentMetadata( QDomElement& parentElement, QDomDocument& doc ) const;
    QStringList wcsLayers() const;
    void describeCoverage( const QString& aCoveName, QDomElement& parentElement, QDomDocument& doc ) const;
    QList<QgsMapLayer*> mapLayerFromCoverage( const QString& cName, bool useCache = true ) const;

  private:
    QgsServerProjectParser* mProjectParser;
#ifdef HAVE_SERVER_PYTHON_PLUGINS
    const QgsAccessControl* mAccessControl;
#endif

    QgsWCSProjectParser( const QgsWCSProjectParser& rh );
    QgsWCSProjectParser& operator=( const QgsWCSProjectParser& rh );
};

#endif // QGSWCSPROJECTPARSER_H
