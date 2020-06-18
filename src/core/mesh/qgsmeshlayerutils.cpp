/***************************************************************************
                         qgsmeshlayerutils.cpp
                         --------------------------
    begin                : August 2018
    copyright            : (C) 2018 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <limits>
#include <QTime>
#include <QDateTime>

#include "qgsmeshlayerutils.h"
#include "qgsmeshtimesettings.h"
#include "qgstriangularmesh.h"
#include "qgslogger.h"
#include "qgsmeshdataprovider.h"
#include "qgsmesh3daveraging.h"
#include "qgsmeshlayer.h"


///@cond PRIVATE

int QgsMeshLayerUtils::datasetValuesCount( const QgsMesh *mesh, QgsMeshDatasetGroupMetadata::DataType dataType )
{
  if ( !mesh )
    return 0;

  switch ( dataType )
  {
    case QgsMeshDatasetGroupMetadata::DataType::DataOnEdges:
      return mesh->edgeCount();
    case QgsMeshDatasetGroupMetadata::DataType::DataOnFaces:
      return mesh->faceCount();
    case QgsMeshDatasetGroupMetadata::DataType::DataOnVertices:
      return mesh->vertexCount();
    case QgsMeshDatasetGroupMetadata::DataType::DataOnVolumes:
      return mesh->faceCount(); // because they are averaged to faces
  }
  return 0;
}

QgsMeshDatasetGroupMetadata::DataType QgsMeshLayerUtils::datasetValuesType( const QgsMeshDatasetGroupMetadata::DataType &type )
{
  // data on volumes are averaged to 2D data on faces
  if ( type == QgsMeshDatasetGroupMetadata::DataType::DataOnVolumes )
    return QgsMeshDatasetGroupMetadata::DataType::DataOnFaces;

  return type;
}

QgsMeshDataBlock QgsMeshLayerUtils::datasetValues(
  const QgsMeshLayer *meshLayer,
  QgsMeshDatasetIndex index,
  int valueIndex,
  int count )
{
  QgsMeshDataBlock block;
  if ( !meshLayer )
    return block;

  const QgsMeshDataProvider *provider = meshLayer->dataProvider();
  if ( !provider )
    return block;

  if ( !index.isValid() )
    return block;

  const QgsMeshDatasetGroupMetadata meta = meshLayer->dataProvider()->datasetGroupMetadata( index.group() );
  if ( meta.dataType() != QgsMeshDatasetGroupMetadata::DataType::DataOnVolumes )
  {
    block = provider->datasetValues( index, valueIndex, count );
    if ( block.isValid() )
      return block;
  }
  else
  {
    const QgsMesh3dAveragingMethod *averagingMethod = meshLayer->rendererSettings().averagingMethod();
    if ( !averagingMethod )
      return block;

    QgsMesh3dDataBlock block3d = provider->dataset3dValues( index, valueIndex, count );
    if ( !block3d.isValid() )
      return block;

    block = averagingMethod->calculate( block3d );
  }
  return block;
}

QVector<QgsVector> QgsMeshLayerUtils::griddedVectorValues( const QgsMeshLayer *meshLayer,
    const QgsMeshDatasetIndex index,
    double xSpacing,
    double ySpacing,
    const QSize &size,
    const QgsPointXY &minCorner )
{
  QVector<QgsVector> vectors;

  if ( !meshLayer || !index.isValid() )
    return vectors;

  const QgsTriangularMesh *triangularMesh = meshLayer->triangularMesh();
  const QgsMesh *nativeMesh = meshLayer->nativeMesh();

  if ( !triangularMesh || !nativeMesh )
    return vectors;

  QgsMeshDatasetGroupMetadata meta = meshLayer->dataProvider()->datasetGroupMetadata( index );
  if ( !meta.isVector() )
    return vectors;

  // extract vector dataset
  bool vectorDataOnVertices = meta.dataType() == QgsMeshDatasetGroupMetadata::DataOnVertices;
  int datacount = vectorDataOnVertices ? nativeMesh->vertices.count() : nativeMesh->faces.count();
  const QgsMeshDataBlock vals = QgsMeshLayerUtils::datasetValues( meshLayer, index, 0, datacount );

  const QgsMeshDataBlock isFacesActive = meshLayer->dataProvider()->areFacesActive( index, 0, nativeMesh->faceCount() );
  const QgsMeshDatasetGroupMetadata::DataType dataType = meta.dataType();

  if ( dataType == QgsMeshDatasetGroupMetadata::DataOnEdges )
    return vectors;

  try
  {
    vectors.reserve( size.height()*size.width() );
  }
  catch ( ... )
  {
    QgsDebugMsgLevel( "Unable to store the arrow grid in memory", 1 );
    return QVector<QgsVector>();
  }

  for ( int iy = 0; iy < size.height(); ++iy )
  {
    double y = minCorner.y() + iy * ySpacing;
    for ( int ix = 0; ix < size.width(); ++ix )
    {
      double x = minCorner.x() + ix * xSpacing;
      QgsPoint point( x, y );
      int faceIndex = triangularMesh->faceIndexForPoint_v2( point );
      int nativeFaceIndex = -1;
      if ( faceIndex != -1 )
        nativeFaceIndex = triangularMesh->trianglesToNativeFaces().at( faceIndex );
      QgsMeshDatasetValue value;
      if ( nativeFaceIndex != -1 && isFacesActive.active( nativeFaceIndex ) )
      {
        switch ( dataType )
        {
          case QgsMeshDatasetGroupMetadata::DataOnFaces:
          case QgsMeshDatasetGroupMetadata::DataOnVolumes:
            value = vals.value( nativeFaceIndex );
            break;
          case QgsMeshDatasetGroupMetadata::DataOnVertices:
          {
            const QgsMeshFace &face = triangularMesh->triangles()[faceIndex];
            const int v1 = face[0], v2 = face[1], v3 = face[2];
            const QgsPoint p1 = triangularMesh->vertices()[v1], p2 = triangularMesh->vertices()[v2], p3 = triangularMesh->vertices()[v3];
            const QgsMeshDatasetValue val1 = vals.value( v1 );
            const QgsMeshDatasetValue val2 = vals.value( v2 );
            const QgsMeshDatasetValue val3 = vals.value( v3 );
            const double x = QgsMeshLayerUtils::interpolateFromVerticesData( p1, p2, p3, val1.x(), val2.x(), val3.x(), point );
            const double y = QgsMeshLayerUtils::interpolateFromVerticesData( p1, p2, p3, val1.y(), val2.y(), val3.y(), point );
            value = QgsMeshDatasetValue( x, y );
          }
          break;
          case QgsMeshDatasetGroupMetadata::DataOnEdges:
            break;
        }
      }
      vectors.append( QgsVector( value.x(), value.y() ) );
    }
  }
  return vectors;
}

QVector<double> QgsMeshLayerUtils::calculateMagnitudes( const QgsMeshDataBlock &block )
{
  Q_ASSERT( QgsMeshDataBlock::ActiveFlagInteger != block.type() );
  int count = block.count();
  QVector<double> ret( count );

  for ( int i = 0; i < count; ++i )
  {
    double mag = block.value( i ).scalar();
    ret[i] = mag;
  }
  return ret;
}

void QgsMeshLayerUtils::boundingBoxToScreenRectangle( const QgsMapToPixel &mtp,
    const QSize &outputSize,
    const QgsRectangle &bbox,
    int &leftLim,
    int &rightLim,
    int &topLim,
    int &bottomLim )
{
  QgsPointXY ll = mtp.transform( bbox.xMinimum(), bbox.yMinimum() );
  QgsPointXY ur = mtp.transform( bbox.xMaximum(), bbox.yMaximum() );
  topLim = std::max( int( ur.y() ), 0 );
  bottomLim = std::min( int( ll.y() ), outputSize.height() - 1 );
  leftLim = std::max( int( ll.x() ), 0 );
  rightLim = std::min( int( ur.x() ), outputSize.width() - 1 );
}

static void lamTol( double &lam )
{
  const static double eps = 1e-6;
  if ( ( lam < 0.0 ) && ( lam > -eps ) )
  {
    lam = 0.0;
  }
}

static bool E3T_physicalToBarycentric( const QgsPointXY &pA, const QgsPointXY &pB, const QgsPointXY &pC, const QgsPointXY &pP,
                                       double &lam1, double &lam2, double &lam3 )
{
  if ( pA == pB || pA == pC || pB == pC )
    return false; // this is not a valid triangle!

  // Compute vectors
  QgsVector v0( pC - pA );
  QgsVector v1( pB - pA );
  QgsVector v2( pP - pA );

  // Compute dot products
  double dot00 = v0 * v0;
  double dot01 = v0 * v1;
  double dot02 = v0 * v2;
  double dot11 = v1 * v1;
  double dot12 = v1 * v2;

  // Compute barycentric coordinates
  double invDenom = 1.0 / ( dot00 * dot11 - dot01 * dot01 );
  lam1 = ( dot11 * dot02 - dot01 * dot12 ) * invDenom;
  lam2 = ( dot00 * dot12 - dot01 * dot02 ) * invDenom;
  lam3 = 1.0 - lam1 - lam2;

  // Apply some tolerance to lam so we can detect correctly border points
  lamTol( lam1 );
  lamTol( lam2 );
  lamTol( lam3 );

  // Return if POI is outside triangle
  if ( ( lam1 < 0 ) || ( lam2 < 0 ) || ( lam3 < 0 ) )
  {
    return false;
  }

  return true;
}

double QgsMeshLayerUtils::interpolateFromVerticesData( const QgsPointXY &p1, const QgsPointXY &p2, const QgsPointXY &p3,
    double val1, double val2, double val3, const QgsPointXY &pt )
{
  double lam1, lam2, lam3;
  if ( !E3T_physicalToBarycentric( p1, p2, p3, pt, lam1, lam2, lam3 ) )
    return std::numeric_limits<double>::quiet_NaN();

  return lam1 * val3 + lam2 * val2 + lam3 * val1;
}

double QgsMeshLayerUtils::interpolateFromVerticesData( double fraction, double val1, double val2 )
{
  if ( std::isnan( val1 ) || std::isnan( val2 ) || ( fraction < 0 ) || ( fraction > 1 ) )
  {
    return std::numeric_limits<double>::quiet_NaN();
  }
  return val1 + ( val2 - val1 ) * fraction;
}

QgsMeshDatasetValue QgsMeshLayerUtils::interpolateFromVerticesData( double fraction, const QgsMeshDatasetValue &val1, const QgsMeshDatasetValue &val2 )
{
  return QgsMeshDatasetValue( interpolateFromVerticesData( fraction, val1.x(), val2.x() ),
                              interpolateFromVerticesData( fraction, val1.y(), val2.y() ) );
}


QgsVector QgsMeshLayerUtils::interpolateVectorFromVerticesData( const QgsPointXY &p1, const QgsPointXY &p2, const QgsPointXY &p3, QgsVector vect1, QgsVector vect2, QgsVector vect3, const QgsPointXY &pt )
{
  double lam1, lam2, lam3;
  if ( !E3T_physicalToBarycentric( p1, p2, p3, pt, lam1, lam2, lam3 ) )
    return QgsVector( std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN() );

  return vect3 * lam1 + vect2 * lam2 + vect1 * lam3;
}

double QgsMeshLayerUtils::interpolateFromFacesData( const QgsPointXY &p1, const QgsPointXY &p2, const QgsPointXY &p3,
    double val, const QgsPointXY &pt )
{
  double lam1, lam2, lam3;
  if ( !E3T_physicalToBarycentric( p1, p2, p3, pt, lam1, lam2, lam3 ) )
    return std::numeric_limits<double>::quiet_NaN();

  return val;
}

QgsVector QgsMeshLayerUtils::interpolateVectorFromFacesData( const QgsPointXY &p1, const QgsPointXY &p2, const QgsPointXY &p3,
    QgsVector vect, const QgsPointXY &pt )
{
  double lam1, lam2, lam3;
  if ( !E3T_physicalToBarycentric( p1, p2, p3, pt, lam1, lam2, lam3 ) )
    return QgsVector( std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN() );

  return vect;
}


QVector<double> QgsMeshLayerUtils::interpolateFromFacesData(
  QVector<double> valuesOnFaces,
  const QgsMesh *nativeMesh,
  const QgsTriangularMesh *triangularMesh,
  QgsMeshDataBlock *active,
  QgsMeshRendererScalarSettings::DataResamplingMethod method )
{
  assert( nativeMesh );
  Q_UNUSED( method );
  assert( method == QgsMeshRendererScalarSettings::NeighbourAverage );

  // assuming that native vertex count = triangular vertex count
  assert( nativeMesh->vertices.size() == triangularMesh->vertices().size() );
  int vertexCount = triangularMesh->vertices().size();

  QVector<double> res( vertexCount, 0.0 );
  // for face datasets do simple average of the valid values of all faces that contains this vertex
  QVector<int> count( vertexCount, 0 );

  for ( int i = 0; i < nativeMesh->faces.size(); ++i )
  {
    if ( !active || active->active( i ) )
    {
      double val = valuesOnFaces[ i ];
      if ( !std::isnan( val ) )
      {
        // assign for all vertices
        const QgsMeshFace &face = nativeMesh->faces.at( i );
        for ( int j = 0; j < face.size(); ++j )
        {
          int vertexIndex = face[j];
          res[vertexIndex] += val;
          count[vertexIndex] += 1;
        }
      }
    }
  }

  for ( int i = 0; i < vertexCount; ++i )
  {
    if ( count.at( i ) > 0 )
    {
      res[i] = res[i] / double( count.at( i ) );
    }
    else
    {
      res[i] = std::numeric_limits<double>::quiet_NaN();
    }
  }

  return res;
}

QVector<double> QgsMeshLayerUtils::resampleFromVerticesToFaces(
  const QVector<double> valuesOnVertices,
  const QgsMesh *nativeMesh,
  const QgsTriangularMesh *triangularMesh,
  const QgsMeshDataBlock *active,
  QgsMeshRendererScalarSettings::DataResamplingMethod method )
{
  assert( nativeMesh );
  Q_UNUSED( method );
  assert( method == QgsMeshRendererScalarSettings::NeighbourAverage );

  // assuming that native vertex count = triangular vertex count
  Q_UNUSED( triangularMesh );
  assert( nativeMesh->vertices.size() == triangularMesh->vertices().size() );

  QVector<double> ret( nativeMesh->faceCount(), std::numeric_limits<double>::quiet_NaN() );

  for ( int i = 0; i < nativeMesh->faces.size(); ++i )
  {
    const QgsMeshFace face = nativeMesh->face( i );
    if ( active->active( i ) && face.count() > 2 )
    {
      double value = 0;
      for ( int j = 0; j < face.count(); ++j )
      {
        value += valuesOnVertices.at( face.at( j ) );
      }
      ret[i] = value / face.count();
    }
  }

  return ret;
}

QVector<double> QgsMeshLayerUtils::calculateMagnitudeOnVertices( const QgsMeshLayer *meshLayer,
    const QgsMeshDatasetIndex index,
    QgsMeshDataBlock *activeFaceFlagValues,
    const QgsMeshRendererScalarSettings::DataResamplingMethod method )
{
  QVector<double> ret;

  if ( !meshLayer && !index.isValid() )
    return ret;

  const QgsTriangularMesh *triangularMesh = meshLayer->triangularMesh();
  const QgsMesh *nativeMesh = meshLayer->nativeMesh();
  if ( !triangularMesh || !nativeMesh )
    return ret;

  const QgsMeshDatasetGroupMetadata metadata = meshLayer->dataProvider()->datasetGroupMetadata( index );
  bool scalarDataOnVertices = metadata.dataType() == QgsMeshDatasetGroupMetadata::DataOnVertices;

  // populate scalar values
  int datacount = scalarDataOnVertices ? nativeMesh->vertices.count() : nativeMesh->faces.count();
  QgsMeshDataBlock vals = QgsMeshLayerUtils::datasetValues(
                            meshLayer,
                            index,
                            0,
                            datacount );

  if ( vals.isValid() )
  {
    ret = QgsMeshLayerUtils::calculateMagnitudes( vals );

    if ( !scalarDataOnVertices )
    {
      //Need to interpolate data on vertices
      ret = QgsMeshLayerUtils::interpolateFromFacesData(
              ret,
              nativeMesh,
              triangularMesh,
              activeFaceFlagValues,
              method );
    }
  }
  return ret;
}

QgsRectangle QgsMeshLayerUtils::triangleBoundingBox( const QgsPointXY &p1, const QgsPointXY &p2, const QgsPointXY &p3 )
{
  // p1
  double xMin = p1.x();
  double xMax = p1.x();
  double yMin = p1.y();
  double yMax = p1.y();

  //p2
  xMin = ( ( xMin < p2.x() ) ? xMin : p2.x() );
  xMax = ( ( xMax > p2.x() ) ? xMax : p2.x() );
  yMin = ( ( yMin < p2.y() ) ? yMin : p2.y() );
  yMax = ( ( yMax > p2.y() ) ? yMax : p2.y() );

  // p3
  xMin = ( ( xMin < p3.x() ) ? xMin : p3.x() );
  xMax = ( ( xMax > p3.x() ) ? xMax : p3.x() );
  yMin = ( ( yMin < p3.y() ) ? yMin : p3.y() );
  yMax = ( ( yMax > p3.y() ) ? yMax : p3.y() );

  QgsRectangle bbox( xMin,  yMin,  xMax,  yMax );
  return bbox;
}

QString QgsMeshLayerUtils::formatTime( double hours, const QDateTime &referenceTime, const QgsMeshTimeSettings &settings )
{
  QString ret;

  if ( referenceTime.isValid() )
  {
    QString format( settings.absoluteTimeFormat() );
    QDateTime dateTime( referenceTime );
    qint64 seconds = static_cast<qint64>( hours * 3600.0 );
    dateTime = dateTime.addSecs( seconds );
    ret = dateTime.toString( format );
    if ( ret.isEmpty() ) // error
      ret = dateTime.toString();
  }
  else
  {
    QString format( settings.relativeTimeFormat() );
    format = format.trimmed();
    int totalHours = static_cast<int>( hours );

    if ( format == QStringLiteral( "hh:mm:ss.zzz" ) )
    {
      int ms = static_cast<int>( hours * 3600.0 * 1000 );
      int seconds = ms / 1000;
      int z = ms % 1000;
      int m = seconds / 60;
      int s = seconds % 60;
      int h = m / 60;
      m = m % 60;
      ret = QStringLiteral( "%1:%2:%3.%4" ).
            arg( h, 2, 10, QLatin1Char( '0' ) ).
            arg( m, 2, 10, QLatin1Char( '0' ) ).
            arg( s, 2, 10, QLatin1Char( '0' ) ).
            arg( z, 3, 10, QLatin1Char( '0' ) );
    }
    else if ( format == QStringLiteral( "hh:mm:ss" ) )
    {
      int seconds = static_cast<int>( hours * 3600.0 );
      int m = seconds / 60;
      int s = seconds % 60;
      int h = m / 60;
      m = m % 60;
      ret = QStringLiteral( "%1:%2:%3" ).
            arg( h, 2, 10, QLatin1Char( '0' ) ).
            arg( m, 2, 10, QLatin1Char( '0' ) ).
            arg( s, 2, 10, QLatin1Char( '0' ) );

    }
    else if ( format == QStringLiteral( "d hh:mm:ss" ) )
    {
      int seconds = static_cast<int>( hours * 3600.0 );
      int m = seconds / 60;
      int s = seconds % 60;
      int h = m / 60;
      m = m % 60;
      int d = totalHours / 24;
      h = totalHours % 24;
      ret = QStringLiteral( "%1 d %2:%3:%4" ).
            arg( d ).
            arg( h, 2, 10, QLatin1Char( '0' ) ).
            arg( m, 2, 10, QLatin1Char( '0' ) ).
            arg( s, 2, 10, QLatin1Char( '0' ) );
    }
    else if ( format == QStringLiteral( "d hh" ) )
    {
      int d = totalHours / 24;
      int h = totalHours % 24;
      ret = QStringLiteral( "%1 d %2" ).
            arg( d ).
            arg( h );
    }
    else if ( format == QStringLiteral( "d" ) )
    {
      int d = totalHours / 24;
      ret = QStringLiteral( "%1" ).arg( d );
    }
    else if ( format == QStringLiteral( "ss" ) )
    {
      int seconds = static_cast<int>( hours * 3600.0 );
      ret = QStringLiteral( "%1" ).arg( seconds );
    }
    else     // "hh"
    {
      ret = QStringLiteral( "%1" ).arg( hours );
    }
  }
  return ret;
}

QVector<QVector3D> QgsMeshLayerUtils::calculateNormals( const QgsTriangularMesh &triangularMesh, const QVector<double> &verticalMagnitude, bool isRelative )
{
  QVector<QVector3D> normals( triangularMesh.vertices().count() );
  for ( const auto &face : triangularMesh.triangles() )
  {
    for ( int i = 0; i < 3; i++ )
    {
      int index( face.at( i ) );
      int index1( face.at( ( i + 1 ) % 3 ) );
      int index2( face.at( ( i + 2 ) % 3 ) );

      const QgsMeshVertex &vert( triangularMesh.vertices().at( index ) );
      const QgsMeshVertex &otherVert1( triangularMesh.vertices().at( index1 ) );
      const QgsMeshVertex &otherVert2( triangularMesh.vertices().at( index2 ) );

      float adjustRelative = 0;
      float adjustRelative1 = 0;
      float adjustRelative2 = 0;

      if ( isRelative )
      {
        adjustRelative = vert.z();
        adjustRelative1 = otherVert1.z();
        adjustRelative2 = otherVert2.z();
      }

      QVector3D v1( float( otherVert1.x() - vert.x() ),
                    float( otherVert1.y() - vert.y() ),
                    float( verticalMagnitude[index1] - verticalMagnitude[index] + adjustRelative1 - adjustRelative ) );
      QVector3D v2( float( otherVert2.x() - vert.x() ),
                    float( otherVert2.y() - vert.y() ),
                    float( verticalMagnitude[index2] - verticalMagnitude[index] + adjustRelative2 - adjustRelative ) );

      normals[index] += QVector3D::crossProduct( v1, v2 );
    }
  }

  return normals;
}

///@endcond
