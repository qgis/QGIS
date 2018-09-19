/***************************************************************************
    qgsmdalprovider.h
    -----------------
    begin                : April 2018
    copyright            : (C) 2018 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMDALPROVIDER_H
#define QGSMDALPROVIDER_H

#include <QString>
#include <QVector>

#include <mdal.h>

#include "qgscoordinatereferencesystem.h"
#include "qgsmeshdataprovider.h"

class QMutex;
class QgsCoordinateTransform;
class QgsCoordinateReferenceSystem;

/**
  \brief Data provider for MDAL layers.
*/
class QgsMdalProvider : public QgsMeshDataProvider
{
    Q_OBJECT

  public:

    /**
     * Constructor for the provider.
     *
     * \param uri file name
     * \param options generic provider options
     */
    QgsMdalProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options );
    ~QgsMdalProvider() override;

    bool isValid() const override;
    QString name() const override;
    QString description() const override;
    QgsCoordinateReferenceSystem crs() const override;

    int vertexCount() const override;
    int faceCount() const override;
    QgsMeshVertex vertex( int index ) const override;
    QgsMeshFace face( int index ) const override;

    bool addDataset( const QString &uri ) override;
    QStringList extraDatasets() const override;

    int datasetGroupCount() const override;
    int datasetCount( int groupIndex ) const override;

    QgsMeshDatasetGroupMetadata datasetGroupMetadata( int groupIndex ) const override;
    QgsMeshDatasetMetadata datasetMetadata( QgsMeshDatasetIndex index ) const override;
    QgsMeshDatasetValue datasetValue( QgsMeshDatasetIndex index, int valueIndex ) const override;
    bool isFaceActive( QgsMeshDatasetIndex index, int faceIndex ) const override;

  private:
    MeshH mMeshH;
    QStringList mExtraDatasetUris;
    QgsCoordinateReferenceSystem mCrs;
};

#endif //QGSMDALPROVIDER_H
