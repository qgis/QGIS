/***************************************************************************
                         qgsmeshmemorydataprovider.h
                         ---------------------------
    begin                : April 2018
    copyright            : (C) 2018 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMESHMEMORYDATAPROVIDER_H
#define QGSMESHMEMORYDATAPROVIDER_H

#define SIP_NO_FILE

///@cond PRIVATE

#include <QString>
#include <memory>

#include "qgis_core.h"
#include "qgis.h"
#include "qgsmeshdataprovider.h"
#include "qgsrectangle.h"

/**
 * \ingroup core
 * \brief Provides data stored in-memory for QgsMeshLayer. Useful for plugins or tests.
 * \since QGIS 3.2
 */
class CORE_EXPORT QgsMeshMemoryDataProvider final: public QgsMeshDataProvider
{
    Q_OBJECT

  public:

    /**
     * Construct a mesh in-memory data provider from data string
     *
     * Data string contains simple definition of vertices and faces or edges
     *
     * Each entry is separated by "\n" sign and section deliminer "---"
     * First section defines vertices (x and y coordinate separated by comma)
     * Second section defines face list (vertex indexes, numbered from 0. A face has 3 or move vertices)
     * or defines edge list (vertex indexes, numbered from 0. An edge has 2 vertices)
     *
     * It is not possible to define mesh with both faces and edges
     *
     * For example (mesh with faces and vertices):
     *
     *  \code
     *    QString uri(
     *      "1.0, 2.0 \n" \
     *      "2.0, 2.0 \n" \
     *      "3.0, 2.0 \n" \
     *      "2.0, 3.0 \n" \
     *      "1.0, 3.0 \n" \
     *      "--- \n"
     *      "0, 1, 3, 4 \n" \
     *      "1, 2, 3 \n"
     *    );
     * \endcode
     *
     *  For example (mesh with edges and vertices):
     *
     *  \code
     *    QString uri(
     *      "1.0, 2.0 \n" \
     *      "2.0, 2.0 \n" \
     *      "3.0, 2.0 \n" \
     *      "2.0, 3.0 \n" \
     *      "1.0, 3.0 \n" \
     *      "---\n"
     *      "0, 1 \n" \
     *      "1, 2 \n"
     *    );
     * \endcode
     */
    QgsMeshMemoryDataProvider( const QString &uri, const QgsDataProvider::ProviderOptions &providerOptions,
                               QgsDataProvider::ReadFlags flags = QgsDataProvider::ReadFlags() );

    bool isValid() const override;
    QString name() const override;
    QString description() const override;
    QgsCoordinateReferenceSystem crs() const override;

    int vertexCount() const override;
    int faceCount() const override;
    int edgeCount() const override;
    void populateMesh( QgsMesh *mesh ) const override;
    QgsRectangle extent() const override;

    /**
     * Adds dataset to a mesh in-memory data provider from data string
     *
     * Data string contains simple definition of datasets
     * Each entry is separated by "\n" sign and section deliminer "---"
     * First section defines the dataset group: vertex/edge/face vector/scalar Name
     * Second section defines the group metadata: key: value pairs
     * Third section defines the datasets (timesteps). First line is time,
     * other lines are values (one value on line). For vectors separated by comma
     *
     * For example:
     *
     *  \code
     *    QString uri(
     *      "vertex vector MyVertexVectorDataset\n" \
     *      "---"
     *      "description: My great dataset \n" \
     *      "reference_time: Midnight  \n" \
     *      "---"
     *      "0 \n"
     *      "3, 2 \n" \
     *      "1, -2 \n"
     *      "---"
     *      "1 \n"
     *      "2, 2 \n" \
     *      "2, -2 \n"
     *    );
     * \endcode
     */
    bool addDataset( const QString &uri ) override;
    QStringList extraDatasets() const override;
    int datasetGroupCount() const override;
    int datasetCount( int groupIndex ) const override;

    QgsMeshDatasetGroupMetadata datasetGroupMetadata( int groupIndex ) const override;
    QgsMeshDatasetMetadata datasetMetadata( QgsMeshDatasetIndex index ) const override;
    QgsMeshDatasetValue datasetValue( QgsMeshDatasetIndex index, int valueIndex ) const override;
    QgsMeshDataBlock datasetValues( QgsMeshDatasetIndex index, int valueIndex, int count ) const override;
    QgsMesh3dDataBlock dataset3dValues( QgsMeshDatasetIndex index, int faceIndex, int count ) const override;

    bool isFaceActive( QgsMeshDatasetIndex index, int faceIndex ) const override;
    QgsMeshDataBlock areFacesActive( QgsMeshDatasetIndex index, int faceIndex, int count ) const override;
    bool persistDatasetGroup( const QString &outputFilePath,
                              const QString &outputDriver,
                              const QgsMeshDatasetGroupMetadata &meta,
                              const QVector<QgsMeshDataBlock> &datasetValues,
                              const QVector<QgsMeshDataBlock> &datasetActive,
                              const QVector<double> &times
                            ) override;

    virtual bool persistDatasetGroup( const QString &outputFilePath,
                                      const QString &outputDriver,
                                      QgsMeshDatasetSourceInterface *source,
                                      int datasetGroupIndex
                                    ) override;

    bool saveMeshFrame( const QgsMesh & ) override {return false;}

    void close() override;

    //! Returns the memory provider key
    static QString providerKey();
    //! Returns the memory provider description
    static QString providerDescription();
    //! Provider factory
    static QgsMeshMemoryDataProvider *createProvider( const QString &uri,
        const QgsDataProvider::ProviderOptions &providerOptions,
        QgsDataProvider::ReadFlags flags = QgsDataProvider::ReadFlags() );

  private:
    QgsRectangle calculateExtent( ) const;

    bool splitMeshSections( const QString &uri );
    bool addMeshVertices( const QString &def );
    bool addMeshFacesOrEdges( const QString &def );

    bool splitDatasetSections( const QString &uri, QgsMeshMemoryDatasetGroup &datasetGroup );
    bool setDatasetGroupType( const QString &uri, QgsMeshMemoryDatasetGroup &datasetGroup );
    bool addDatasetGroupMetadata( const QString &def, QgsMeshMemoryDatasetGroup &datasetGroup );
    bool addDatasetValues( const QString &def, std::shared_ptr<QgsMeshMemoryDataset> &dataset, bool isScalar );
    bool checkDatasetValidity( std::shared_ptr<QgsMeshMemoryDataset> &dataset, QgsMeshDatasetGroupMetadata::DataType dataType );
    bool checkVertexId( int vertex_id );

    void addGroupToTemporalCapabilities( int groupIndex, const QgsMeshMemoryDatasetGroup &group );

    QVector<QgsMeshVertex> mVertices;
    QVector<QgsMeshFace> mFaces;
    QVector<QgsMeshEdge> mEdges;
    QVector<QgsMeshMemoryDatasetGroup> mDatasetGroups;

    bool mIsValid = false;
    QStringList mExtraDatasetUris;
};

///@endcond

#endif // QGSMESHMEMORYDATAPROVIDER_H
