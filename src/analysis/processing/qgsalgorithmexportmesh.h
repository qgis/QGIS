/***************************************************************************
                         qgsalgorithmexportmesh.h
                         ---------------------------
    begin                : October 2020
    copyright            : (C) 2020 by Vincent Cloarec
    email                : vcloarec at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSALGORITHMEXPORTMESH_H
#define QGSALGORITHMEXPORTMESH_H

#define SIP_NO_FILE

#include "qgsprocessingalgorithm.h"
#include "qgsmeshdataset.h"
#include "qgsmeshdataprovider.h"
#include "qgstriangularmesh.h"
#include "qgsmeshrenderersettings.h"

///@cond PRIVATE

struct DataGroup
{
  QgsMeshDatasetGroupMetadata metadata;
  QgsMeshDataBlock datasetValues;
  QgsMeshDataBlock activeFaces;
  QgsMesh3dDataBlock dataset3dStakedValue; //will be filled only if data are 3d stacked
};

class QgsExportMeshOnElement : public QgsProcessingAlgorithm
{

  public:
    QString group() const override;
    QString groupId() const override;

  protected:
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QVariantMap processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QgsMesh mNativeMesh;

  private:

    virtual QSet<int> supportedDataType() const = 0;
    virtual QgsProcessing::SourceType sinkType() const = 0;
    virtual QgsWkbTypes::Type sinkGeometryType() const = 0;
    virtual QgsGeometry meshElement( int index ) const = 0;
    virtual QgsMesh::ElementType meshElementType() const = 0;

    QList<DataGroup> mDataPerGroup;
    QgsCoordinateTransform mTransform;
    int mExportVectorOption = 2;
    int mElementCount = 0;
};

class QgsExportMeshVerticesAlgorithm : public QgsExportMeshOnElement
{
  public:
    QString shortHelpString() const override;
    QString shortDescription() const override;
    QString name() const override;
    QString displayName() const override;

  protected:
    QgsProcessingAlgorithm *createInstance() const override;

  private:
    QgsWkbTypes::Type sinkGeometryType() const override {return QgsWkbTypes::PointZ;}
    QSet<int> supportedDataType() const override
    {
      return QSet<int>( {QgsMeshDatasetGroupMetadata::DataOnVertices} );
    }
    QgsProcessing::SourceType sinkType() const override {return QgsProcessing::TypeVectorPoint;}
    QgsGeometry meshElement( int index ) const override;
    QgsMesh::ElementType meshElementType()const override {return QgsMesh::Vertex;}
};

class QgsExportMeshFacesAlgorithm : public QgsExportMeshOnElement
{
  public:
    QString shortHelpString() const override;
    QString shortDescription() const override;
    QString name() const override;
    QString displayName() const override;

  protected:
    QgsProcessingAlgorithm *createInstance() const override;

  private:
    QgsWkbTypes::Type sinkGeometryType() const override {return QgsWkbTypes::PolygonZ;}
    QSet<int> supportedDataType() const override
    {
      return QSet<int>( {QgsMeshDatasetGroupMetadata::DataOnFaces} );
    }
    QgsProcessing::SourceType sinkType() const override {return QgsProcessing::TypeVectorPolygon;}
    QgsGeometry meshElement( int index ) const override;
    QgsMesh::ElementType meshElementType()const override {return QgsMesh::Face;}
};

class QgsExportMeshEdgesAlgorithm : public QgsExportMeshOnElement
{
  public:
    QString shortHelpString() const override;
    QString shortDescription() const override;
    QString name() const override;
    QString displayName() const override;

  protected:
    QgsProcessingAlgorithm *createInstance() const override;

  private:
    QgsWkbTypes::Type sinkGeometryType() const override {return QgsWkbTypes::LineStringZ;}
    QSet<int> supportedDataType() const override
    {
      return QSet<int>( {QgsMeshDatasetGroupMetadata::DataOnEdges} );
    }
    QgsProcessing::SourceType sinkType() const override {return QgsProcessing::TypeVectorLine;}
    QgsGeometry meshElement( int index ) const override;
    QgsMesh::ElementType meshElementType()const override {return QgsMesh::Edge;}
};


class QgsExportMeshOnGridAlgorithm : public QgsProcessingAlgorithm
{

  public:
    QString name() const override;
    QString displayName() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QString shortDescription() const override;

  protected:
    QgsProcessingAlgorithm *createInstance() const override;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QVariantMap processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:

    QSet<int> supportedDataType();

    QgsTriangularMesh mTriangularMesh;

    QList<DataGroup> mDataPerGroup;
    QgsCoordinateTransform mTransform;
    int mExportVectorOption = 2;
    QgsMeshRendererSettings mLayerRendererSettings;
};

class QgsMeshRasterizeAlgorithm : public QgsProcessingAlgorithm
{

  public:
    QString name() const override;
    QString displayName() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QString shortDescription() const override;

  protected:
    QgsProcessingAlgorithm *createInstance() const override;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QVariantMap processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:

    QSet<int> supportedDataType();

    QgsTriangularMesh mTriangularMesh;

    QList<DataGroup> mDataPerGroup;
    QgsCoordinateTransform mTransform;
    QgsMeshRendererSettings mLayerRendererSettings;
};

class QgsMeshContoursAlgorithm : public QgsProcessingAlgorithm
{

  public:
    QString name() const override;
    QString displayName() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QString shortDescription() const override;

  protected:
    QgsProcessingAlgorithm *createInstance() const override;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QVariantMap processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:

    QSet<int> supportedDataType()
    {
      return QSet<int>(
      {
        QgsMeshDatasetGroupMetadata::DataOnVertices,
        QgsMeshDatasetGroupMetadata::DataOnFaces,
        QgsMeshDatasetGroupMetadata::DataOnVolumes} );
    }

    QgsTriangularMesh mTriangularMesh;
    QgsMesh mNativeMesh;
    QVector<double> mLevels;

    QList<DataGroup> mDataPerGroup;
    QgsCoordinateTransform mTransform;
    QgsMeshRendererSettings mLayerRendererSettings;
    QString mDateTimeString;

};

class QgsMeshExportCrossSection : public QgsProcessingAlgorithm
{

  public:
    QString name() const override;
    QString displayName() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QString shortDescription() const override;

  protected:
    QgsProcessingAlgorithm *createInstance() const override;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QVariantMap processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:

    QSet<int> supportedDataType()
    {
      return QSet<int>(
      {
        QgsMeshDatasetGroupMetadata::DataOnVertices,
        QgsMeshDatasetGroupMetadata::DataOnFaces,
        QgsMeshDatasetGroupMetadata::DataOnVolumes} );
    }

    QgsTriangularMesh mTriangularMesh;

    QList<DataGroup> mDataPerGroup;
    QgsCoordinateReferenceSystem mMeshLayerCrs;
    QgsMeshRendererSettings mLayerRendererSettings;

};

class QgsMeshExportTimeSeries : public QgsProcessingAlgorithm
{

  public:
    QString name() const override;
    QString displayName() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QString shortDescription() const override;

  protected:
    QgsProcessingAlgorithm *createInstance() const override;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QVariantMap processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:

    QSet<int> supportedDataType()
    {
      return QSet<int>(
      {
        QgsMeshDatasetGroupMetadata::DataOnVertices,
        QgsMeshDatasetGroupMetadata::DataOnFaces,
        QgsMeshDatasetGroupMetadata::DataOnVolumes} );
    }

    QgsTriangularMesh mTriangularMesh;

    QgsCoordinateReferenceSystem mMeshLayerCrs;
    QgsMeshRendererSettings mLayerRendererSettings;

    QList<int> mGroupIndexes;
    QList<DataGroup> mDatasets;
    QList<qint64> mRelativeTimeSteps;
    QStringList mTimeStepString;
    QMap<qint64, QMap<int, int>> mRelativeTimeToData;
    QMap<int, QgsMeshDatasetGroupMetadata> mGroupsMetadata;

};


///@endcond PRIVATE

#endif // QGSALGORITHMEXPORTMESH_H
