/***************************************************************************
    qgswfsrequest.h
    ---------------------
    begin                : February 2016
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
#ifndef QGSWFSREQUEST_H
#define QGSWFSREQUEST_H

#include "qgsbasenetworkrequest.h"
#include "qgswfscapabilities.h"
#include "qgswfsdatasourceuri.h"

#include <QAuthenticator>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QObject>
#include <QUrl>

//! Abstract base class for a WFS request.
class QgsWfsRequest : public QgsBaseNetworkRequest
{
    Q_OBJECT
  public:
    explicit QgsWfsRequest( const QgsWFSDataSourceURI &uri );

    //! Returns the url for a WFS request
    [[nodiscard]] QUrl requestUrl( const QString &request ) const;

  protected:
    [[nodiscard]] QDomDocument createPostDocument() const;
    QDomElement createRootPostElement(
      const QgsWfsCapabilities &capabilities,
      const QString &wfsVersion,
      QDomDocument &postDocument,
      const QString &name,
      const QStringList &typeNamesForNamespaces = QStringList()
    ) const;

    //! URI
    QgsWFSDataSourceURI mUri;
};

#endif // QGSWFSREQUEST_H
