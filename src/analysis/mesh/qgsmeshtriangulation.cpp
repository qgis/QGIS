/***************************************************************************
                          qgsmeshtriangulation.cpp
                          -----------------
    begin                : August 9th, 2020
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

#include "qgsmeshtriangulation.h"
#include "qgsdualedgetriangulation.h"
#include "qgsvectorlayer.h"
#include "qgscoordinatetransformcontext.h"
#include "qgscurve.h"
#include "qgscurvepolygon.h"
#include "qgsmultisurface.h"
#include "qgsmulticurve.h"
#include "qgsfeedback.h"

QgsMeshTriangulation::QgsMeshTriangulation(): QObject()
{
  mTriangulation.reset( new QgsDualEdgeTriangulation() );
}


QgsMeshTriangulation::~QgsMeshTriangulation() = default;


bool QgsMeshTriangulation::addVertices( QgsVectorLayer *vectorLayer,
                                        int valueAttribute,
                                        const QgsCoordinateTransformContext &transformContext,
                                        QgsFeedback *feedback )
{
  if ( !vectorLayer )
    return false;

  if ( !vectorLayer->isValid() )
    return false;

  if ( feedback )
    feedback->setProgress( 0 );

  QgsCoordinateTransform transform( vectorLayer->crs(), mCrs, transformContext );

  QgsFeatureIterator fIt = vectorLayer->getFeatures();
  QgsFeature feat;
  bool isZValueGeom = valueAttribute < 0;
  int i = 0;
  int featureCount = vectorLayer->featureCount();
  while ( fIt.nextFeature( feat ) )
  {
    if ( feedback )
    {
      if ( feedback->isCanceled() )
        break;

      feedback->setProgress( 100 * i / featureCount );
      i++;
    }

    QgsGeometry geom = feat.geometry();
    geom.transform( transform, QgsCoordinateTransform::ForwardTransform, true );
    QgsAbstractGeometry::vertex_iterator vit = geom.vertices_begin();

    double value = 0;
    if ( !isZValueGeom )
      value = feat.attribute( valueAttribute ).toDouble();

    while ( vit != geom.vertices_end() )
    {
      if ( feedback && feedback->isCanceled() )
        break;
      if ( isZValueGeom )
        mTriangulation->addPoint( *vit );
      else
      {
        ;
        mTriangulation->addPoint( QgsPoint( QgsWkbTypes::PointZ, ( *vit ).x(), ( *vit ).y(), value ) );
      }
      ++vit;
    }
  }

  return true;
}

bool QgsMeshTriangulation::addBreakLines( QgsVectorLayer *vectorLayer, int valueAttribute, const QgsCoordinateTransformContext &transformContext, QgsFeedback *feedback )
{
  if ( !vectorLayer )
    return false;

  QgsCoordinateTransform transform( vectorLayer->crs(), mCrs, transformContext );

  QgsWkbTypes::GeometryType geomType = vectorLayer->geometryType();

  switch ( geomType )
  {
    case QgsWkbTypes::PointGeometry:
      return addVertices( vectorLayer, valueAttribute, transformContext, feedback );
      break;
    case QgsWkbTypes::LineGeometry:
    case QgsWkbTypes::PolygonGeometry:
    {
      QgsFeatureIterator fIt = vectorLayer->getFeatures();
      QgsFeature feat;
      while ( fIt.nextFeature( feat ) )
      {
        addBreakLinesFromFeature( feat, valueAttribute, transform, feedback );
      }
      return true;
    }
    break;
    default:
      return false;
  }

  return false;
}

QgsMesh QgsMeshTriangulation::triangulatedMesh() const
{
  return mTriangulation->triangulationToMesh();
}

void QgsMeshTriangulation::setCrs( const QgsCoordinateReferenceSystem &crs )
{
  mCrs = crs;
}

void QgsMeshTriangulation::addBreakLinesFromFeature( const QgsFeature &feature, int valueAttribute, const QgsCoordinateTransform &transform, QgsFeedback *feedback )
{
  double valueOnVertex = 0;
  if ( valueAttribute >= 0 )
    valueOnVertex = feature.attribute( valueAttribute ).toDouble();

  if ( feedback )
    feedback->setProgress( 0 );

  //from QgsTinInterpolator::insertData()
  std::vector<const QgsCurve *> curves;
  QgsGeometry geom = feature.geometry();
  geom.transform( transform, QgsCoordinateTransform::ForwardTransform, true );
  if ( QgsWkbTypes::geometryType( geom.wkbType() ) == QgsWkbTypes::PolygonGeometry )
  {
    std::vector< const QgsCurvePolygon * > polygons;
    if ( geom.isMultipart() )
    {
      const QgsMultiSurface *ms = qgsgeometry_cast< const QgsMultiSurface * >( geom.constGet() );
      for ( int i = 0; i < ms->numGeometries(); ++i )
      {
        polygons.emplace_back( qgsgeometry_cast< const QgsCurvePolygon * >( ms->geometryN( i ) ) );
      }
    }
    else
    {
      polygons.emplace_back( qgsgeometry_cast< const QgsCurvePolygon * >( geom.constGet() ) );
    }

    for ( const QgsCurvePolygon *polygon : polygons )
    {
      if ( feedback && feedback->isCanceled() )
        break;
      if ( !polygon )
        continue;

      if ( polygon->exteriorRing() )
        curves.emplace_back( polygon->exteriorRing() );

      for ( int i = 0; i < polygon->numInteriorRings(); ++i )
      {
        if ( feedback && feedback->isCanceled() )
          break;
        curves.emplace_back( polygon->interiorRing( i ) );
      }
    }
  }
  else
  {
    if ( geom.isMultipart() )
    {
      const QgsMultiCurve *mc = qgsgeometry_cast< const QgsMultiCurve * >( geom.constGet() );
      for ( int i = 0; i < mc->numGeometries(); ++i )
      {
        if ( feedback && feedback->isCanceled() )
          break;
        curves.emplace_back( qgsgeometry_cast< const QgsCurve * >( mc->geometryN( i ) ) );
      }
    }
    else
    {
      curves.emplace_back( qgsgeometry_cast< const QgsCurve * >( geom.constGet() ) );
    }
  }

  int i = 0;
  for ( const QgsCurve *curve : curves )
  {
    if ( !curve )
      continue;

    if ( feedback )
    {
      if ( feedback->isCanceled() )
        break;

      feedback->setProgress( 100 * i / curves.size() );
      i++;
    }

    QgsPointSequence linePoints;
    curve->points( linePoints );
    if ( valueAttribute >= 0 )
      for ( QgsPoint &point : linePoints )
        point.setZ( valueOnVertex );

    mTriangulation->addLine( linePoints, QgsInterpolator::SourceBreakLines );
  }
}

QgsMeshZValueDatasetGroup::QgsMeshZValueDatasetGroup( const QString &datasetGroupName, const QgsMesh &mesh ):
  QgsMeshDatasetGroup( datasetGroupName, QgsMeshDatasetGroupMetadata::DataOnVertices )
{
  mDataset.reset( new QgsMeshZValueDataset( mesh ) );
}

void QgsMeshZValueDatasetGroup::initialize()
{
  calculateStatistic();
}

QgsMeshDatasetMetadata QgsMeshZValueDatasetGroup::datasetMetadata( int datasetIndex ) const
{
  if ( datasetIndex != 0 )
    return QgsMeshDatasetMetadata();

  return mDataset->metadata();
}

int QgsMeshZValueDatasetGroup::datasetCount() const {return 1;}

QgsMeshDataset *QgsMeshZValueDatasetGroup::dataset( int index ) const
{
  if ( index != 0 )
    return nullptr;

  return mDataset.get();
}

QDomElement QgsMeshZValueDatasetGroup::writeXml( QDomDocument &doc, const QgsReadWriteContext &context ) const
{
  Q_UNUSED( doc );
  Q_UNUSED( context );
  return QDomElement();
}

QgsMeshZValueDataset::QgsMeshZValueDataset( const QgsMesh &mesh ): mMesh( mesh )
{
  for ( const QgsMeshVertex vertex : mMesh.vertices )
  {
    if ( vertex.z() < mZMinimum )
      mZMinimum = vertex.z();
    if ( vertex.z() > mZMaximum )
      mZMaximum = vertex.z();
  }
}

QgsMeshDatasetValue QgsMeshZValueDataset::datasetValue( int valueIndex ) const
{
  if ( valueIndex < 0 || valueIndex >= mMesh.vertexCount() )
    return QgsMeshDatasetValue();

  return QgsMeshDatasetValue( mMesh.vertex( valueIndex ).z() );
}

QgsMeshDataBlock QgsMeshZValueDataset::datasetValues( bool isScalar, int valueIndex, int count ) const
{
  Q_UNUSED( isScalar )
  QgsMeshDataBlock ret( QgsMeshDataBlock::ScalarDouble, count );
  QVector<double> zValues( count );
  for ( int i = valueIndex; i < valueIndex + count; ++i )
    zValues[i - valueIndex] = mMesh.vertex( i ).z();
  ret.setValues( zValues );
  return ret;
}

QgsMeshDataBlock QgsMeshZValueDataset::areFacesActive( int faceIndex, int count ) const
{
  Q_UNUSED( faceIndex );
  Q_UNUSED( count );
  QgsMeshDataBlock ret( QgsMeshDataBlock::ActiveFlagInteger, count );
  ret.setValid( true );
  return ret;
}

bool QgsMeshZValueDataset::isActive( int faceIndex ) const
{
  return ( faceIndex > 0 && faceIndex < mMesh.faceCount() );
}

QgsMeshDatasetMetadata QgsMeshZValueDataset::metadata() const
{
  return QgsMeshDatasetMetadata( 0, true, mZMinimum, mZMaximum, 0 );
}

int QgsMeshZValueDataset::valuesCount() const
{
  return mMesh.vertexCount();
}
