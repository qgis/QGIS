/***************************************************************************
  qgstiledmeshprovidermetadata.h
  --------------------------------------
  Date                 : June 2023
  Copyright            : (C) 2023 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSTILEDMESHPROVIDERMETADATA_H
#define QGSTILEDMESHPROVIDERMETADATA_H


#include "qgsprovidermetadata.h"

///@cond PRIVATE
#define SIP_NO_FILE

/**
 * This metadata class is responsible for generic tiled mesh handling.
 *
 * Specific tiled mesh data provider classes will also implement their own
 * QgsProviderMetadata.
 */
class QgsTiledMeshProviderMetadata : public QgsProviderMetadata
{
    Q_OBJECT
  public:
    QgsTiledMeshProviderMetadata();
    QIcon icon() const override;
    QList< QgsDataItemProvider * > dataItemProviders() const override;

    // handling of stored connections

    QMap<QString, QgsAbstractProviderConnection *> connections( bool cached ) override;
    QgsAbstractProviderConnection *createConnection( const QString &name ) override;
    void deleteConnection( const QString &name ) override;
    void saveConnection( const QgsAbstractProviderConnection *connection, const QString &name ) override;

    ProviderCapabilities providerCapabilities() const override;
};

///@endcond

#endif // QGSTILEDMESHPROVIDERMETADATA_H
