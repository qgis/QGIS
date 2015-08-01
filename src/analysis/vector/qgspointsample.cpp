#include "qgspointsample.h"
#include "qgsgeometry.h"
#include "qgsspatialindex.h"
#include "qgsvectorfilewriter.h"
#include "qgsvectorlayer.h"
#include <QFile>
#include "mersenne-twister.h"


QgsPointSample::QgsPointSample( QgsVectorLayer* inputLayer, const QString& outputLayer, QString nPointsAttribute, QString minDistAttribute ): mInputLayer( inputLayer ),
    mOutputLayer( outputLayer ), mNumberOfPointsAttribute( nPointsAttribute ), mMinDistanceAttribute( minDistAttribute ), mNCreatedPoints( 0 )
{
}

QgsPointSample::QgsPointSample()
    : mInputLayer( NULL )
    , mNCreatedPoints( 0 )
{
}

QgsPointSample::~QgsPointSample()
{
}

int QgsPointSample::createRandomPoints( QProgressDialog* pd )
{
  Q_UNUSED( pd );

  //create input layer from id (test if polygon, valid)
  if ( !mInputLayer )
  {
    return 1;
  }

  if ( mInputLayer->geometryType() != QGis::Polygon )
  {
    return 2;
  }

  //delete output file if it already exists
  if ( QFile::exists( mOutputLayer ) )
  {
    QgsVectorFileWriter::deleteShapeFile( mOutputLayer );
  }

  //create vector file writer
  QgsFields outputFields;
  outputFields.append( QgsField( "id", QVariant::Int ) );
  outputFields.append( QgsField( "station_id", QVariant::Int ) );
  outputFields.append( QgsField( "stratum_id", QVariant::Int ) );
  QgsVectorFileWriter writer( mOutputLayer, "UTF-8",
                              outputFields,
                              QGis::WKBPoint,
                              &( mInputLayer->crs() ) );

  //check if creation of output layer successfull
  if ( writer.hasError() != QgsVectorFileWriter::NoError )
  {
    return 3;
  }

  //init random number generator
  mt_srand( QTime::currentTime().msec() );
  QgsFeature fet;
  int nPoints = 0;
  double minDistance = 0;
  mNCreatedPoints = 0;

  QgsFeatureIterator fIt = mInputLayer->getFeatures( QgsFeatureRequest().setSubsetOfAttributes(
                             QStringList() << mNumberOfPointsAttribute << mMinDistanceAttribute, mInputLayer->fields() ) );
  while ( fIt.nextFeature( fet ) )
  {
    nPoints = fet.attribute( mNumberOfPointsAttribute ).toInt();
    if ( !mMinDistanceAttribute.isEmpty() )
    {
      minDistance = fet.attribute( mMinDistanceAttribute ).toDouble();
    }
    addSamplePoints( fet, writer, nPoints, minDistance );
  }

  return 0;
}

void QgsPointSample::addSamplePoints( QgsFeature& inputFeature, QgsVectorFileWriter& writer, int nPoints, double minDistance )
{
  if ( !inputFeature.constGeometry() )
    return;

  const QgsGeometry* geom = inputFeature.constGeometry();
  QgsRectangle geomRect = geom->boundingBox();
  if ( geomRect.isEmpty() )
  {
    return;
  }

  QgsSpatialIndex sIndex; //to check minimum distance
  QMap< QgsFeatureId, QgsPoint > pointMapForFeature;

  int nIterations = 0;
  int maxIterations = nPoints * 200;
  int points = 0;

  double randX = 0;
  double randY = 0;

  while ( nIterations < maxIterations && points < nPoints )
  {
    randX = (( double )mt_rand() / MD_RAND_MAX ) * geomRect.width() + geomRect.xMinimum();
    randY = (( double )mt_rand() / MD_RAND_MAX ) * geomRect.height() + geomRect.yMinimum();
    QgsPoint randPoint( randX, randY );
    QgsGeometry* ptGeom = QgsGeometry::fromPoint( randPoint );
    if ( ptGeom->within( geom ) && checkMinDistance( randPoint, sIndex, minDistance, pointMapForFeature ) )
    {
      //add feature to writer
      QgsFeature f( mNCreatedPoints );
      f.setAttribute( "id", mNCreatedPoints + 1 );
      f.setAttribute( "station_id", points + 1 );
      f.setAttribute( "stratum_id", inputFeature.id() );
      f.setGeometry( ptGeom );
      writer.addFeature( f );
      sIndex.insertFeature( f );
      pointMapForFeature.insert( mNCreatedPoints, randPoint );
      ++points;
      ++mNCreatedPoints;
    }
    else
    {
      delete ptGeom;
    }
    ++nIterations;
  }
}

bool QgsPointSample::checkMinDistance( QgsPoint& pt, QgsSpatialIndex& index, double minDistance, QMap< QgsFeatureId, QgsPoint >& pointMap )
{
  if ( minDistance <= 0 )
  {
    return true;
  }

  QList<QgsFeatureId> neighborList = index.nearestNeighbor( pt, 1 );
  if ( neighborList.isEmpty() )
  {
    return true;
  }

  QMap< QgsFeatureId, QgsPoint >::const_iterator it = pointMap.find( neighborList[0] );
  if ( it == pointMap.constEnd() ) //should not happen
  {
    return true;
  }

  QgsPoint neighborPt = it.value();
  if ( neighborPt.sqrDist( pt ) < ( minDistance * minDistance ) )
  {
    return false;
  }
  return true;
}




