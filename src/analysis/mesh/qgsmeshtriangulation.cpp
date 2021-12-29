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
#include "qgslogger.h"
#include "qgsmesheditor.h"

QgsMeshTriangulation::QgsMeshTriangulation(): QObject()
{
  mTriangulation.reset( new QgsDualEdgeTriangulation() );
}


QgsMeshTriangulation::~QgsMeshTriangulation() = default;

bool QgsMeshTriangulation::addVertices( QgsFeatureIterator &vertexFeatureIterator, int valueAttribute, const QgsCoordinateTransform &transform, QgsFeedback *feedback, long featureCount )
{
  if ( feedback )
    feedback->setProgress( 0 );

  QgsFeature feat;
  long i = 0;
  while ( vertexFeatureIterator.nextFeature( feat ) )
  {
    if ( feedback )
    {
      if ( feedback->isCanceled() )
        break;

      feedback->setProgress( 100 * i / featureCount );
      i++;
    }

    addVerticesFromFeature( feat, valueAttribute, transform, feedback );
  }

  return true;
}

bool QgsMeshTriangulation::addBreakLines( QgsFeatureIterator &lineFeatureIterator, int valueAttribute, const QgsCoordinateTransform &transform, QgsFeedback *feedback, long featureCount )
{
  if ( feedback )
    feedback->setProgress( 0 );

  QgsFeature feat;
  long i = 0;
  while ( lineFeatureIterator.nextFeature( feat ) )
  {
    if ( feedback )
    {
      if ( feedback->isCanceled() )
        break;

      feedback->setProgress( 100 * i / featureCount );
      i++;
    }

    QgsWkbTypes::GeometryType geomType = feat.geometry().type();
    switch ( geomType )
    {
      case QgsWkbTypes::PointGeometry:
        addVerticesFromFeature( feat, valueAttribute, transform, feedback );
        break;
      case QgsWkbTypes::LineGeometry:
      case QgsWkbTypes::PolygonGeometry:
        addBreakLinesFromFeature( feat, valueAttribute, transform, feedback );
        break;
      default:
        break;
    }
  }

  return true;
}

int QgsMeshTriangulation::addVertex( const QgsPoint &vertex )
{
  return mTriangulation->addPoint( vertex );
}

QgsMesh QgsMeshTriangulation::triangulatedMesh( QgsFeedback *feedback ) const
{
  return mTriangulation->triangulationToMesh( feedback );
}

void QgsMeshTriangulation::setCrs( const QgsCoordinateReferenceSystem &crs )
{
  mCrs = crs;
}

void QgsMeshTriangulation::addVerticesFromFeature( const QgsFeature &feature, int valueAttribute, const QgsCoordinateTransform &transform, QgsFeedback *feedback )
{
  QgsGeometry geom = feature.geometry();
  try
  {
    geom.transform( transform, Qgis::TransformDirection::Forward, true );
  }
  catch ( QgsCsException &cse )
  {
    Q_UNUSED( cse )
    QgsDebugMsgLevel( QStringLiteral( "Caught CRS exception %1" ).arg( cse.what() ), 4 );
    return;
  }

  QgsAbstractGeometry::vertex_iterator vit = geom.vertices_begin();

  double value = 0;
  if ( valueAttribute >= 0 )
    value = feature.attribute( valueAttribute ).toDouble();

  while ( vit != geom.vertices_end() )
  {
    if ( feedback && feedback->isCanceled() )
      break;
    if ( valueAttribute < 0 )
      mTriangulation->addPoint( *vit );
    else
    {
      mTriangulation->addPoint( QgsPoint( QgsWkbTypes::PointZ, ( *vit ).x(), ( *vit ).y(), value ) );
    }
    ++vit;
  }
}

void QgsMeshTriangulation::addBreakLinesFromFeature( const QgsFeature &feature, int valueAttribute, const QgsCoordinateTransform &transform, QgsFeedback *feedback )
{
  double valueOnVertex = 0;
  if ( valueAttribute >= 0 )
    valueOnVertex = feature.attribute( valueAttribute ).toDouble();

  //from QgsTinInterpolator::insertData()
  std::vector<const QgsCurve *> curves;
  QgsGeometry geom = feature.geometry();
  try
  {
    geom.transform( transform, Qgis::TransformDirection::Forward, true );
  }
  catch ( QgsCsException &cse )
  {
    Q_UNUSED( cse )
    QgsDebugMsgLevel( QStringLiteral( "Caught CRS exception %1" ).arg( cse.what() ), 4 );
    return;
  }

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

  for ( const QgsCurve *curve : curves )
  {
    if ( !curve )
      continue;

    if ( feedback && feedback->isCanceled() )
      break;

    QgsPointSequence linePoints;
    curve->points( linePoints );
    bool hasZ = curve->is3D();
    if ( valueAttribute >= 0 )
      for ( int i = 0; i < linePoints.count(); ++i )
      {
        if ( feedback && feedback->isCanceled() )
          break;
        if ( hasZ )
          linePoints[i].setZ( valueOnVertex );
        else
        {
          const QgsPoint &point = linePoints.at( i );
          linePoints[i] = QgsPoint( point.x(), point.y(), valueOnVertex );
        }
      }

    mTriangulation->addLine( linePoints, QgsInterpolator::SourceBreakLines );
  }
}

