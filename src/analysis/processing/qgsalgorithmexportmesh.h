/***************************************************************************
                         qgsalgorithmtinmeshcreation.h
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

#ifndef QGSALGORITHMEXPORTMESHVERTICES_H
#define QGSALGORITHMEXPORTMESHVERTICES_H

#define SIP_NO_FILE

#include "qgsprocessingalgorithm.h"
#include "qgsmeshdataset.h"
#include "qgsmeshdataprovider.h"

///@cond PRIVATE

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

    virtual QList<QgsMeshDatasetGroupMetadata::DataType> supportedDataType() const = 0;
    virtual QgsProcessing::SourceType sinkType() const = 0;
    virtual QgsWkbTypes::Type sinkGeometryType() const = 0;
    virtual QgsGeometry meshElement( int index ) const = 0;
    virtual QgsMesh::ElementType meshElementType() const = 0;

    struct DataGroup
    {
      QgsMeshDatasetGroupMetadata metadata;
      QgsMeshDataBlock datasetValues;
    };

    QList<DataGroup> mDataPerGroup;
    QgsCoordinateTransform mTransform;
    int mExportVectorOption = 2;
    int mElementCount = 0;
};

class QgsExportMeshVerticesAlgorithm : public QgsExportMeshOnElement
{
  public:
    QString shortHelpString() const override;
    QString name() const override;
    QString displayName() const override;

  protected:
    QgsProcessingAlgorithm *createInstance() const override;

  private:
    QgsWkbTypes::Type sinkGeometryType() const override {return QgsWkbTypes::PointZ;}
    QList<QgsMeshDatasetGroupMetadata::DataType> supportedDataType() const override
    {
      return QList<QgsMeshDatasetGroupMetadata::DataType>( {QgsMeshDatasetGroupMetadata::DataOnVertices} );
    }
    QgsProcessing::SourceType sinkType() const override {return QgsProcessing::TypeVectorPoint;}
    QgsGeometry meshElement( int index ) const override;
    QgsMesh::ElementType meshElementType()const override {return QgsMesh::Vertex;}
};

class QgsExportMeshFacesAlgorithm : public QgsExportMeshOnElement
{
  public:
    QString shortHelpString() const override;
    QString name() const override;
    QString displayName() const override;

  protected:
    QgsProcessingAlgorithm *createInstance() const override;

  private:
    QgsWkbTypes::Type sinkGeometryType() const override {return QgsWkbTypes::PolygonZ;}
    QList<QgsMeshDatasetGroupMetadata::DataType> supportedDataType() const override
    {
      return QList<QgsMeshDatasetGroupMetadata::DataType>( {QgsMeshDatasetGroupMetadata::DataOnFaces} );
    }
    QgsProcessing::SourceType sinkType() const override {return QgsProcessing::TypeVectorPolygon;}
    QgsGeometry meshElement( int index ) const override;
    QgsMesh::ElementType meshElementType()const override {return QgsMesh::Face;}
};

class QgsExportMeshEdgesAlgorithm : public QgsExportMeshOnElement
{
  public:
    QString shortHelpString() const override;
    QString name() const override;
    QString displayName() const override;

  protected:
    QgsProcessingAlgorithm *createInstance() const override;

  private:
    QgsWkbTypes::Type sinkGeometryType() const override {return QgsWkbTypes::LineStringZ;}
    QList<QgsMeshDatasetGroupMetadata::DataType> supportedDataType() const override
    {
      return QList<QgsMeshDatasetGroupMetadata::DataType>( {QgsMeshDatasetGroupMetadata::DataOnEdges} );
    }
    QgsProcessing::SourceType sinkType() const override {return QgsProcessing::TypeVectorLine;}
    QgsGeometry meshElement( int index ) const override;
    QgsMesh::ElementType meshElementType()const override {return QgsMesh::Edge;}
};

///@endcond PRIVATE

#endif // QGSALGORITHMEXPORTMESHVERTICES_H
