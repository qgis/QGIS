/***************************************************************************
  qgsvectortileprovidermetadata.h
  --------------------------------------
  Date                 : March 2020
  Copyright            : (C) 2020 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSVECTORTILEPROVIDERMETADATA_H
#define QGSVECTORTILEPROVIDERMETADATA_H


#include "qgsprovidermetadata.h"

///@cond PRIVATE
#define SIP_NO_FILE

/**
 * This metadata class does not support creation of provider instances, because
 * vector tile layer currently does not have a concept of data providers. This class
 * is only used to create data item provider (for browser integration).
 */
class QgsVectorTileProviderMetadata : public QgsProviderMetadata
{
  public:
    QgsVectorTileProviderMetadata();
    QList< QgsDataItemProvider * > dataItemProviders() const override;

    // handling of stored connections

    QMap<QString, QgsAbstractProviderConnection *> connections( bool cached ) override;
    QgsAbstractProviderConnection *createConnection( const QString &name ) override;
    void deleteConnection( const QString &name ) override;
    void saveConnection( const QgsAbstractProviderConnection *connection, const QString &name ) override;

};

///@endcond

#endif // QGSVECTORTILEPROVIDERMETADATA_H