QgsMeshZValueDatasetGroup::QgsMeshZValueDatasetGroup( const QString &datasetGroupName, const QgsMesh &mesh ):
  QgsMeshDatasetGroup( datasetGroupName, QgsMeshDatasetGroupMetadata::DataOnVertices )
{
  mDataset = std::make_unique< QgsMeshZValueDataset >( mesh );
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
  for ( const QgsMeshVertex &vertex : mesh.vertices )
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

QgsMeshEditingDelaunayTriangulation::QgsMeshEditingDelaunayTriangulation() = default;

QString QgsMeshEditingDelaunayTriangulation::text() const
{
  return QObject::tr( "Delaunay triangulation" );
}

QgsTopologicalMesh::Changes QgsMeshEditingDelaunayTriangulation::apply( QgsMeshEditor *meshEditor )
{
  //use only vertices that are on boundary or free, if boundary
  QList<int> vertexIndextoTriangulate;

  QList<int> removedVerticesFromTriangulation;

  for ( const int vertexIndex : std::as_const( mInputVertices ) )
  {
    if ( meshEditor->isVertexFree( vertexIndex ) || meshEditor->isVertexOnBoundary( vertexIndex ) )
      vertexIndextoTriangulate.append( vertexIndex );
    else
      removedVerticesFromTriangulation.append( vertexIndex );
  }

  bool triangulationReady = false;
  bool giveUp = false;
  QgsTopologicalMesh::TopologicalFaces topologicFaces;

  while ( !triangulationReady )
  {
    QgsMeshTriangulation triangulation;

    QVector<int> triangulationVertexToMeshVertex( vertexIndextoTriangulate.count() );
    const QgsMesh *destinationMesh = meshEditor->topologicalMesh().mesh();

    for ( int i = 0; i < vertexIndextoTriangulate.count(); ++i )
    {
      triangulationVertexToMeshVertex[i] = vertexIndextoTriangulate.at( i );
      triangulation.addVertex( destinationMesh->vertices.at( vertexIndextoTriangulate.at( i ) ) );
    }

    QgsMesh resultingTriangulation = triangulation.triangulatedMesh();

    //Transform the new mesh triangulation to destination mesh faces
    QVector<QgsMeshFace> rawDestinationFaces = resultingTriangulation.faces;

    for ( QgsMeshFace &destinationFace : rawDestinationFaces )
    {
      for ( int &vertexIndex : destinationFace )
        vertexIndex = triangulationVertexToMeshVertex[vertexIndex];
    }

    //The new triangulation may contains faces that intersect existing faces, we need to remove them
    QVector<QgsMeshFace> destinationFaces;
    for ( const QgsMeshFace &face : rawDestinationFaces )
    {
      if ( meshEditor->isFaceGeometricallyCompatible( face ) )
        destinationFaces.append( face );
    }

    bool facesReady = false;
    QgsMeshEditingError previousError;
    while ( !facesReady && !giveUp )
    {
      QgsMeshEditingError error;
      topologicFaces = QgsTopologicalMesh::createNewTopologicalFaces( destinationFaces, true, error );

      if ( error == QgsMeshEditingError() )
        error = meshEditor->topologicalMesh().facesCanBeAdded( topologicFaces );

      switch ( error.errorType )
      {
        case Qgis::MeshEditingErrorType::NoError:
          facesReady = true;
          triangulationReady = true;
          break;
        case Qgis::MeshEditingErrorType::InvalidFace:
        case Qgis::MeshEditingErrorType::FlatFace:
        case Qgis::MeshEditingErrorType::TooManyVerticesInFace:
        case Qgis::MeshEditingErrorType::ManifoldFace:
          if ( error.elementIndex != -1 )
            destinationFaces.remove( error.elementIndex );
          else
            giveUp = true; //we don't know what happens, better to give up
          break;
        case Qgis::MeshEditingErrorType::InvalidVertex:
        case Qgis::MeshEditingErrorType::UniqueSharedVertex:
          facesReady = true;
          if ( error.elementIndex != -1 )
          {
            removedVerticesFromTriangulation.append( error.elementIndex );
            vertexIndextoTriangulate.removeOne( error.elementIndex );
          }
          else
            giveUp = true; //we don't know what happens, better to give up
          break;
      }
    }
  }

  Q_ASSERT( meshEditor->topologicalMesh().checkConsistency() == QgsMeshEditingError() );

  if ( !removedVerticesFromTriangulation.isEmpty() )
    mMessage = QObject::tr( "%n vertices have not been included in the triangulation", nullptr, removedVerticesFromTriangulation.count() );

  mIsFinished = true;

  if ( triangulationReady && !giveUp )
    return meshEditor->topologicalMesh().addFaces( topologicFaces );
  else
    return QgsTopologicalMesh::Changes();
}

