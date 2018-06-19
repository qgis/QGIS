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

#include "qgis_core.h"
#include "qgis.h"
#include "qgsmeshdataprovider.h"
#include "qgsrectangle.h"

struct QgsMeshMemoryDataset
{
  QMap<QString, QString> metadata;
  QVector<QgsMeshDatasetValue> values;
  bool isScalar = true;
  bool isOnVertices = true;
  bool valid = false;
};

/**
 * \ingroup core
 * Provides data stored in-memory for QgsMeshLayer. Useful for plugins or tests.
 * \since QGIS 3.2
 */
class QgsMeshMemoryDataProvider: public QgsMeshDataProvider
{
    Q_OBJECT

  public:

    /**
     * Construct a mesh in-memory data provider from data string
     *
     * Data string constains simple definition of vertices and faces
     * Each entry is separated by "\n" sign and section deliminer "---"
     * vertex is x and y coordinate separated by comma
     * face is list of vertex indexes, numbered from 0
     * For example:
     *
     *  \code
     *    QString uri(
     *      "1.0, 2.0 \n" \
     *      "2.0, 2.0 \n" \
     *      "3.0, 2.0 \n" \
     *      "2.0, 3.0 \n" \
     *      "1.0, 3.0 \n" \
     *      "---"
     *      "0, 1, 3, 4 \n" \
     *      "1, 2, 3 \n"
     *    );
     * \endcode
     */
    QgsMeshMemoryDataProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options );

    bool isValid() const override;
    QString name() const override;
    QString description() const override;
    QgsCoordinateReferenceSystem crs() const override;

    int vertexCount() const override;
    int faceCount() const override;
    QgsMeshVertex vertex( int index ) const override;
    QgsMeshFace face( int index ) const override;


    /**
     * Adds dataset to a mesh in-memory data provider from data string
     *
     * Data string constains simple definition of datasets
     * Each entry is separated by "\n" sign and section deliminer "---"
     * First section defines the type of dataset: Vertex/Face Vector/Scalar
     * Second section defines the metadata: key: value pairs
     * Third section defines the values. One value on line. For vectors separated by comma
     *
     * For example:
     *
     *  \code
     *    QString uri(
     *      "Vertex Vector \n" \
     *      "---"
     *      "name: MyVertexVectorDataset \n" \
     *      "time: 0 \n" \
     *      "---"
     *      "3, 2 \n" \
     *      "1, -2 \n"
     *    );
     * \endcode
     */
    bool addDataset( const QString &uri ) override;
    int datasetCount() const override;

    QgsMeshDatasetMetadata datasetMetadata( int datasetIndex ) const override;
    QgsMeshDatasetValue datasetValue( int datasetIndex, int valueIndex ) const override;

    //! Returns the memory provider key
    static QString providerKey();
    //! Returns the memory provider description
    static QString providerDescription();
    //! Provider factory
    static QgsMeshMemoryDataProvider *createProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options );
  private:
    bool splitMeshSections( const QString &uri );
    bool addMeshVertices( const QString &def );
    bool addMeshFaces( const QString &def );

    bool splitDatasetSections( const QString &uri, QgsMeshMemoryDataset &dataset );
    bool setDatasetType( const QString &uri, QgsMeshMemoryDataset &dataset );
    bool addDatasetMetadata( const QString &def, QgsMeshMemoryDataset &dataset );
    bool addDatasetValues( const QString &def, QgsMeshMemoryDataset &dataset );
    bool checkDatasetValidity( QgsMeshMemoryDataset &dataset );

    QVector<QgsMeshVertex> mVertices;
    QVector<QgsMeshFace> mFaces;
    QVector<QgsMeshMemoryDataset> mDatasets;

    bool mIsValid = false;
};

///@endcond

#endif // QGSMESHMEMORYDATAPROVIDER_H
