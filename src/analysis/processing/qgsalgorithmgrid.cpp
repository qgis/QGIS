/***************************************************************************
                         qgsalgorithmgrid.cpp
                         ---------------------
    begin                : August 2019
    copyright            : (C) 2019 by Clemens Raffler
    email                : clemens dot raffler at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

//Disclaimer:This feature was developed by: Michael Minn, 2010

#include "qgsalgorithmgrid.h"
#include "qgslinestring.h"
#include "qgswkbtypes.h"
#include "qgsvectorlayer.h"
#include "qgspolygon.h"

///@cond PRIVATE

QString QgsGridAlgorithm::name() const
{
  return QStringLiteral( "creategrid" );
}

QString QgsGridAlgorithm::displayName() const
{
  return QObject::tr( "Create grid" );
}

QStringList QgsGridAlgorithm::tags() const
{
  return QObject::tr( "grid,lines,polygons,vector,create,fishnet,diamond,hexagon" ).split( ',' );
}

QString QgsGridAlgorithm::group() const
{
  return QObject::tr( "Vector creation" );
}

QString QgsGridAlgorithm::groupId() const
{
  return QStringLiteral( "vectorcreation" );
}

void QgsGridAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterEnum( QStringLiteral( "TYPE" ), QObject::tr( "Grid type" ), QStringList() << QObject::tr( "Point" ) << QObject::tr( "Line" ) << QObject::tr( "Rectangle (Polygon)" ) << QObject::tr( "Diamond (Polygon)" ) << QObject::tr( "Hexagon (Polygon)" ), false, 0 ) );

  addParameter( new QgsProcessingParameterExtent( QStringLiteral( "EXTENT" ), QObject::tr( "Grid extent" ) ) );

  addParameter( new QgsProcessingParameterDistance( QStringLiteral( "HSPACING" ), QObject::tr( "Horizontal spacing" ), 1, QStringLiteral( "CRS" ), false, 0, 1000000000.0 ) );
  addParameter( new QgsProcessingParameterDistance( QStringLiteral( "VSPACING" ), QObject::tr( "Vertical spacing" ), 1, QStringLiteral( "CRS" ), false, 0, 1000000000.0 ) );

  addParameter( new QgsProcessingParameterDistance( QStringLiteral( "HOVERLAY" ), QObject::tr( "Horizontal overlay" ), 0, QStringLiteral( "CRS" ), false, 0, 1000000000.0 ) );
  addParameter( new QgsProcessingParameterDistance( QStringLiteral( "VOVERLAY" ), QObject::tr( "Vertical overlay" ), 0, QStringLiteral( "CRS" ), false, 0, 1000000000.0 ) );

  addParameter( new QgsProcessingParameterCrs( QStringLiteral( "CRS" ), QObject::tr( "Grid CRS" ), QStringLiteral( "ProjectCrs" ) ) );

  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Grid" ), QgsProcessing::TypeVectorAnyGeometry ) );
}

QString QgsGridAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm creates a vector layer with a grid covering a given extent. "
                      "Elements in the grid can be points, lines or polygons. The size and/or "
                      "placement of each element in the grid is defined using a horizontal and "
                      "vertical spacing. The CRS of the output layer must be defined. The grid extent "
                      "and the spacing values must be expressed in the coordinates and units of "
                      "this CRS. The top-left point (minX, maxY) is used as the reference point. "
                      "That means that, at that point, an element is guaranteed to be placed. "
                      "Unless the width and height of the selected extent is a multiple of the "
                      "selected spacing, that is not true for the other points that define that extent."
                    );
}

QgsGridAlgorithm *QgsGridAlgorithm::createInstance() const
{
  return new QgsGridAlgorithm();
}

bool QgsGridAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mIdx = parameterAsEnum( parameters, QStringLiteral( "TYPE" ), context );
  mHSpacing = parameterAsDouble( parameters, QStringLiteral( "HSPACING" ), context );
  mVSpacing = parameterAsDouble( parameters, QStringLiteral( "VSPACING" ), context );
  mHOverlay = parameterAsDouble( parameters, QStringLiteral( "HOVERLAY" ), context );
  mVOverlay = parameterAsDouble( parameters, QStringLiteral( "VOVERLAY" ), context );
  mCrs = parameterAsCrs( parameters, QStringLiteral( "CRS" ), context );
  mGridExtent = parameterAsExtent( parameters, QStringLiteral( "EXTENT" ), context, mCrs );

  return true;
}

QVariantMap QgsGridAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  if ( mHSpacing <= 0 || mVSpacing <= 0 )
    throw QgsProcessingException( QObject::tr( "Invalid grid spacing. horizontal: '%1', vertical: '%2'" ).arg( mHSpacing ).arg( mVSpacing ) );

  if ( mGridExtent.width() < mHSpacing ) //check if grid extent is smaller than horizontal spacing
    throw QgsProcessingException( QObject::tr( "Horizontal spacing is too large for the covered area." ) );

  if ( mGridExtent.height() < mVSpacing ) //check if grid extent is smaller than vertical spacing
    throw QgsProcessingException( QObject::tr( "Vertical spacing is too large for the covered area." ) );

  if ( mHSpacing <= mHOverlay || mVSpacing <= mVOverlay )
    throw QgsProcessingException( QObject::tr( "Invalid overlay: horizontal: '%1', vertical: '%2'" ).arg( mHOverlay ).arg( mVOverlay ) );

  QgsFields fields = QgsFields();
  fields.append( QgsField( QStringLiteral( "id" ), QVariant::LongLong ) );
  fields.append( QgsField( QStringLiteral( "left" ), QVariant::Double ) );
  fields.append( QgsField( QStringLiteral( "top" ), QVariant::Double ) );
  fields.append( QgsField( QStringLiteral( "right" ), QVariant::Double ) );
  fields.append( QgsField( QStringLiteral( "bottom" ), QVariant::Double ) );

  QgsWkbTypes::Type outputWkb = QgsWkbTypes::Polygon;
  switch ( mIdx )
  {
    case 0:
      outputWkb = QgsWkbTypes::Point;
      break;
    case 1:
      outputWkb = QgsWkbTypes::LineString;
      break;
  }

  QString dest;
  std::unique_ptr< QgsFeatureSink > sink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, dest, fields, outputWkb, mCrs ) );
  if ( !sink )
    throw QgsProcessingException( invalidSinkError( parameters, QStringLiteral( "OUTPUT" ) ) );

  feedback->setProgress( 0 );

  switch ( mIdx )
  {
    case 0: //point
      createPointGrid( sink, feedback );
      break;
    case 1: //line
      createLineGrid( sink, feedback );
      break;
    case 2: //rectangle
      createRectangleGrid( sink, feedback );
      break;
    case 3: //diamond
      createDiamondGrid( sink, feedback );
      break;
    case 4: //hexagon
      createHexagonGrid( sink, feedback );
      break;
  }


  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), dest );
  return outputs;
}

void QgsGridAlgorithm::createPointGrid( std::unique_ptr< QgsFeatureSink > &sink, QgsProcessingFeedback *feedback )
{
  QgsFeature f = QgsFeature();

  const long long cols = static_cast<long long>( std::ceil( mGridExtent.width() / ( mHSpacing - mHOverlay ) ) );
  const long long rows = static_cast<long long>( std::ceil( mGridExtent.height() / ( mVSpacing - mVOverlay ) ) );

  long long id = 1;
  long long cnt = 0;
  const long long cellcnt = rows * cols;

  int thisProgress = 0;
  int lastProgress = 0;

  for ( long long col = 0; col < cols; col++ )
  {
    const double x = mGridExtent.xMinimum() + ( col * mHSpacing - col * mHOverlay );

    for ( long long row = 0; row < rows; row++ )
    {
      const double y = mGridExtent.yMaximum() - ( row * mVSpacing - row * mVOverlay );

      f.setGeometry( QgsGeometry( new QgsPoint( x, y ) ) );
      f.setAttributes( QgsAttributes() << id << x << y << x + mHSpacing << y + mVSpacing );
      if ( !sink->addFeature( f, QgsFeatureSink::FastInsert ) )
        throw QgsProcessingException( writeFeatureError( sink.get(), QVariantMap(), QStringLiteral( "OUTPUT" ) ) );

      id++;
      cnt++;

      thisProgress = static_cast<int>( ( static_cast<double>( cnt ) / static_cast<double>( cellcnt ) ) * 100 );
      if ( thisProgress != lastProgress )
      {
        lastProgress = thisProgress;
        feedback->setProgress( lastProgress );
      }

      if ( feedback && feedback->isCanceled() )
        break;
    }
    if ( feedback && feedback->isCanceled() )
      break;
  }
}

void QgsGridAlgorithm::createLineGrid( std::unique_ptr< QgsFeatureSink > &sink, QgsProcessingFeedback *feedback )
{
  QgsFeature f = QgsFeature();

  double hSpace[2];
  if ( mHOverlay > 0 )
  {
    hSpace[0] = mHSpacing - mHOverlay;
    hSpace[1] = mHOverlay;
  }
  else
  {
    hSpace[0] = mHSpacing;
    hSpace[1] = mHSpacing;
  }

  double vSpace[2];
  if ( mVOverlay > 0 )
  {
    vSpace[0] = mVSpacing - mVOverlay;
    vSpace[1] = mVOverlay;
  }
  else
  {
    vSpace[0] = mVSpacing;
    vSpace[1] = mVSpacing;
  }

  long long cnt = 0;
  long long id = 1;

  //latitude lines
  double cntMax = mGridExtent.height() / mVSpacing;

  int thisProgress = 0;
  int lastProgress = 0;

  double y = mGridExtent.yMaximum();

  while ( y >= mGridExtent.yMinimum() )
  {
    if ( feedback && feedback->isCanceled() )
      break;

    const QgsPoint pt1 = QgsPoint( mGridExtent.xMinimum(), y );
    const QgsPoint pt2 = QgsPoint( mGridExtent.xMaximum(), y );

    f.setGeometry( QgsGeometry( new QgsLineString( pt1, pt2 ) ) );
    f.setAttributes( QgsAttributes() << id << mGridExtent.xMinimum() << y << mGridExtent.xMaximum() << y );
    if ( !sink->addFeature( f, QgsFeatureSink::FastInsert ) )
      throw QgsProcessingException( writeFeatureError( sink.get(), QVariantMap(), QStringLiteral( "OUTPUT" ) ) );
    y = y - vSpace[cnt % 2];

    id++;
    cnt++;

    //use 50 as count multiplicator because only half of the features are processed at this point
    thisProgress = static_cast<int>( ( static_cast<double>( cnt ) / cntMax ) * 50 );
    if ( thisProgress != lastProgress )
    {
      lastProgress = thisProgress;
      feedback->setProgress( lastProgress );
    }

  }
  //set progress to 50 manually in case the division doesn't amount to 50.
  feedback->setProgress( 50 );

  //longitude lines
  cnt = 0;

  //latitude lines
  cntMax = mGridExtent.width() / mHSpacing;

  lastProgress = 50;

  double x = mGridExtent.xMinimum();

  while ( x <= mGridExtent.xMaximum() )
  {
    if ( feedback->isCanceled() )
      break;

    const QgsPoint pt1 = QgsPoint( x, mGridExtent.yMaximum() );
    const QgsPoint pt2 = QgsPoint( x, mGridExtent.yMinimum() );
    f.setGeometry( QgsGeometry( new QgsLineString( pt1, pt2 ) ) );
    f.setAttributes( QgsAttributes() << id << x << mGridExtent.yMaximum() << x << mGridExtent.yMinimum() );
    if ( !sink->addFeature( f, QgsFeatureSink::FastInsert ) )
      throw QgsProcessingException( writeFeatureError( sink.get(), QVariantMap(), QStringLiteral( "OUTPUT" ) ) );
    x = x + hSpace[cnt % 2];

    id++;
    cnt++;

    thisProgress = static_cast<int>( static_cast<double>( 50 ) + ( static_cast<double>( cnt ) / cntMax ) * 100 );
    if ( thisProgress != lastProgress )
    {
      lastProgress = thisProgress;
      feedback->setProgress( lastProgress );
    }
  }
  feedback->setProgress( 100 );
}

void QgsGridAlgorithm::createRectangleGrid( std::unique_ptr< QgsFeatureSink > &sink, QgsProcessingFeedback *feedback )
{
  QgsFeature f = QgsFeature();

  const long long cols = static_cast<long long>( std::ceil( mGridExtent.width() / ( mHSpacing - mHOverlay ) ) );
  const long long rows = static_cast<long long>( std::ceil( mGridExtent.height() / ( mVSpacing - mVOverlay ) ) );

  long long id = 1;
  long long cnt = 0;
  const long long cellcnt = rows * cols;

  int thisProgress = 0;
  int lastProgress = 0;
  QVector< double > ringX( 5 );
  QVector< double > ringY( 5 );

  for ( long long col = 0; col < cols; col++ )
  {
    if ( feedback && feedback->isCanceled() )
      break;

    const double x1 = mGridExtent.xMinimum() + ( col * mHSpacing - col * mHOverlay );
    const double x2 = x1 + mHSpacing;

    for ( long long row = 0; row < rows; row++ )
    {
      const double y1 = mGridExtent.yMaximum() - ( row * mVSpacing - row * mVOverlay );
      const double y2 = y1 - mVSpacing;

      ringX = { x1, x2, x2, x1, x1 };
      ringY = { y1, y1, y2, y2, y1 };
      std::unique_ptr< QgsPolygon > poly = std::make_unique< QgsPolygon >();
      poly->setExteriorRing( new QgsLineString( ringX, ringY ) );
      f.setGeometry( std::move( poly ) );
      f.setAttributes( QgsAttributes() << id << x1 << y1 << x2 << y2 );
      if ( !sink->addFeature( f, QgsFeatureSink::FastInsert ) )
        throw QgsProcessingException( writeFeatureError( sink.get(), QVariantMap(), QStringLiteral( "OUTPUT" ) ) );

      id++;
      cnt++;

      thisProgress = static_cast<int>( ( static_cast<double>( cnt ) / static_cast<double>( cellcnt ) ) * 100 );
      if ( thisProgress != lastProgress )
      {
        lastProgress = thisProgress;
        feedback->setProgress( lastProgress );
      }

      if ( feedback && feedback->isCanceled() )
        break;
    }
  }
}

void QgsGridAlgorithm::createDiamondGrid( std::unique_ptr< QgsFeatureSink > &sink, QgsProcessingFeedback *feedback )
{
  QgsFeature f = QgsFeature();

  const double halfHSpacing = mHSpacing / 2;
  const double halfVSpacing = mVSpacing / 2;

  const double halfHOverlay = mHOverlay / 2;
  const double halfVOverlay = mVOverlay / 2;

  const long long cols =  static_cast<long long>( std::ceil( mGridExtent.width() / ( halfHSpacing - halfHOverlay ) ) );
  const long long rows = static_cast<long long>( std::ceil( mGridExtent.height() / ( mVSpacing - halfVOverlay ) ) );

  long long id = 1;
  long long cnt = 0;
  const long long cellcnt = rows * cols;

  int thisProgress = 0;
  int lastProgress = 0;
  QVector< double > ringX( 5 );
  QVector< double > ringY( 5 );

  for ( long long col = 0; col < cols; col++ )
  {
    if ( feedback && feedback->isCanceled() )
      break;

    const double x = mGridExtent.xMinimum() - ( col * halfHOverlay );
    const double x1 = x + ( ( col + 0 ) * halfHSpacing );
    const double x2 = x + ( ( col + 1 ) * halfHSpacing );
    const double x3 = x + ( ( col + 2 ) * halfHSpacing );

    for ( long long row = 0; row < rows; row++ )
    {
      const double y = mGridExtent.yMaximum() + ( row * halfVOverlay );

      double y1;
      double y2;
      double y3;

      if ( ( col % 2 ) == 0 )
      {
        y1 = y - ( ( ( row * 2 ) + 0 ) * halfVSpacing );
        y2 = y - ( ( ( row * 2 ) + 1 ) * halfVSpacing );
        y3 = y - ( ( ( row * 2 ) + 2 ) * halfVSpacing );
      }
      else
      {
        y1 = y - ( ( ( row * 2 ) + 1 ) * halfVSpacing );
        y2 = y - ( ( ( row * 2 ) + 2 ) * halfVSpacing );
        y3 = y - ( ( ( row * 2 ) + 3 ) * halfVSpacing );
      }

      ringX = { x1, x2, x3, x2, x1 };
      ringY = { y2, y1, y2, y3, y2 };
      std::unique_ptr< QgsPolygon > poly = std::make_unique< QgsPolygon >();
      poly->setExteriorRing( new QgsLineString( ringX, ringY ) );
      f.setGeometry( std::move( poly ) );
      f.setAttributes( QgsAttributes() << id << x1 << y1 << x3 << y3 );
      if ( !sink->addFeature( f, QgsFeatureSink::FastInsert ) )
        throw QgsProcessingException( writeFeatureError( sink.get(), QVariantMap(), QStringLiteral( "OUTPUT" ) ) );

      id++;
      cnt++;

      thisProgress = static_cast<int>( ( static_cast<double>( cnt ) / static_cast<double>( cellcnt ) ) * 100 );
      if ( thisProgress != lastProgress )
      {
        lastProgress = thisProgress;
        feedback->setProgress( lastProgress );
      }

      if ( feedback && feedback->isCanceled() )
        break;
    }
  }
}

void QgsGridAlgorithm::createHexagonGrid( std::unique_ptr<QgsFeatureSink> &sink, QgsProcessingFeedback *feedback )
{
  QgsFeature f = QgsFeature();

  // To preserve symmetry, hspacing is fixed relative to vspacing
  const double xVertexLo = 0.288675134594813 * mVSpacing;
  const double xVertexHi = 0.577350269189626 * mVSpacing;

  mHSpacing = xVertexLo + xVertexHi;

  mHOverlay = mHSpacing - mHOverlay;

  if ( mHOverlay < 0 )
  {
    throw QgsProcessingException( QObject::tr( "To preserve symmetry, hspacing is fixed relative to vspacing\n hspacing is fixed at: %1 and hoverlay is fixed at: %2 hoverlay cannot be negative. Increase hoverlay." ).arg( mHSpacing ).arg( mHOverlay ) );
  }

  const double halfVSpacing = mVSpacing / 2;

  const long long cols =  static_cast<long long>( std::ceil( mGridExtent.width() / ( mHOverlay ) ) );
  const long long rows = static_cast<long long>( std::ceil( mGridExtent.height() / ( mVSpacing - mVOverlay ) ) );

  long long id = 1;
  long long cnt = 0;
  const long long cellcnt = rows * cols;

  int thisProgress = 0;
  int lastProgress = 0;

  QVector< double > ringX( 7 );
  QVector< double > ringY( 7 );
  for ( long long col = 0; col < cols; col++ )
  {
    if ( feedback && feedback->isCanceled() )
      break;

    // (column + 1) and (row + 1) calculation is used to maintain
    // topology between adjacent shapes and avoid overlaps/holes
    // due to rounding errors

    const double x1 = mGridExtent.xMinimum() + ( col * mHOverlay );
    const double x2 = x1 + ( xVertexHi - xVertexLo );
    const double x3 = mGridExtent.xMinimum() + ( col * mHOverlay ) + mHSpacing;
    const double x4 = x3 + ( xVertexHi - xVertexLo );

    for ( long long row = 0; row < rows; row++ )
    {
      double y1;
      double y2;
      double y3;

      if ( ( col % 2 ) == 0 )
      {
        y1 = mGridExtent.yMaximum() + ( row * mVOverlay ) - ( ( ( row * 2 ) + 0 ) * halfVSpacing );
        y2 = mGridExtent.yMaximum() + ( row * mVOverlay ) - ( ( ( row * 2 ) + 1 ) * halfVSpacing );
        y3 = mGridExtent.yMaximum() + ( row * mVOverlay ) - ( ( ( row * 2 ) + 2 ) * halfVSpacing );
      }
      else
      {
        y1 = mGridExtent.yMaximum() + ( row * mVOverlay ) - ( ( ( row * 2 ) + 1 ) * halfVSpacing );
        y2 = mGridExtent.yMaximum() + ( row * mVOverlay ) - ( ( ( row * 2 ) + 2 ) * halfVSpacing );
        y3 = mGridExtent.yMaximum() + ( row * mVOverlay ) - ( ( ( row * 2 ) + 3 ) * halfVSpacing );
      }

      ringX = { x1, x2, x3, x4, x3, x2, x1 };
      ringY = { y2, y1, y1, y2, y3, y3, y2 };
      std::unique_ptr< QgsPolygon > poly = std::make_unique< QgsPolygon >();
      poly->setExteriorRing( new QgsLineString( ringX, ringY ) );
      f.setGeometry( std::move( poly ) );
      f.setAttributes( QgsAttributes() << id << x1 << y1 << x4 << y3 );
      if ( !sink->addFeature( f, QgsFeatureSink::FastInsert ) )
        throw QgsProcessingException( writeFeatureError( sink.get(), QVariantMap(), QStringLiteral( "OUTPUT" ) ) );

      id++;
      cnt++;

      thisProgress = static_cast<int>( ( static_cast<double>( cnt ) / static_cast<double>( cellcnt ) ) * 100 );
      if ( thisProgress != lastProgress )
      {
        lastProgress = thisProgress;
        feedback->setProgress( lastProgress );
      }

      if ( feedback && feedback->isCanceled() )
        break;
    }
  }
}

///@endcond
