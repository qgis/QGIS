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
#include <QStringList>

#include <mdal.h>

#include "qgscoordinatereferencesystem.h"
#include "qgsmeshdataprovider.h"

class QMutex;
class QgsCoordinateTransform;
class QgsCoordinateReferenceSystem;

class QgsMdalDatasetIndex
{
  public:
    QgsMdalDatasetIndex( int mdalIndex, int uriIndex );
    QgsMdalDatasetIndex( int mdalIndex, const QString &persistFile );

    int mdalGroupIndex() const {return mMdalGroupIndex;}
    int uriIndex() const {return mUriIndex;}
    QString persistFile() const {return mPersisitFile;}

    void invalidate();
    void validate( int mdalGroupIndex );

  private:
    int mMdalGroupIndex = -1; //index of the dataset for MDAL
    int mUriIndex = -1; //index of the extra dataset groups uri
    QString mPersisitFile = "";
};


class QgsMdalProviderDatasetProxy
{
  public:

    void addMeshDatasetGroups( int mdalIndex );
    void addExtraDatasetGroups( int firstMdalIndex, int groupsCount, int uriIndex );
    void addPersistDatasetGroups( int mdalIndex, QString path );

    int mdalGroupIndex( int qgisIndex ) const;
    int dataSetGroupsCount() const;

    void invalidateIndexes( int uriIndex );
    void invalidateIndexes( QString persistFile );
    void validateIndexes( int uriIndex, int firstIndexMdal, int count );
    void validateIndexes( QString persistFile, int mdalIndex );

    void writeToXml( QDomDocument &document, QDomElement &parent ) const;
    void readFromXml( const QDomElement &elementTable );

    QStringList persistDatasetGroupFilesList() const
    {
      QStringList list;
      for ( const auto &mdi : mdalGroupIndexes )
      {
        QString file = mdi.persistFile();
        if ( !file.isEmpty() )
          list.append( mdi.persistFile() );
      }

      return list;
    }

  private:
    QList<QgsMdalDatasetIndex> mdalGroupIndexes;
};


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
    QgsMdalProvider( const QString &uri, const QgsDataProvider::ProviderOptions &providerOptions );
    ~QgsMdalProvider() override;

    bool isValid() const override;
    QString name() const override;
    QString description() const override;
    QgsCoordinateReferenceSystem crs() const override;

    int vertexCount() const override;
    int faceCount() const override;
    void populateMesh( QgsMesh *mesh ) const override;

    bool addDataset( const QString &uri ) override;
    void addUriDataset( const QString &uri ) override;
    QStringList extraDatasets() const override;

    int datasetGroupCount() const override;
    int datasetCount( int groupIndex ) const override;

    QgsMeshDatasetGroupMetadata datasetGroupMetadata( int groupIndex ) const override;
    QgsMeshDatasetMetadata datasetMetadata( QgsMeshDatasetIndex index ) const override;
    QgsMeshDatasetValue datasetValue( QgsMeshDatasetIndex index, int valueIndex ) const override;
    QgsMeshDataBlock datasetValues( QgsMeshDatasetIndex index, int valueIndex, int count ) const override;
    bool isFaceActive( QgsMeshDatasetIndex index, int faceIndex ) const override;
    QgsMeshDataBlock areFacesActive( QgsMeshDatasetIndex index, int faceIndex, int count ) const override;
    QgsRectangle extent() const override;

    bool persistDatasetGroup( const QString &path,
                              const QgsMeshDatasetGroupMetadata &meta,
                              const QVector<QgsMeshDataBlock> &datasetValues,
                              const QVector<QgsMeshDataBlock> &datasetActive,
                              const QVector<double> &times
                            ) override;

    void reloadData() override;

    /**
     * Returns file filters for meshes and datasets to be used in Open File Dialogs
     * \param fileMeshFiltersString file mesh filters
     * \param fileMeshDatasetFiltersString file mesh datasets filters
     *
     * \see fileMeshExtensions()
     *
     * \since QGIS 3.6
     */
    static void fileMeshFilters( QString &fileMeshFiltersString, QString &fileMeshDatasetFiltersString );

    /**
     * Returns file extensions for meshes and datasets
     * \param fileMeshExtensions file mesh extensions
     * \param fileMeshDatasetExtensions file mesh datasets extensions
     *
     * \see fileMeshFilters()
     *
     * \since QGIS 3.6
     */
    static void fileMeshExtensions( QStringList &fileMeshExtensions, QStringList &fileMeshDatasetExtensions );

    QDomElement writeProxyToXml( QDomDocument &document ) const override;
    void readProxyFromXml( const QDomNode &layer_node ) override;

    void reloadExtraDatasetUris( ) override;
    void reloadPersistDatasetGroups( ) override;

  private:
    QVector<QgsMeshVertex> vertices( ) const;
    QVector<QgsMeshFace> faces( ) const;
    void loadData();

    MeshH mMeshH;
    QStringList mExtraDatasetUris;
    QgsCoordinateReferenceSystem mCrs;

    QgsMdalProviderDatasetProxy mDatasetProxy;
    int datasetGroupMDALCount( ) const;
};

#endif //QGSMDALPROVIDER_H
