/***************************************************************************
    qgsowsprovider.h  -  OWS meta provider for WMS,WFS,WCS in browser
                         -------------------
    begin                : 4/2012
    copyright            : (C) 2012 by Radim Blazek
    email                : radim dot blazek at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSOWSPROVIDER_H
#define QGSOWSPROVIDER_H

#include "qgsdataprovider.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsdataitem.h"
#include "qgsrectangle.h"

#include <QString>

#include "qgsprovidermetadata.h"

/**

  \brief Data provider for OWS layers.

  This provider implements the interface defined in the QgsDataProvider class
  to provide access to spatial data residing in a OWS layers.

*/
class QgsOwsProvider : public QgsDataProvider
{
    Q_OBJECT

  public:

    /**
     * Constructor for the provider.
     *
     * \param   uri   HTTP URL of the Web Server.  If needed a proxy will be used
     *                otherwise we contact the host directly.
     * \param options generic data provider options
     *
     */
    explicit QgsOwsProvider( const QString &uri, const QgsDataProvider::ProviderOptions &providerOptions );

    /* Pure virtuals */

    QString name() const override;

    QString description() const override;

    QgsCoordinateReferenceSystem crs() const override { return QgsCoordinateReferenceSystem(); }

    QgsRectangle extent() const override { return QgsRectangle(); }

    bool isValid() const override { return false; }
};

class QgsOwsProviderMetadata: public QgsProviderMetadata
{
  public:
    QgsOwsProviderMetadata();
    QgsOwsProvider *createProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options ) override;
    QList< QgsDataItemProvider * > dataItemProviders() const override;
};

#endif // QGSOWSPROVIDER_H
