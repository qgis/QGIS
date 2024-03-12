/***************************************************************************
  qgsquickmapsettings.cpp
  --------------------------------------
  Date                 : 27.12.2014
  Copyright            : (C) 2014 by Matthias Kuhn
  Email                : matthias (at) opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgis.h"
#include "qgsquickmapsettings.h"

#include "qgsmaplayer.h"
#include "qgsmessagelog.h"
#include "qgsprojectviewsettings.h"

QgsQuickMapSettings::QgsQuickMapSettings( QObject *parent )
  : QObject( parent )
{
  // Connect signals for derived values
  connect( this, &QgsQuickMapSettings::destinationCrsChanged, this, &QgsQuickMapSettings::mapUnitsPerPixelChanged );
  connect( this, &QgsQuickMapSettings::extentChanged, this, &QgsQuickMapSettings::mapUnitsPerPixelChanged );
  connect( this, &QgsQuickMapSettings::outputSizeChanged, this, &QgsQuickMapSettings::mapUnitsPerPixelChanged );
  connect( this, &QgsQuickMapSettings::extentChanged, this, &QgsQuickMapSettings::visibleExtentChanged );
  connect( this, &QgsQuickMapSettings::rotationChanged, this, &QgsQuickMapSettings::visibleExtentChanged );
  connect( this, &QgsQuickMapSettings::outputSizeChanged, this, &QgsQuickMapSettings::visibleExtentChanged );
}

void QgsQuickMapSettings::setProject( QgsProject *project )
{
  if ( project == mProject )
    return;

  // If we have already something connected, disconnect it!
  if ( mProject )
  {
    mProject->disconnect( this );
  }

  mProject = project;

  // Connect all signals
  if ( mProject )
  {
    connect( mProject, &QgsProject::readProject, this, &QgsQuickMapSettings::onReadProject );
    connect( mProject, &QgsProject::crsChanged, this, &QgsQuickMapSettings::onCrsChanged );
    setDestinationCrs( mProject->crs() );
    mMapSettings.setTransformContext( mProject->transformContext() );
    mMapSettings.setPathResolver( mProject->pathResolver() );
  }
  else
  {
    mMapSettings.setTransformContext( QgsCoordinateTransformContext() );
  }

  emit projectChanged();
}

QgsProject *QgsQuickMapSettings::project() const
{
  return mProject;
}

QgsCoordinateTransformContext QgsQuickMapSettings::transformContext() const
{
  return mMapSettings.transformContext();
}

QgsRectangle QgsQuickMapSettings::extent() const
{
  return mMapSettings.extent();
}

void QgsQuickMapSettings::setExtent( const QgsRectangle &extent )
{
  if ( mMapSettings.extent() == extent )
    return;

  mMapSettings.setExtent( extent );
  emit extentChanged();
}

QgsPoint QgsQuickMapSettings::center() const
{
  return QgsPoint( extent().center() );
}

void QgsQuickMapSettings::setCenter( const QgsPoint &center )
{
  QgsVector delta = QgsPointXY( center ) - mMapSettings.extent().center();

  QgsRectangle e = mMapSettings.extent();
  e.setXMinimum( e.xMinimum() + delta.x() );
  e.setXMaximum( e.xMaximum() + delta.x() );
  e.setYMinimum( e.yMinimum() + delta.y() );
  e.setYMaximum( e.yMaximum() + delta.y() );

  setExtent( e );
}

double QgsQuickMapSettings::mapUnitsPerPixel() const
{
  return mMapSettings.mapUnitsPerPixel();
}

void QgsQuickMapSettings::setCenterToLayer( QgsMapLayer *layer, bool shouldZoom )
{
  Q_ASSERT( layer );

  const QgsRectangle extent = mapSettings().layerToMapCoordinates( layer, layer->extent() );

  if ( !extent.isEmpty() )
  {
    if ( shouldZoom )
      setExtent( extent );
    else
      setCenter( QgsPoint( extent.center() ) );
  }
}

double QgsQuickMapSettings::mapUnitsPerPoint() const
{
  return mMapSettings.mapUnitsPerPixel() * devicePixelRatio();
}

QgsRectangle QgsQuickMapSettings::visibleExtent() const
{
  return mMapSettings.visibleExtent();
}

QPointF QgsQuickMapSettings::coordinateToScreen( const QgsPoint &point ) const
{
  QgsPointXY pt( point.x(), point.y() );
  QgsPointXY pp = mMapSettings.mapToPixel().transform( pt );
  pp.setX( pp.x() / devicePixelRatio() );
  pp.setY( pp.y() / devicePixelRatio() );
  return pp.toQPointF();
}

QgsPoint QgsQuickMapSettings::screenToCoordinate( const QPointF &point ) const
{
  const QgsPointXY pp = mMapSettings.mapToPixel().toMapCoordinates( point.x() * devicePixelRatio(), point.y() * devicePixelRatio() );
  return QgsPoint( pp );
}

void QgsQuickMapSettings::setTransformContext( const QgsCoordinateTransformContext &ctx )
{
  mMapSettings.setTransformContext( ctx );
}

QgsMapSettings QgsQuickMapSettings::mapSettings() const
{
  return mMapSettings;
}

QSize QgsQuickMapSettings::outputSize() const
{
  return mMapSettings.outputSize();
}

void QgsQuickMapSettings::setOutputSize( QSize outputSize )
{
  outputSize.setWidth( outputSize.width() * devicePixelRatio() );
  outputSize.setHeight( outputSize.height() * devicePixelRatio() );
  if ( mMapSettings.outputSize() == outputSize )
    return;

  mMapSettings.setOutputSize( outputSize );
  emit outputSizeChanged();
}

double QgsQuickMapSettings::outputDpi() const
{
  return mMapSettings.outputDpi();
}

void QgsQuickMapSettings::setOutputDpi( double outputDpi )
{
  outputDpi *= devicePixelRatio();
  if ( qgsDoubleNear( mMapSettings.outputDpi(), outputDpi ) )
    return;

  mMapSettings.setOutputDpi( outputDpi );
  emit outputDpiChanged();
}

QgsCoordinateReferenceSystem QgsQuickMapSettings::destinationCrs() const
{
  return mMapSettings.destinationCrs();
}

void QgsQuickMapSettings::setDestinationCrs( const QgsCoordinateReferenceSystem &destinationCrs )
{
  if ( mMapSettings.destinationCrs() == destinationCrs )
    return;

  mMapSettings.setDestinationCrs( destinationCrs );
  emit destinationCrsChanged();
}

QList<QgsMapLayer *> QgsQuickMapSettings::layers() const
{
  return mMapSettings.layers();
}

void QgsQuickMapSettings::setLayers( const QList<QgsMapLayer *> &layers )
{
  mMapSettings.setLayers( layers );
  emit layersChanged();
}

void QgsQuickMapSettings::onCrsChanged()
{
  setDestinationCrs( mProject->crs() );
}

void QgsQuickMapSettings::onReadProject( const QDomDocument &doc )
{
  if ( mProject )
  {
    int red = mProject->readNumEntry( QStringLiteral( "Gui" ), QStringLiteral( "/CanvasColorRedPart" ), 255 );
    int green = mProject->readNumEntry( QStringLiteral( "Gui" ), QStringLiteral( "/CanvasColorGreenPart" ), 255 );
    int blue = mProject->readNumEntry( QStringLiteral( "Gui" ), QStringLiteral( "/CanvasColorBluePart" ), 255 );
    mMapSettings.setBackgroundColor( QColor( red, green, blue ) );

    const bool isTemporal = mProject->readNumEntry( QStringLiteral( "TemporalControllerWidget" ), QStringLiteral( "/NavigationMode" ), 0 ) != 0;
    const QString startString = QgsProject::instance()->readEntry( QStringLiteral( "TemporalControllerWidget" ), QStringLiteral( "/StartDateTime" ) );
    const QString endString = QgsProject::instance()->readEntry( QStringLiteral( "TemporalControllerWidget" ), QStringLiteral( "/EndDateTime" ) );
    mMapSettings.setIsTemporal( isTemporal );
    mMapSettings.setTemporalRange( QgsDateTimeRange( QDateTime::fromString( startString, Qt::ISODateWithMs ),
                                   QDateTime::fromString( endString, Qt::ISODateWithMs ) ) );
  }

  QDomNodeList nodes = doc.elementsByTagName( "mapcanvas" );
  bool foundTheMapCanvas = false;
  for ( int i = 0; i < nodes.size(); i++ )
  {
    QDomNode node = nodes.item( 0 );
    QDomElement element = node.toElement();

    if ( element.hasAttribute( QStringLiteral( "name" ) ) && element.attribute( QStringLiteral( "name" ) ) == QLatin1String( "theMapCanvas" ) )
    {
      foundTheMapCanvas = true;
      mMapSettings.readXml( node );

      if ( !qgsDoubleNear( mMapSettings.rotation(), 0 ) )
        QgsMessageLog::logMessage( tr( "Map Canvas rotation is not supported. Resetting from %1 to 0." ).arg( mMapSettings.rotation() ) );
    }
  }
  if ( !foundTheMapCanvas )
  {
    mMapSettings.setDestinationCrs( mProject->crs() );
    mMapSettings.setExtent( mProject->viewSettings()->fullExtent() );
  }

  mMapSettings.setRotation( 0 );

  mMapSettings.setTransformContext( mProject->transformContext() );
  mMapSettings.setPathResolver( mProject->pathResolver() );
  mMapSettings.setElevationShadingRenderer( mProject->elevationShadingRenderer() );

  emit extentChanged();
  emit destinationCrsChanged();
  emit outputSizeChanged();
  emit outputDpiChanged();
  emit layersChanged();
  emit temporalStateChanged();
  emit zRangeChanged();
}

double QgsQuickMapSettings::rotation() const
{
  return mMapSettings.rotation();
}

void QgsQuickMapSettings::setRotation( double rotation )
{
  if ( !qgsDoubleNear( rotation, 0 ) )
    QgsMessageLog::logMessage( tr( "Map Canvas rotation is not supported. Resetting from %1 to 0." ).arg( rotation ) );
}

QColor QgsQuickMapSettings::backgroundColor() const
{
  return mMapSettings.backgroundColor();
}

void QgsQuickMapSettings::setBackgroundColor( const QColor &color )
{
  if ( mMapSettings.backgroundColor() == color )
    return;

  mMapSettings.setBackgroundColor( color );
  emit backgroundColorChanged();
}

qreal QgsQuickMapSettings::devicePixelRatio() const
{
  return mDevicePixelRatio;
}

void QgsQuickMapSettings::setDevicePixelRatio( const qreal &devicePixelRatio )
{
  mDevicePixelRatio = devicePixelRatio;
  emit devicePixelRatioChanged();
}

bool QgsQuickMapSettings::isTemporal() const
{
  return mMapSettings.isTemporal();
}

void QgsQuickMapSettings::setIsTemporal( bool temporal )
{
  mMapSettings.setIsTemporal( temporal );
  emit temporalStateChanged();
}

QDateTime QgsQuickMapSettings::temporalBegin() const
{
  return mMapSettings.temporalRange().begin();
}

void QgsQuickMapSettings::setTemporalBegin( const QDateTime &begin )
{
  const QgsDateTimeRange range = mMapSettings.temporalRange();
  mMapSettings.setTemporalRange( QgsDateTimeRange( begin, range.end() ) );
  emit temporalStateChanged();
}

QDateTime QgsQuickMapSettings::temporalEnd() const
{
  return mMapSettings.temporalRange().end();
}

void QgsQuickMapSettings::setTemporalEnd( const QDateTime &end )
{
  const QgsDateTimeRange range = mMapSettings.temporalRange();
  mMapSettings.setTemporalRange( QgsDateTimeRange( range.begin(), end ) );
  emit temporalStateChanged();
}

double QgsQuickMapSettings::zRangeLower() const
{
  const QgsDoubleRange zRange = mMapSettings.zRange();
  return zRange.lower();
}

void QgsQuickMapSettings::setZRangeLower( const double &lower )
{
  const QgsDoubleRange zRange = mMapSettings.zRange();
  if ( zRange.lower() == lower )
  {
    return;
  }

  mMapSettings.setZRange( QgsDoubleRange( lower, zRange.upper(), zRange.includeLower(), zRange.includeUpper() ) );
  emit zRangeChanged();
}

double QgsQuickMapSettings::zRangeUpper() const
{
  const QgsDoubleRange zRange = mMapSettings.zRange();
  return zRange.upper();
}

void QgsQuickMapSettings::setZRangeUpper( const double &upper )
{
  const QgsDoubleRange zRange = mMapSettings.zRange();
  if ( zRange.upper() == upper )
  {
    return;
  }

  mMapSettings.setZRange( QgsDoubleRange( zRange.lower(), upper, zRange.includeLower(), zRange.includeUpper() ) );
  emit zRangeChanged();
}
