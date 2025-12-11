/***************************************************************************
    qgswfsfeaturehitsasyncrequest.h
    -------------------------------
    begin                : January 2013
    copyright            : (C) 2013 by Marco Hugentobler
                           (C) 2016 by Even Rouault
    email                : marco dot hugentobler at sourcepole dot ch
                           even.rouault at spatialys.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSWFSFEATUREHITSASYNCREQUEST_H
#define QGSWFSFEATUREHITSASYNCREQUEST_H

#include "qgswfsdatasourceuri.h"
#include "qgswfsrequest.h"

//! Utility class to issue a GetFeature resultType=hits request
class QgsWFSFeatureHitsAsyncRequest final : public QgsWfsRequest
{
    Q_OBJECT
  public:
    explicit QgsWFSFeatureHitsAsyncRequest( QgsWFSDataSourceURI &uri );

    void launchGet( const QUrl &url );
    void launchPost( const QUrl &url, const QByteArray &data );

    //! Returns result of request, or -1 if not known/error
    [[nodiscard]] int numberMatched() const { return mNumberMatched; }

  signals:
    void gotHitsResponse();

  private slots:
    void hitsReplyFinished();

  protected:
    QString errorMessageWithReason( const QString &reason ) override;

  private:
    int mNumberMatched = -1;
};

#endif
