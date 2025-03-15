/***************************************************************************
    qgswfsgetcapabilities.h
    ---------------------
    begin                : October 2011
    copyright            : (C) 2011 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSWFSGETCAPABILITIES_H
#define QGSWFSGETCAPABILITIES_H

#include <QObject>
#include <QDomElement>

#include "qgswfsrequest.h"
#include "qgsdataprovider.h"
#include "qgswfscapabilities.h"

//! Manages the GetCapabilities request
class QgsWfsGetCapabilitiesRequest : public QgsWfsRequest
{
    Q_OBJECT
  public:
    explicit QgsWfsGetCapabilitiesRequest( const QString &uri, const QgsDataProvider::ProviderOptions &options = QgsDataProvider::ProviderOptions() );

    //! returns request URL
    QUrl requestUrl() const;

    //! start network connection to get capabilities
    bool requestCapabilities( bool synchronous, bool forceRefresh );

    //! Application level error
    enum class ApplicationLevelError
    {
      NoError,
      XmlError,
      VersionNotSupported,
    };

    //! Returns parsed capabilities - requestCapabilities() must be called before
    const QgsWfsCapabilities &capabilities() const { return mCaps; }

    //! Returns application level error
    ApplicationLevelError applicationLevelError() const { return mAppLevelError; }

  signals:
    //! emitted when the capabilities have been fully parsed, or an error occurred */
    void gotCapabilities();

  private slots:
    void capabilitiesReplyFinished();

  protected:
    QString errorMessageWithReason( const QString &reason ) override;
    int defaultExpirationInSec() override;

  private:
    QgsWfsCapabilities mCaps;

    QgsDataProvider::ProviderOptions mOptions;

    ApplicationLevelError mAppLevelError = ApplicationLevelError::NoError;

    //! Takes <Operations> element and updates the capabilities
    void parseSupportedOperations( const QDomElement &operationsElem, bool &insertCap, bool &updateCap, bool &deleteCap );

    void parseFilterCapabilities( const QDomElement &filterCapabilitiesElem );

    static QString NormalizeSRSName( const QString &crsName );
};

#endif // QGSWFSCAPABILITIES_H
