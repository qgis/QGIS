/***************************************************************************
   qgsstatusbarcoordinateswidget.cpp
    --------------------------------------
   Date                 : 05.08.2015
   Copyright            : (C) 2015 Denis Rouzaud
   Email                : denis.rouzaud@gmail.com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include <QFont>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QSpacerItem>
#include <QTimer>
#include <QToolButton>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QRandomGenerator>

#include "qgsstatusbarcoordinateswidget.h"
#include "qgsapplication.h"
#include "qgsmapcanvas.h"
#include "qgsproject.h"
#include "qgscoordinateutils.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayerjoininfo.h"
#include "qgscoordinatereferencesystemutils.h"

QgsStatusBarCoordinatesWidget::QgsStatusBarCoordinatesWidget( QWidget *parent )
  : QWidget( parent )
  , mMousePrecisionDecimalPlaces( 0 )
{
  // calculate the size of two chars
  mTwoCharSize = fontMetrics().boundingRect( 'O' ).width();
  mMinimumWidth = mTwoCharSize * 4;

  // add a label to show current position
  mLabel = new QLabel( QString(), this );
  mLabel->setObjectName( QStringLiteral( "mCoordsLabel" ) );
  mLabel->setMinimumWidth( 10 );
  //mCoordsLabel->setMaximumHeight( 20 );
  mLabel->setMargin( 3 );
  mLabel->setAlignment( Qt::AlignCenter );
  mLabel->setFrameStyle( QFrame::NoFrame );
  mLabel->setText( tr( "Projected" ) );
  mLabel->setToolTip( tr( "Current map coordinate" ) );

  // add a label to show DSM GR
  mLabelgeocord = new QLabel(QString(), this);
  mLabelgeocord->setObjectName(QStringLiteral("mCoordsGeocord"));
  mLabelgeocord->setMinimumWidth(10);
  //mLabelgeocord->setMaximumHeight( 20 );
  mLabelgeocord->setMargin(3);
  mLabelgeocord->setAlignment(Qt::AlignCenter);
  mLabelgeocord->setFrameStyle(QFrame::NoFrame);
  mLabelgeocord->setText(tr("Geographic"));
  mLabelgeocord->setToolTip(tr("Show Geographic Coordinate"));

  // add a label to show DSM GR
  mLabeldgr = new QLabel( QString(), this );
  mLabeldgr->setObjectName( QStringLiteral( "mCoordsLabeldgr" ) );
  mLabeldgr->setMinimumWidth( 10 );
  //mLabeldgr->setMaximumHeight( 20 );
  mLabeldgr->setMargin( 3 );
  mLabeldgr->setAlignment( Qt::AlignCenter );
  mLabeldgr->setFrameStyle( QFrame::NoFrame );
  mLabeldgr->setText( tr( "DSM GR" ) );
  mLabeldgr->setToolTip( tr( "Show DSM GR" ) );

// add a label to show DSM Sheet No
  mLabeldsheet = new QLabel( QString(), this );
  mLabeldsheet->setObjectName( QStringLiteral( "mCoordsLabeldsheet" ) );
  mLabeldsheet->setMinimumWidth( 10 );
  //mLabeldsheet->setMaximumHeight( 20 );
  mLabeldsheet->setMargin( 3 );
  mLabeldsheet->setAlignment( Qt::AlignCenter );
  mLabeldsheet->setFrameStyle( QFrame::NoFrame );
  mLabeldsheet->setText( tr( "DSM No." ) );
  mLabeldsheet->setToolTip( tr( "Show DSM Sheet No" ) );
  
  // add a label to show ESM GR
  mLabelegr = new QLabel( QString(), this );
  mLabelegr->setObjectName( QStringLiteral( "mCoordsLabelegr" ) );
  mLabelegr->setMinimumWidth( 10 );
  //mLabelegr->setMaximumHeight( 20 );
  mLabelegr->setMargin( 3 );
  mLabelegr->setAlignment( Qt::AlignCenter );
  mLabelegr->setFrameStyle( QFrame::NoFrame );
  mLabelegr->setText( tr( "ESM GR" ) );
  mLabelegr->setToolTip( tr( "Show ESM GR" ) );
  
  // add a label to show DSM Sheet No
  mLabelesheet = new QLabel( QString(), this );
  mLabelesheet->setObjectName( QStringLiteral( "mCoordsLabelesheet" ) );
  mLabelesheet->setMinimumWidth( 10 );
  //mLabelesheet->setMaximumHeight( 20 );
  mLabelesheet->setMargin( 3 );
  mLabelesheet->setAlignment( Qt::AlignCenter );
  mLabelesheet->setFrameStyle( QFrame::NoFrame );
  mLabelesheet->setText( tr( "ESM No." ) );
  mLabelesheet->setToolTip( tr( "Show ESM Sheet No" ) );
  
  mLineEdit = new QLineEdit( this );
  mLineEdit->setMinimumWidth( 10 );
  //mLineEdit->setMaximumHeight( 20 );
  mLineEdit->setContentsMargins( 0, 0, 0, 0 );
  mLineEdit->setAlignment( Qt::AlignCenter );
  connect( mLineEdit, &QLineEdit::returnPressed, this, &QgsStatusBarCoordinatesWidget::validateCoordinates );

  const QRegularExpression coordValidator( "[+-]?\\d+\\.?\\d*\\s*,\\s*[+-]?\\d+\\.?\\d*" );
  mCoordsEditValidator = new QRegularExpressionValidator( coordValidator, this );
  mLineEdit->setToolTip( tr( "Current map coordinate (longitude,latitude or east,north)" ) );

  //toggle to switch between mouse pos and extents display in status bar widget
  mToggleExtentsViewButton = new QToolButton( this );
  mToggleExtentsViewButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "tracking.svg" ) ) );
  mToggleExtentsViewButton->setToolTip( tr( "Toggle extents and mouse position display" ) );
  mToggleExtentsViewButton->setCheckable( true );
  mToggleExtentsViewButton->setAutoRaise( true );
  connect( mToggleExtentsViewButton, &QAbstractButton::toggled, this, &QgsStatusBarCoordinatesWidget::extentsViewToggled );

//Nihcas below
  //Coordinate status bar widget GeoCoord
  mCoordsGeocord = new QLineEdit(this);
  mCoordsGeocord->setObjectName("mCoordsGeocord");
  mCoordsGeocord->setMinimumWidth(10);
  //mCoordsGeocord->setMaximumWidth(150);
  mCoordsGeocord->setContentsMargins(0, 0, 0, 0);
  mCoordsGeocord->setAlignment(Qt::AlignLeft);
  //mCoordsGeocordValidator = new QRegularExpressionValidator(coordValidator, mCoordsGeocord);
  mCoordsGeocord->setWhatsThis(tr("Shows Geographic coordinates at the current cursor position."));
  mCoordsGeocord->setToolTip(tr("Shows the Geographic Coordinates"));
  connect(mCoordsGeocord, &QLineEdit::returnPressed, this, &QgsStatusBarCoordinatesWidget::validateCoordinates);

  //Coordinate status bar widget DSM
  mCoordsEditMgrid = new QLineEdit(this);
  mCoordsEditMgrid->setObjectName("mCoordsEditMgrid");
  mCoordsEditMgrid->setMinimumWidth(110);
  mCoordsEditMgrid->setMaximumWidth(150);
  mCoordsEditMgrid->setContentsMargins(0, 0, 0, 0);
  mCoordsEditMgrid->setAlignment(Qt::AlignLeft);
  mCoordsEditMgridValidator = new QRegularExpressionValidator(coordValidator, mCoordsEditMgrid);
  mCoordsEditMgrid->setWhatsThis(tr("Shows DSM GR at the current cursor position."));
  mCoordsEditMgrid->setToolTip(tr("Shows the Mil grid of DSM series maps"));
  connect(mCoordsEditMgrid, &QLineEdit::returnPressed, this, &QgsStatusBarCoordinatesWidget::validateCoordinates);
  
  //DSM Sheet Number widget 
  mCoordsEditMsheet = new QLineEdit(this);
  mCoordsEditMsheet->setObjectName("mCoordsEditMsheet");
  mCoordsEditMsheet->setMinimumWidth(10);
  mCoordsEditMsheet->setMaximumWidth(50);
  mCoordsEditMsheet->setContentsMargins(0, 0, 0, 0);
  mCoordsEditMsheet->setAlignment(Qt::AlignLeft);
  mCoordsEditMgridValidator = new QRegularExpressionValidator(coordValidator, mCoordsEditMsheet);
  mCoordsEditMsheet->setWhatsThis(tr("Shows the DSM Sheet No."));
  mCoordsEditMsheet->setToolTip(tr("Shows the DSM series Map Sheet No."));  
  connect(mCoordsEditMsheet, &QLineEdit::returnPressed, this, &QgsStatusBarCoordinatesWidget::validateCoordinates);
  
  
  //Cordinate status bar widget Everest
  mCoordsEditMgrideve = new QLineEdit(this);
  mCoordsEditMgrideve->setObjectName("mCoordsEditMgrideve");
  mCoordsEditMgrideve->setMinimumWidth(150);
  mCoordsEditMgrideve->setMaximumWidth(150);
  mCoordsEditMgrideve->setContentsMargins(0, 0, 0, 0);
  mCoordsEditMgrideve->setAlignment(Qt::AlignLeft);
  mCoordsEditMgrideveValidator = new QRegularExpressionValidator(coordValidator, mCoordsEditMgrideve);
  mCoordsEditMgrideve->setWhatsThis(tr("Shows ESM GR at the current cursor position"));
  mCoordsEditMgrideve->setToolTip(tr("Show the ESM GR of Everest series maps)"));
  connect(mCoordsEditMgrideve, &QLineEdit::returnPressed, this, &QgsStatusBarCoordinatesWidget::validateCoordinates);
  //Nihcas above
  
  //Everest Series Map Sheet Number 
  mCoordsEditMsheeteve = new QLineEdit(this);
  mCoordsEditMsheeteve->setObjectName("mCoordsEditMsheeteve");
  mCoordsEditMsheeteve->setMinimumWidth(6);
  mCoordsEditMsheeteve->setMaximumWidth(50);
  mCoordsEditMsheeteve->setContentsMargins(0, 0, 0, 0);
  mCoordsEditMsheeteve->setAlignment(Qt::AlignLeft);
  mCoordsEditMgrideveValidator = new QRegularExpressionValidator(coordValidator, mCoordsEditMsheeteve);
  mCoordsEditMsheeteve->setWhatsThis(tr("Shows ESM Sheet No."));
  mCoordsEditMsheeteve->setToolTip(tr("Shows the ESM series Map Sheet No."));
  connect(mCoordsEditMsheeteve, &QLineEdit::returnPressed, this, &QgsStatusBarCoordinatesWidget::validateCoordinates);
  

  QHBoxLayout *layout = new QHBoxLayout( this );
  setLayout( layout );
  layout->addItem( new QSpacerItem( 0, 0, QSizePolicy::Expanding ) );
  layout->addWidget( mLabel );
  layout->addWidget( mLineEdit );
  //Nihcas add below
  layout->addWidget(mLabelgeocord);
  layout->addWidget(mCoordsGeocord);
  layout->addWidget( mLabeldgr );
  layout->addWidget(mCoordsEditMgrid);  
  layout->addWidget( mLabeldsheet );
  layout->addWidget(mCoordsEditMsheet);
  layout->addWidget( mLabelegr );
  layout->addWidget(mCoordsEditMgrideve);
  layout->addWidget( mLabelesheet );
  layout->addWidget(mCoordsEditMsheeteve);
  //Nihcas add above
  layout->addWidget( mToggleExtentsViewButton );
  layout->setContentsMargins( 0, 0, 0, 0 );
  layout->setAlignment( Qt::AlignLeft );
  layout->setSpacing( 0 );

  // When you feel dizzy
  mDizzyTimer = new QTimer( this );
  connect( mDizzyTimer, &QTimer::timeout, this, &QgsStatusBarCoordinatesWidget::dizzy );

  connect( QgsProject::instance()->displaySettings(), &QgsProjectDisplaySettings::coordinateCrsChanged, this, &QgsStatusBarCoordinatesWidget::coordinateDisplaySettingsChanged );
  connect( QgsProject::instance()->displaySettings(), &QgsProjectDisplaySettings::geographicCoordinateFormatChanged, this, &QgsStatusBarCoordinatesWidget::coordinateDisplaySettingsChanged );
  connect( QgsProject::instance()->displaySettings(), &QgsProjectDisplaySettings::coordinateTypeChanged, this, &QgsStatusBarCoordinatesWidget::coordinateDisplaySettingsChanged );

  coordinateDisplaySettingsChanged();
}

void QgsStatusBarCoordinatesWidget::setMapCanvas( QgsMapCanvas *mapCanvas )
{
  if ( mMapCanvas )
  {
    disconnect( mMapCanvas, &QgsMapCanvas::xyCoordinates, this, &QgsStatusBarCoordinatesWidget::showMouseCoordinates );
    disconnect( mMapCanvas, &QgsMapCanvas::extentsChanged, this, &QgsStatusBarCoordinatesWidget::showExtent );
  }

  mMapCanvas = mapCanvas;
  connect( mMapCanvas, &QgsMapCanvas::xyCoordinates, this, &QgsStatusBarCoordinatesWidget::showMouseCoordinates );
  connect( mMapCanvas, &QgsMapCanvas::extentsChanged, this, &QgsStatusBarCoordinatesWidget::showExtent );
}

void QgsStatusBarCoordinatesWidget::setFont( const QFont &myFont )
{
  mLineEdit->setFont( myFont );
  mLabel->setFont( myFont );
}

void QgsStatusBarCoordinatesWidget::setMouseCoordinatesPrecision( unsigned int precision )
{
  mMousePrecisionDecimalPlaces = precision;
}

void QgsStatusBarCoordinatesWidget::validateCoordinates()
{
  if ( !mMapCanvas )
  {
    return;
  }
  else if ( mLineEdit->text() == QLatin1String( "world" ) )
  {
    world();
  }
  if ( mLineEdit->text() == QLatin1String( "contributors" ) )
  {
    contributors();
  }
  else if ( mLineEdit->text() == QLatin1String( "hackfests" ) )
  {
    hackfests();
  }
  else if ( mLineEdit->text() == QLatin1String( "user groups" ) )
  {
    userGroups();
  }
  else if ( mLineEdit->text() == QLatin1String( "dizzy" ) )
  {
    // sometimes you may feel a bit dizzy...
    if ( mDizzyTimer->isActive() )
    {
      mDizzyTimer->stop();
      mMapCanvas->setSceneRect( mMapCanvas->viewport()->rect() );
      mMapCanvas->setTransform( QTransform() );
    }
    else
    {
      mDizzyTimer->start( 100 );
    }
    return;
  }
  else if ( mLineEdit->text() == QLatin1String( "retro" ) )
  {
    mMapCanvas->setProperty( "retro", !mMapCanvas->property( "retro" ).toBool() );
    refreshMapCanvas();
    return;
  }
  else if ( mLineEdit->text() == QLatin1String( "bored" ) )
  {
    // it's friday afternoon and too late to start another piece of work...
    emit weAreBored();
  }

  bool xOk = false;
  bool  yOk = false;
  double first = 0;
  double second = 0;
  QString coordText = mLineEdit->text();
  const thread_local QRegularExpression sMultipleWhitespaceRx( QStringLiteral( " {2,}" ) );
  coordText.replace( sMultipleWhitespaceRx, QStringLiteral( " " ) );

  QStringList parts = coordText.split( ',' );
  if ( parts.size() == 2 )
  {
    first = parts.at( 0 ).toDouble( &xOk );
    second = parts.at( 1 ).toDouble( &yOk );
  }

  if ( !xOk || !yOk )
  {
    parts = coordText.split( ' ' );
    if ( parts.size() == 2 )
    {
      first = parts.at( 0 ).toDouble( &xOk );
      second = parts.at( 1 ).toDouble( &yOk );
    }
  }

  if ( !xOk || !yOk )
    return;

  const Qgis::CoordinateOrder projectAxisOrder = QgsProject::instance()->displaySettings()->coordinateAxisOrder();

  const Qgis::CoordinateOrder coordinateOrder = projectAxisOrder == Qgis::CoordinateOrder::Default ? QgsCoordinateReferenceSystemUtils::defaultCoordinateOrderForCrs( mMapCanvas->mapSettings().destinationCrs() ) : projectAxisOrder;
  // we may need to flip coordinates depending on crs axis ordering
  switch ( coordinateOrder )
  {
    case Qgis::CoordinateOrder::Default:
    case Qgis::CoordinateOrder::XY:
      mMapCanvas->setCenter( QgsPointXY( first, second ) );
      break;
    case Qgis::CoordinateOrder::YX:
      mMapCanvas->setCenter( QgsPointXY( second, first ) );
      break;
  }

  mMapCanvas->refresh();
}


void QgsStatusBarCoordinatesWidget::dizzy()
{
  if ( !mMapCanvas )
  {
    return;
  }
  // constants should go to options so that people can customize them to their taste
  const int d = 10; // max. translational dizziness offset
  const int r = 4;  // max. rotational dizzines angle
  QRectF rect = mMapCanvas->sceneRect();
  if ( rect.x() < -d || rect.x() > d || rect.y() < -d || rect.y() > d )
    return; // do not affect panning

  rect.moveTo( static_cast< int >( QRandomGenerator::global()->generate() % ( 2 * d ) ) - d, static_cast< int >( QRandomGenerator::global()->generate() % ( 2 * d ) ) - d );
  mMapCanvas->setSceneRect( rect );
  QTransform matrix;
  matrix.rotate( static_cast<int >( QRandomGenerator::global()->generate() % ( 2 * r ) ) - r );
  mMapCanvas->setTransform( matrix );
}

void QgsStatusBarCoordinatesWidget::contributors()
{
  if ( !mMapCanvas )
  {
    return;
  }
  const QString fileName = QgsApplication::pkgDataPath() + QStringLiteral( "/resources/data/contributors.json" );
  const QFileInfo fileInfo = QFileInfo( fileName );
  const QgsVectorLayer::LayerOptions options { QgsProject::instance()->transformContext() };
  QgsVectorLayer *layer = new QgsVectorLayer( fileInfo.absoluteFilePath(),
      tr( "QGIS Contributors" ), QStringLiteral( "ogr" ), options );
  // Register this layer with the layers registry
  QgsProject::instance()->addMapLayer( layer );
  layer->setAutoRefreshInterval( 500 );
  layer->setAutoRefreshEnabled( true );
}

void QgsStatusBarCoordinatesWidget::world()
{
  if ( !mMapCanvas )
  {
    return;
  }
  const QString fileName = QgsApplication::pkgDataPath() + QStringLiteral( "/resources/data/world_map.gpkg|layername=countries" );
  const QFileInfo fileInfo = QFileInfo( fileName );
  QgsVectorLayer::LayerOptions options { QgsProject::instance()->transformContext() };
  options.forceReadOnly = true;
  QgsVectorLayer *layer = new QgsVectorLayer( fileInfo.absoluteFilePath(),
      tr( "World Map" ), QStringLiteral( "ogr" ), options );
  // Register this layer with the layers registry
  QgsProject::instance()->addMapLayer( layer );
}

void QgsStatusBarCoordinatesWidget::hackfests()
{
  if ( !mMapCanvas )
  {
    return;
  }
  const QString fileName = QgsApplication::pkgDataPath() + QStringLiteral( "/resources/data/qgis-hackfests.json" );
  const QFileInfo fileInfo = QFileInfo( fileName );
  const QgsVectorLayer::LayerOptions options { QgsProject::instance()->transformContext() };
  QgsVectorLayer *layer = new QgsVectorLayer( fileInfo.absoluteFilePath(),
      tr( "QGIS Hackfests" ), QStringLiteral( "ogr" ), options );
  // Register this layer with the layers registry
  QgsProject::instance()->addMapLayer( layer );
  layer->setAutoRefreshInterval( 500 );
  layer->setAutoRefreshEnabled( true );
}

void QgsStatusBarCoordinatesWidget::userGroups()
{
  if ( !mMapCanvas )
  {
    return;
  }
  const QString fileName = QgsApplication::pkgDataPath() + QStringLiteral( "/resources/data/world_map.gpkg|layername=countries" );
  const QFileInfo fileInfo = QFileInfo( fileName );
  const QgsVectorLayer::LayerOptions options { QgsProject::instance()->transformContext() };
  QgsVectorLayer *layer = new QgsVectorLayer( fileInfo.absoluteFilePath(),
      tr( "User Groups" ), QStringLiteral( "ogr" ), options );

  const QString fileNameData = QgsApplication::pkgDataPath() + QStringLiteral( "/resources/data/user_groups_data.json" );
  const QFileInfo fileInfoData = QFileInfo( fileNameData );
  QgsVectorLayer *layerData = new QgsVectorLayer( fileInfoData.absoluteFilePath(),
      tr( "user_groups_data" ), QStringLiteral( "ogr" ), options );

  // Register layers with the layers registry
  QgsProject::instance()->addMapLayers( QList<QgsMapLayer *>() << layer << layerData );

  // Create join
  QgsVectorLayerJoinInfo joinInfo;
  joinInfo.setTargetFieldName( QStringLiteral( "iso_a2" ) );
  joinInfo.setJoinLayer( layerData );
  joinInfo.setJoinFieldName( QStringLiteral( "country" ) );
  joinInfo.setUsingMemoryCache( true );
  joinInfo.setPrefix( QStringLiteral( "ug_" ) );
  joinInfo.setJoinFieldNamesSubset( nullptr );  // Use all join fields
  layer->addJoin( joinInfo );

  // Load QML for polygon symbology and maptips
  const QString fileNameStyle = QgsApplication::pkgDataPath() + QStringLiteral( "/resources/data/user_groups.qml" );
  bool styleFlag = false;
  layer->loadNamedStyle( fileNameStyle, styleFlag, true );
}

void QgsStatusBarCoordinatesWidget::extentsViewToggled( bool flag )
{
  if ( flag )
  {
    //extents view mode!
    mToggleExtentsViewButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "extents.svg" ) ) );
    mLineEdit->setToolTip( tr( "Map coordinates for the current view extents" ) );
    mLineEdit->setReadOnly( true );
    showExtent();
  }
  else
  {
    //mouse cursor pos view mode!
    mToggleExtentsViewButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "tracking.svg" ) ) );
    mLineEdit->setToolTip( tr( "Map coordinates at mouse cursor position" ) );
    mLineEdit->setReadOnly( false );
    mLabel->setText( tr( "Coordinate" ) );
  }
}

void QgsStatusBarCoordinatesWidget::refreshMapCanvas()
{
  if ( !mMapCanvas )
    return;

  //stop any current rendering
  mMapCanvas->stopRendering();
  mMapCanvas->redrawAllLayers();
}

void QgsStatusBarCoordinatesWidget::showMouseCoordinates( const QgsPointXY &Qp )
{
  mLastCoordinate = Qp;  
  //updateCoordinateDisplay();
  //updateCoordinateDisplayUpdated(p);

  
  //Nihcas added below

  QgsPointXY p;
  QgsPointXY p1 = QgsPointXY(Qp);
  QgsCoordinateReferenceSystem crsSrc, crsWgs;
  crsSrc = mMapCanvas->mapSettings().destinationCrs();
  if (crsSrc.authid() != "EPSG:4326")
  {
    crsWgs = QgsCoordinateReferenceSystem(4326);
    QgsCoordinateTransform xform = QgsCoordinateTransform(crsSrc, crsWgs, QgsProject::instance());
    p = xform.transform(p1);
    //QString qs = p.toString();
  
    mCoordsGeocord->setText(QgsCoordinateUtils::formatCoordinateForProject(QgsProject::instance(), p, crsSrc, 4));  //precision for latitude and longitude to show on screen
    ensureCoordinatesVisible();
  }
  else
  {
    p = Qp;
    mLineEdit->setText(QgsCoordinateUtils::formatCoordinateForProject(QgsProject::instance(), p, mMapCanvas->mapSettings().destinationCrs(),
      mMousePrecisionDecimalPlaces));
    crsWgs = QgsCoordinateReferenceSystem(4326);
    QgsCoordinateTransform xform = QgsCoordinateTransform(crsSrc, crsWgs, QgsProject::instance());
    p = xform.transform(p1);
    QString qs = p.toString();
    mCoordsGeocord->setText(QgsCoordinateUtils::formatCoordinateForProject(QgsProject::instance(), p, crsSrc, 4));  //precision for latitude and longitude to show on screen
    ensureCoordinatesVisible();

  }

//Nihcas added below
//Mil Grid Display for dsm series map 

  QString str, str1, str2, str3;
  if ((p.x() > -180 && p.x() < 180) && (p.y() > -90 && p.y() < 90))
  {
    str = LatLongToMilgridConversion(p);
  }
  else
  {
    str = "OUT OF BOUND AREA";
  }

  if (((p.x() > -180 && p.x() < 180)) && (p.y() > -90 && p.y() < 90))
  {
    str1 = LatLongTopoSheetConversion(p);
  }
  else
  {
    str1 = "OUT OF BOUND AREA";
  }

  //for everest series map
  if ((p.x() > 57 && p.x() < 110) && (p.y() > 8 && p.y() < 44))
  {
    str2 = eveLatLongToMilgridConversion(p);
  }
  else
  {
    str2 = "OUT OF BOUND AREA";
  }

  if (((p.x() > 44 && p.x() < 104)) && (p.y() > 4 && p.y() < 40))
  {
    str3 = eveLatLongTopoSheetConversion(p);
  }
  else
  {
    str3 = "NOT AVAILABLE";
  }

  //To display values

  mCoordsEditMgrid->setText(str); //Display DSM GR
  mCoordsEditMsheet->setText(str1); //Display DSM Sheet No
  mCoordsEditMgrideve->setText(str2); //Display ESM GR
  mCoordsEditMsheeteve->setText(str3); //Display ESM Sheet No
  //mCoordsGeocord->setText(" T");
  updateCoordinateDisplay();
  
//Nihcas above 
}


void QgsStatusBarCoordinatesWidget::showExtent()
{
  if ( !mToggleExtentsViewButton->isChecked() )
  {
    return;
  }

  mLabel->setText( tr( "Extents" ) );
  mLineEdit->setText( QgsCoordinateUtils::formatExtentForProject( QgsProject::instance(), mMapCanvas->extent(), mMapCanvas->mapSettings().destinationCrs(),
                      mMousePrecisionDecimalPlaces ) );
  ensureCoordinatesVisible();
}

void QgsStatusBarCoordinatesWidget::ensureCoordinatesVisible()
{

  //ensure the label is big (and small) enough
  const int width = std::max( mLineEdit->fontMetrics().boundingRect( mLineEdit->text() ).width() + 16, mMinimumWidth );
  if ( mLineEdit->minimumWidth() < width || ( mLineEdit->minimumWidth() - width ) > mTwoCharSize )
  {
    mLineEdit->setMinimumWidth( width );
    mLineEdit->setMaximumWidth( width );
  }
}

//Nihcas below Function for Mil Grid
QString QgsStatusBarCoordinatesWidget::eveLatLongTopoSheetConversion(const QgsPointXY &mp)
{
  double LAT = mp.y();
  double LONG = mp.x();
  double LATdiff, LONGdiff;
  int x, y;
  QChar arr[10];
  QString finalarr;
  //int lat_ndsm = ceil(mp.y());												//take the upper value and integer of latitude
  //float lat_ndsm1 = mp.y();													//take floating value of latitude
  //int long_ndsm = mp.x();	
  char str_temp1;
  char str_temp2;
  char str_temp3;
  char str_temp4;
  int lat_esm = ceil(LAT);								//take the upper value and integer of latitude
  double lat_esm1 = LAT;									//take floating value of latitude
  int long_esm = LONG;									//take integer value of longitude
  double long_esm1 = LONG - 44;							//take floating value of latitude
  int lat1;													//a temporary variable
  int matrix1[9][15];
  QChar matrix2[4][4] = { { 'D','H','L','P' },{ 'C','G','K','O' },{ 'B','F','J','N' },{ 'A','E','I','M' } };
  int matrix3[4][4] = { { 4, 8, 12, 16 },{ 3, 7, 11, 15 },{ 2, 6, 10, 14 },{ 1, 5, 9, 13 } };
  lat_esm = lat_esm - 4;
  long_esm = long_esm - 44;
  int i = 0;
  //the below for loop initializes matrix1 array...the if between for loop eliminates some numbers that are not on map
  for (int col = 0; col < 15; col++) {				      //for initializing m1 array
    for (int row = 8; row >= 0; row--) {
      if ((col == 0 && (row == 1 || row == 0)) || (col == 1 && (row == 1 || row == 0)) || (col == 2 && (row == 1 || row == 0)) || (col == 3 && (row == 2 || row == 1 || row == 0)) || (col == 4 && (row == 4 || row == 3 || row == 2 || row == 1 || row == 0)) || (col == 5 && (row == 3 || row == 2 || row == 1 || row == 0)) || (col == 6 && (row == 3 || row == 2 || row == 1 || row == 0)) || (col == 10 && (row == 2 || row == 1 || row == 0)) || (col == 11 && (row == 3 || row == 2 || row == 1 || row == 0))) {
      }
      else {
        i++;
        matrix1[row][col] = i;
      }
    }
  }
  if ((lat_esm % 4) == 0) {
    lat1 = int(lat_esm / 4);							//division by 4 is done as latitude varies by 4 degree
    lat1 = lat1 - 1;								//if boubndary value is presented like 16 degree or 32 degree then 1 will be subtracted from it to make it of lower grid
    if (lat1 < 0) {									//value cannot be less than 0 hence this check is for that
      lat1 = 0;
    }
  }
  else {											//if a value that is not divisible by 4 is presented this else will work
    lat1 = int(lat_esm / 4);
  }
  int long1 = int(long_esm / 4);
  int first = matrix1[lat1][long1];			       //Value at m1(lat1,long1)  will be the toposheet number for level I
  int lat2 = int(lat_esm - (lat1) * 4);
  lat2 = lat2 - 1;
  if (lat2 < 0) {
    lat2 = 0;
  }
  int long2 = int(long_esm - (long1) * 4);
  QChar second = matrix2[lat2][long2];			//Value at m2(lat2,long2) will be the character,for level II toposheet division
                          /*int lat3 = int((lat_esm1 - int(lat_esm1))*4);
                          lat3 = lat3 - 1;
                          if(lat3 < 0 || lat3 == 0) {
                          lat3 = 3;
                          }
                          int long3 = int((long_esm1 - int(long_esm))*4);
                          int third = matrix3[lat3][long3];			//Value at m3(lat3,long3) will be the character,for level III toposheet division
                          //int *pointer = arr_esm;						//a pointer is assigned to an array...this pointer will be returned
                          //arr_esm[0] = first;							//this stores value of toposheet number for level I
                          //arr_esm[1] = second;						//this stores value of toposheet number for level II
                          //arr_esm[2] = third;							//this stores value of toposheet number for level III
                          */
  LATdiff = LAT - floor(LAT);
  if ((0.0 <= (float)(4 * LATdiff)) && ((float)(4 * LATdiff) <= 1.0)) 	y = 0;
  if ((1.0 <= (float)(4 * LATdiff)) && ((float)(4 * LATdiff) <= 2.0))	    y = 1;
  if ((2.0 <= (float)(4 * LATdiff)) && ((float)(4 * LATdiff) <= 3.0))	    y = 2;
  if ((3.0 <= (float)(4 * LATdiff)) && ((float)(4 * LATdiff) <= 4.0))	    y = 3;
  LONGdiff = LONG - floor(LONG);
  if ((0.0 <= (float)(4 * LONGdiff)) && ((float)(4 * LONGdiff) <= 1.0)) 	 x = 0;
  if ((1.0 < (float)(4 * LONGdiff)) && ((float)(4 * LONGdiff) <= 2.0))     x = 1;
  if ((2.0 < (float)(4 * LONGdiff)) && ((float)(4 * LONGdiff) <= 3.0))	 x = 2;
  if ((3.0 < (float)(4 * LONGdiff)) && ((float)(4 * LONGdiff) <= 4.0))	 x = 3;
  int third = matrix3[y][x];
  str_temp1 = (char)(((int)'0') + ((first / 10) % 10));
  str_temp2 = (char)(((int)'0') + (first % 10));  // Note:: putting (char) converts the righte side ascii value to a valid alphanumeric character;
  str_temp3 = (char)(((int)'0') + ((third / 10) % 10));
  str_temp4 = (char)(((int)'0') + ((third) % 10));
  arr[0] = str_temp1;
  arr[1] = str_temp2;
  arr[2] = second;
  arr[3] = str_temp3;
  arr[4] = str_temp4;
  for (int i = 0; i < 10; i++)
  {
    finalarr[i] = arr[i];
  }
  return finalarr;								//a pointer to an array is returned
}
inline QString QgsStatusBarCoordinatesWidget::LatLongTopoSheetConversion(const QgsPointXY &mp)
{
  double LAT = mp.y();
  double LONG = mp.x();
  double LATdiff, LONGdiff;
  int x, y, longskc;
  int lat_ndsm = ceil(mp.y());												//take the upper value and integer of latitude
  float lat_ndsm1 = mp.y();													//take floating value of latitude
  int long_ndsm = mp.x();													//take integer value of longitude
  float long_ndsm1 = (mp.x() - 60);										//take floating value of latitude
  int lat1 = 0, long1 = 0;	//a temporary variable
  QChar temp[2];
  QChar arr[10];
  QString arr_ndsm;
  QChar matrix2[4][6] = { { 'S','T','U','V','W','X' },{ 'M','N','O','P','Q','R' },{ 'G','H','I','J','K','L' },{ 'A','B','C','D','E','F' } };
  int matrix3[4][4] = { { 4, 8, 12, 16 },{ 3, 7, 11, 15 },{ 2, 6, 10, 14 },{ 1, 5, 9, 13 } };
//NEW SERIES ADDED
  QChar matrix1[46] = { 'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W' };
  int matrix1_part1[61] = { 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61 };
  //QChar matrix1[14] = { 'A','B','C','D','E','F','G','H','I','J','K','L','M','N' };
  //int matrix1_part1[17] = { 37,38,39,40,41, 42, 43, 44, 45, 46, 47,48,49,50,51,51,52 };
  char str_temp1;
  char str_temp2;
  char str_temp3;
  char str_temp4;
  lat_ndsm = lat_ndsm - 0;										//map starts from 0 degree latitude
  lat_ndsm1 = lat_ndsm1 - 0;
  //long_ndsm = long_ndsm - 36;										//map starts from 36 degree longitude
  long_ndsm = long_ndsm - 0;
  LATdiff = LAT - floor(LAT);
  if ((0.0 <= (float)(4 * LATdiff)) && ((float)(4 * LATdiff) <= 1.0)) 	y = 0;
  if ((1.0 <= (float)(4 * LATdiff)) && ((float)(4 * LATdiff) <= 2.0))	    y = 1;
  if ((2.0 <= (float)(4 * LATdiff)) && ((float)(4 * LATdiff) <= 3.0))	    y = 2;
  if ((3.0 <= (float)(4 * LATdiff)) && ((float)(4 * LATdiff) <= 4.0))	    y = 3;
  LONGdiff = LONG - floor(LONG);
  if ((0.0 <= (float)(4 * LONGdiff)) && ((float)(4 * LONGdiff) <= 1.0)) 	 x = 0;
  if ((1.0 < (float)(4 * LONGdiff)) && ((float)(4 * LONGdiff) <= 2.0))     x = 1;
  if ((2.0 < (float)(4 * LONGdiff)) && ((float)(4 * LONGdiff) <= 3.0))	 x = 2;
  if ((3.0 < (float)(4 * LONGdiff)) && ((float)(4 * LONGdiff) <= 4.0))	 x = 3;
  int third = matrix3[y][x];
  //the below code checks if a boundary latitude is presented like 16 degree or 32 degree,
  //if a boundary value is presented then 1 will be subtracted from it else nothing is done
  if ((lat_ndsm % 4) == 0)
  {
    lat1 = abs(int(lat_ndsm)) / 4;
    //lat1 = int(lat_ndsm) / 4;					  //division by 4 is done as latitude varies by 4 degree
    lat1 = lat1 - 1;			              //if boubndary value is presented like 16 degree or 32 degree then 1 will be subtracted from it to make it of lower grid
    if (lat1 < 0)											  //value cannot be less than 0 hence this check is for that
    {
      lat1 = 0;
    }
  }
  else														//if a value that is not divisible by 4 is presented this else will work
  {
    lat1 = abs(int(lat_ndsm / 4));
    //lat1 = int(lat_ndsm / 4);
  }
  /*
  if ((long_ndsm % 6) == 0)
  {
    long1 = int(long_ndsm) / 6;									//division by 6 is done as long. varies by 6 degree
                                  //long1 = long1-1;		//if boubndary value is presented like 12 degree or 30 degree then 1 will be subtracted from it to make it of lower grid
                                  //Boundary value will move to higher grid in case of Longitude.
    if (long1 < 0)											        //value cannot be less than 0 hence this check is for that
    {
      long1 = 0;
    }
  }
  else														//if a value that is not divisible by 6 is presented this else will work
  {
    long1 = int(long_ndsm / 6);
  }
   
  */
  long1= int(long_ndsm/6);								//longitude are at a distance of 6 degrees
  temp[0] = matrix1[lat1];
  if ((long_ndsm >= 0) && (long_ndsm < 180))
  {
    longskc = long1 + 30;
  }
  else
  {
    longskc = long1 + 29;
  }
  
  int first_part1 = matrix1_part1[longskc];			       //Value at matrix1(lat1,long1)  will be the toposheet number for level I
  int lat2 = int(abs(lat_ndsm) - abs(lat1) * 4);
  lat2 = lat2 - 1;
  if (lat2 < 0) {
    lat2 = 0;
  }
  int long2 = int(abs(long_ndsm) - abs(long1) * 6);
  temp[1] = matrix2[lat2][long2];			//Value at matrix2(lat2,long2) will be the character,for level II toposheet division
  int lat3 = int((lat_ndsm1 - int(lat_ndsm1)) * 4);
  lat3 = lat3 - 1;
  if (lat3 < 0 || lat3 == 0) {
    lat3 = 3;
  }
  int long3 = int((long_ndsm1 - int(long_ndsm1)) * 4);
  //Value at matrix3(lat3,long3) will be the character,for level III toposheet division
  str_temp1 = (char)(((int)'0') + ((first_part1 / 10) % 10));
  str_temp2 = (char)(((int)'0') + (first_part1 % 10));  // Note:: putting (char) converts the righte side ascii value to a valid alphanumeric character;
  str_temp3 = (char)(((int)'0') + ((third / 10) % 10));
  str_temp4 = (char)(((int)'0') + ((third) % 10));
  arr[0] = temp[0];
  arr[1] = str_temp1;
  arr[2] = str_temp2;
  arr[3] = temp[1];
  arr[4] = str_temp3;
  arr[5] = str_temp4;
  /*arr[0]=  0;
  arr[1] = 0;
  arr[2] = 0;
  arr[3] = 0;
  arr[4] = 0;
  arr[5] = 0;*/
  //QgsDebugMsg(QString("str_temp1[2] = %1").arg(str_temp1[1]));
  for (int i = 0; i < 6; i++)
  {
    arr_ndsm[i] = arr[i];
  }
  return arr_ndsm;
}


inline QString QgsStatusBarCoordinatesWidget::LatLongToMilgridConversion(const QgsPointXY &mp)
{
  int false_easting = 500000, false_northing = 500000;
  double ecc = 298.2572, semi_minor_axis = 6356752.3142, e1 = 0.081819190842622, e = 2.718281828, semi_major_axis = 6378137.0;
  double diff = 6.0 / 7.0, pi = 180.0, correct;//pi=22.f/7.f;
  double round(double, int);
  double clat, clong;
  clat = 0;
  clong = 0;
  correct = 0.017453292519943295;
  //QString zone[] = { "1A","1B","1C","1D","1E","1F","1G","1H","1J","1K","1L","2A","2B","2C","2D","2E","2F","2G","2H","2J","2K","2L","3A","3B","3C","3D","3E","3F","3G","3H","3J","3K","3L","4A","4B","4C","4D","4E","4F","4G","4H","4J","4K","4L","5A","5B","5C","5D","5E","5F","5G","5H","5J","5K","5L","6A","6B","6C","6D","6E","6F","6G","6H","6J","6K","6L","7A","7B","7C","7D","7E","7F","7G","7H","7J","7K","7L","8A","8B","8C","8D","8E","8F","8G","8H""8J","8K","8L","9A","9B","9C","9D","9E","9F","9G","9H","9J","9K","9L","10A","10B","10C","10D","10E","10F","10G","10H","10J","10K","10L","11A","11B","11C","11D","11E","11F","11G","11H","11J","11K","11L","12A","12B","12C","12D","12E","12F","12G","12H","12J","12K","12L" };
  //New Zone added
  QString zone[] = { "19A6","19A5","19A4","19A3","19A2","19A1","19A","19B","19C","19D","19E","19F","19G","19H","19J","19K","19L","19M","19N","19O","19P","19Q","19R","19S","19T","19U","19V","19W","19X","19Y","20A6","20A5","20A4","20A3","20A2","20A1","20A","20B","20C","20D","20E","20F","20G","20H","20J","20K","20L","20M","20N","20O","20P","20Q","20R","20S","20T","20U","20V","20W","20X","20Y","21A6","21A5","21A4","21A3","21A2","21A1","21A","21B","21C","21D","21E","21F","21G","21H","21J","21K","21L","21M","21N","21O","21P","21Q","21R","21S","21T","21U","21V","21W","21X","21Y","22A6","22A5","22A4","22A3","22A2","22A1","22A","22B","22C","22D","22E","22F","22G","22H","22J","22K","22L","22M","22N","22O","22P","22Q","22R","22S","22T","22U","22V","22W","22X","22Y","23A6","23A5","23A4","23A3","23A2","23A1","23A","23B","23C","23D","23E","23F","23G","23H","23J","23K","23L","23M","23N","23O","23P","23Q","23R","23S","23T","23U","23V","23W","23X","23Y","24A6","24A5","24A4","24A3","24A2","24A1","24A","24B","24C","24D","24E","24F","24G","24H","24J","24K","24L","24M","24N","24O","24P","24Q","24R","24S","24T","24U","24V","24W","24X","24Y","25A6","25A5","25A4","25A3","25A2","25A1","25A","25B","25C","25D","25E","25F","25G","25H","25J","25K","25L","25M","25N","25O","25P","25Q","25R","25S","25T","25U","25V","25W","25X","25Y","26A6","26A5","26A4","26A3","26A2","26A1","26A","26B","26C","26D","26E","26F","26G","26H","26J","26K","26L","26M","26N","26O","26P","26Q","26R","26S","26T","26U","26V","26W","26X","26Y","27A6","27A5","27A4","27A3","27A2","27A1","27A","27B","27C","27D","27E","27F","27G","27H","27J","27K","27L","27M","27N","27O","27P","27Q","27R","27S","27T","27U","27V","27W","27X","27Y","28A6","28A5","28A4","28A3","28A2","28A1","28A","28B","28C","28D","28E","28F","28G","28H","28J","28K","28L","28M","28N","28O","28P","28Q","28R","28S","28T","28U","28V","28W","28X","28Y","29A6","29A5","29A4","29A3","29A2","29A1","29A","29B","29C","29D","29E","29F","29G","29H","29J","29K","29L","29M","29N","29O","29P","29Q","29R","29S","29T","29U","29V","29W","29X","29Y","30A6","30A5","30A4","30A3","30A2","30A1","30A","30B","30C","30D","30E","30F","30G","30H","30J","30K","30L","30M","30N","30O","30P","30Q","30R","30S","30T","30U","30V","30W","30X","30Y","31A6","31A5","31A4","31A3","31A2","31A1","31A","31B","31C","31D","31E","31F","31G","31H","31J","31K","31L","31M","31N","31O","31P","31Q","31R","31S","31T","31U","31V","31W","31X","31Y","32A6","32A5","32A4","32A3","32A2","32A1","32A","32B","32C","32D","32E","32F","32G","32H","32J","32K","32L","32M","32N","32O","32P","32Q","32R","32S","32T","32U","32V","32W","32X","32Y","33A6","33A5","33A4","33A3","33A2","33A1","33A","33B","33C","33D","33E","33F","33G","33H","33J","33K","33L","33M","33N","33O","33P","33Q","33R","33S","33T","33U","33V","33W","33X","33Y","34A6","34A5","34A4","34A3","34A2","34A1","34A","34B","34C","34D","34E","34F","34G","34H","34J","34K","34L","34M","34N","34O","34P","34Q","34R","34S","34T","34U","34V","34W","34X","34Y","35A6","35A5","35A4","35A3","35A2","35A1","35A","35B","35C","35D","35E","35F","35G","35H","35J","35K","35L","35M","35N","35O","35P","35Q","35R","35S","35T","35U","35V","35W","35X","35Y","36A6","36A5","36A4","36A3","36A2","36A1","36A","36B","36C","36D","36E","36F","36G","36H","36J","36K","36L","36M","36N","36O","36P","36Q","36R","36S","36T","36U","36V","36W","36X","36Y","37A6","37A5","37A4","37A3","37A2","37A1","37A","37B","37C","37D","37E","37F","37G","37H","37J","37K","37L","37M","37N","37O","37P","37Q","37R","37S","37T","37U","37V","37W","37X","37Y","38A6","38A5","38A4","38A3","38A2","38A1","38A","38B","38C","38D","38E","38F","38G","38H","38J","38K","38L","38M","38N","38O","38P","38Q","38R","38S","38T","38U","38V","38W","38X","38Y","39A6","39A5","39A4","39A3","39A2","39A1","39A","39B","39C","39D","39E","39F","39G","39H","39J","39K","39L","39M","39N","39O","39P","39Q","39R","39S","39T","39U","39V","39W","39X","39Y","40A6","40A5","40A4","40A3","40A2","40A1","40A","40B","40C","40D","40E","40F","40G","40H","40J","40K","40L","40M","40N","40O","40P","40Q","40R","40S","40T","40U","40V","40W","40X","40Y","41A6","41A5","41A4","41A3","41A2","41A1","41A","41B","41C","41D","41E","41F","41G","41H","41J","41K","41L","41M","41N","41O","41P","41Q","41R","41S","41T","41U","41V","41W","41X","41Y","42A6","42A5","42A4","42A3","42A2","42A1","42A","42B","42C","42D","42E","42F","42G","42H","42J","42K","42L","42M","42N","42O","42P","42Q","42R","42S","42T","42U","42V","42W","42X","42Y","43A6","43A5","43A4","43A3","43A2","43A1","43A","43B","43C","43D","43E","43F","43G","43H","43J","43K","43L","43M","43N","43O","43P","43Q","43R","43S","43T","43U","43V","43W","43X","43Y","44A6","44A5","44A4","44A3","44A2","44A1","44A","44B","44C","44D","44E","44F","44G","44H","44J","44K","44L","44M","44N","44O","44P","44Q","44R","44S","44T","44U","44V","44W","44X","44Y","45A6","45A5","45A4","45A3","45A2","45A1","45A","45B","45C","45D","45E","45F","45G","45H","45J","45K","45L","45M","45N","45O","45P","45Q","45R","45S","45T","45U","45V","45W","45X","45Y","1A6","1A5","1A4","1A3","1A2","1A1","1A","1B","1C","1D","1E","1F","1G","1H","1J","1K","1L","1M","1N","1O","1P","1Q","1R","1S","1T","1U","1V","1W","1X","1Y","2A6","2A5","2A4","2A3","2A2","2A1","2A","2B","2C","2D","2E","2F","2G","2H","2J","2K","2L","2M","2N","2O","2P","2Q","2R","2S","2T","2U","2V","2W","2X","2Y","3A6","3A5","3A4","3A3","3A2","3A1","3A","3B","3C","3D","3E","3F","3G","3H","3J","3K","3L","3M","3N","3O","3P","3Q","3R","3S","3T","3U","3V","3W","3X","3Y","4A6","4A5","4A4","4A3","4A2","4A1","4A","4B","4C","4D","4E","4F","4G","4H","4J","4K","4L","4M","4N","4O","4P","4Q","4R","4S","4T","4U","4V","4W","4X","4Y","5A6","5A5","5A4","5A3","5A2","5A1","5A","5B","5C","5D","5E","5F","5G","5H","5J","5K","5L","5M","5N","5O","5P","5Q","5R","5S","5T","5U","5V","5W","5X","5Y","6A6","6A5","6A4","6A3","6A2","6A1","6A","6B","6C","6D","6E","6F","6G","6H","6J","6K","6L","6M","6N","6O","6P","6Q","6R","6S","6T","6U","6V","6W","6X","6Y","7A7","7A5","7A4","7A3","7A2","7A1","7A","7B","7C","7D","7E","7F","7G","7H","7J","7K","7L","7M","7N","7O","7P","7Q","7R","7S","7T","7U","7V","7W","7X","7Y","8A8","8A5","8A4","8A3","8A2","8A1","8A","8B","8C","8D","8E","8F","8G","8H""8J","8K","8L","8M","8N","8O","8P","8Q","8R","8S","8T","8U","8V","8W","8X","8Y","9A9","9A5","9A4","9A3","9A2","9A1","9A","9B","9C","9D","9E","9F","9G","9H","9J","9K","9L","9M","9N","9O","9P","9Q","9R","9S","9T","9U","9V","9W","9X","9Y","10A10","10A5","10A4","10A3","10A2","10A1","10A","10B","10C","10D","10E","10F","10G","10H","10J","10K","10L","10M","10N","10O","10P","10Q","10R","10S","10T","10U","10V","10W","10X","10Y","11A6","11A5","11A4","11A3","11A2","11A1","11A","11B","11C","11D","11E","11F","11G","11H","11J","11K","11L","11M","11N","11O","11P","11Q","11R","11S","11T","11U","11V","11W","11X","11Y","12A6","12A5","12A4","12A3","12A2","12A1","12A","12B","12C","12D","12E","12F","12G","12H","12J","12K","12L","12M","12N","12O","12P","12Q","12R","12S","12T","12U","12V","12W","12X","12Y","13A6","13A5","13A4","13A3","13A2","13A1","13A","13B","13C","13D","13E","13F","13G","13H","13J","13K","13L","13M","13N","13O","13P","13Q","13R","13S","13T","13U","13V","13W","13X","13Y","14A6","14A5","14A4","14A3","14A2","14A1","14A","14B","14C","14D","14E","14F","14G","14H","14J","14K","14L","14M","14N","14O","14P","14Q","14R","14S","14T","14U","14V","14W","14X","14Y","15A6","15A5","15A4","15A3","15A2","15A1","15A","15B","15C","15D","15E","15F","15G","15H","15J","15K","15L","15M","15N","15O","15P","15Q","15R","15S","15T","15U","15V","15W","15X","15Y","16A6","16A5","16A4","16A3","16A2","16A1","16A","16B","16C","16D","16E","16F","16G","16H","16J","16K","16L","16M","16N","16O","16P","16Q","16R","16S","16T","16U","16V","16W","16X","16Y","17A6","17A5","17A4","17A3","17A2","17A1","17A","17B","17C","17D","17E","17F","17G","17H","17J","17K","17L","17M","17N","17O","17P","17Q","17R","17S","17T","17U","17V","17W","17X","17Y","18A6","18A5","18A4","18A3","18A2","18A1","18A","18B","18C","18D","18E","18F","18G","18H","18J","18K","18L","18M","18N","18O","18P","18Q","18R","18S","18T","18U","18V","18W","18X","18Y" };
  //QgsPoint mp  = p;
  int limiting_meridian; // 36 to 108
  int limiting_parallel; // -12 to 54
  int i, j, k, l = 0, i1;
  double std_para1 = 0.0, std_para2 = 0.0, temp1 = 0.0, temp2 = 0.0;
  double central_meridian, central_parallel;
  double dist_east = 0.0, dist_north = 0.0, pe = 0.0, pn = 0.0;
  double ro = 0.0, r = 0.0, r0 = 0.0, n1 = 0.0, n2 = 0.0, n = 0.0, n0 = 0.0, q0 = 0.0, q = 0.0, q1 = 0.0, q2 = 0.0, l1 = 0.0, zone_constant = 0.0, scale_factor = 0.0, scale_factor_center = 0.0;
  char third, fourth;
  char str_temp[20];
  QChar milgrid[20];
  QString str;
  double LAT = mp.y();            //30.47691944;//mp.y();
  double LONG = mp.x();           //78.01554444;//mp.x();
             /*LAT = 28;
             LONG = 68;*/
 // for (i = 0; i < 20; i++)
  for (i = 0; i < 22; i++)
    milgrid[i] = ' ';
  // Find out the grid zone
  k = -1;
//  for (i = 36; i <= 132; i = i + 8)
//  {
//    for (j = 54; j > (-13); j = j - 6)
  for (i = -180; i <= 180; i = i + 8)
  {
    for (j = 90; j > (-91); j = j - 6)
    {
      k = k + 1;
      if ((LONG >= i) && (LONG < i + 8) && (LAT < j) && (LAT >= j - 6))
      {
        l = k;
        //cout<<"\n\n Zone Found as "<<zone[l];
        limiting_meridian = i;
        limiting_parallel = j;
        break;
      }
    }
    k = k - 1;
  }
  //cout<<"\n\n Zone Found as "<<zone[l];
  //strcpy(milgrid, "");
  milgrid[0] = zone[l][0];
  milgrid[1] = zone[l][1];
  //milgrid[2] = ' ';
  milgrid[2] = zone[l][2];
  milgrid[3] = zone[l][3];
  milgrid[4] = ' ';
  std_para1 = limiting_parallel - 6;
  std_para1 = std_para1 + diff;
  std_para2 = limiting_parallel;
  std_para2 = std_para2 - diff;
  central_meridian = limiting_meridian + 4;
  temp1 = 0.0;
  temp2 = (1 - e1 * sin(std_para1*correct)) / (1 + e1 * sin(std_para1*correct));
  temp1 = pow(temp2, (e1 / 2));
  q1 = tan(((pi / 4) + (std_para1 / 2))*correct);
  q1 = temp1 * q1;
  q1 = log(q1);
  temp1 = 0.0;
  temp2 = 0.0;
  double sinph2, sinq2;
  sinph2 = sin(std_para2*correct);
  sinq2 = e1 * sinph2;
  temp2 = (1 - sinq2) / (1 + sinq2);
  temp2 = (1 - e1 * sinf(std_para2*correct)) / (1 + e1 * sinf(std_para2*correct));
  temp1 = powf(temp2, (e1 / 2));
  q2 = temp1 * (tan(((pi / 4) + (std_para2 / 2))*correct));
  q2 = logf(q2);
  temp1 = 0.0;
  temp2 = 0.0;
  temp2 = (1 - e1 * sinf(LAT*correct)) / (1 + e1 * sinf(LAT*correct));
  temp1 = powf(temp2, (e1 / 2));
  q = temp1 * (tan(((pi / 4) + (LAT / 2))*correct));
  q = logf(q);
  temp1 = 0.0;
  temp2 = 0.0;
  temp1 = powf(e1, 2.0)*powf(sinf(std_para1*correct), 2);
  n1 = semi_major_axis / sqrt(1 - temp1);
  temp1 = 0.0;
  temp1 = powf(e1, 2.0)*powf(sinf(std_para2*correct), 2);
  n2 = semi_major_axis / sqrt(1 - temp1);
  temp1 = 0.0;
  temp1 = powf(e1, 2.0)*powf(sinf(LAT*correct), 2);
  n = semi_major_axis / sqrt(1 - temp1);
  temp1 = 0.0;
  temp1 = logf(n1*cosf(std_para1*correct));
  temp2 = logf(n2*cosf(std_para2*correct));
  temp1 = temp1 - temp2;
  l1 = temp1 / (q2 - q1);
  temp1 = 0.0;
  temp1 = asinf(l1);
  central_parallel = temp1 / correct;  // central parallel by calculation
  temp1 = 0.0;
  temp2 = (1 - e1 * sinf(central_parallel*correct)) / (1 + e1 * sinf(central_parallel*correct));
  temp1 = powf(temp2, (e1 / 2));
  q0 = temp1 * (tan(((pi / 4) + (central_parallel / 2))*correct));
  q0 = logf(q0);
  temp1 = 0.0;
  temp2 = 0.0;
  temp1 = powf(e1, 2.0)*powf(sinf(central_parallel*correct), 2);
  n0 = semi_major_axis / sqrt(1 - temp1);
  temp1 = 0.0;
  zone_constant = (n1*cosf(std_para1*correct)) / (l1*expf(-l1 * q1));
  zone_constant = (n2*cosf(std_para2*correct)) / (l1*expf(-l1 * q2));
  scale_factor = (zone_constant*l1*expf(-l1 * q)) / (n*cosf(LAT*correct));
  scale_factor_center = (zone_constant*l1*expf(-l1 * q0)) / (n0*cosf(central_parallel*correct));
  r0 = scale_factor_center * n0*(1 / (tanf(central_parallel*correct)));
  r = zone_constant * expf(-l1 * q);//scale_factor*expf(-l1*q);//zone_constant*expf(-l1*q);
  dist_east = r * sinf(l1*(LONG - central_meridian)*correct); //distance from false eassting
  dist_north = r0 - r * cosf(l1*(LONG - central_meridian)*correct);// distance from false northing
  pe = false_easting + dist_east;
  pn = false_northing + dist_north + 11;
  i = int(pe);
  j = int(pn);
  if ((i >= 0) && (i < 100000))
    fourth = 'A';
  else if ((i >= 100000) && (i < 200000))
    fourth = 'B';
  else if ((i >= 200000) && (i < 300000))
    fourth = 'C';
  else if ((i >= 300000) && (i < 400000))
    fourth = 'D';
  else if ((i >= 400000) && (i < 500000))
    fourth = 'E';
  else if ((i >= 500000) && (i < 600000))
    fourth = 'F';
  else if ((i >= 600000) && (i < 700000))
    fourth = 'G';
  else if ((i >= 700000) && (i < 800000))
    fourth = 'H';
  else if ((i >= 800000) && (i < 900000))
    fourth = 'J';
  else if ((i >= 900000) && (i < 1000000))
    fourth = 'K';
  //milgrid[3] = fourth;
  milgrid[5] = fourth;
  if ((j < 100000) && (j >= 900000))
    third = 'K';
  else if ((j < 900000) && (j >= 800000))
    third = 'J';
  else if ((j < 800000) && (j >= 700000))
    third = 'H';
  else if ((j < 700000) && (j >= 600000))
    third = 'G';
  else if ((j < 600000) && (j >= 500000))
    third = 'F';
  else if ((j < 500000) && (j >= 400000))
    third = 'E';
  else if ((j < 400000) && (j >= 300000))
    third = 'D';
  else if ((j < 300000) && (j >= 200000))
    third = 'C';
  else if ((j < 200000) && (j >= 100000))
    third = 'B';
  else if ((j < 100000) && (j >= 0))
    third = 'A';
  //milgrid[4] = third;
  //milgrid[5] = ' ';
  milgrid[6] = third;
  milgrid[7] = ' ';
  sprintf(str_temp, "%d", i);
  k = strlen(str_temp);
  /*if (k > 5)
  {
    for (l = 1; l < k; l++)
      milgrid[5 + l] = str_temp[l];
  }
  else
  {
    k = k + 1;
    for (l = 0; l < k; l++)
      milgrid[5 + 1 + l] = str_temp[l];
  }
  */
  if (k < 6)
  {
    for (l = 0; l < k; l++)
      milgrid[7 + l] = str_temp[l];
  }
  else
  {
    for (l = 1; l < k; l++)
      milgrid[7 + l] = str_temp[l];
  }
  //strcpy(str_temp,"");
  sprintf(str_temp, "%d", j);
  i1 = strlen(str_temp);
  for (l = 1; l < i1; l++)
 //   milgrid[5 + l + k] = str_temp[l];
 // milgrid[5 + l + k + 1] = '\0';
    milgrid[7 + l + k] = str_temp[l];
  milgrid[7 + l + k + 1] = '\0';
  //for (i = 0; i < 18; i++)
  for (i = 0; i < 20; i++)
  {
    str[i] = milgrid[i];
  }
  //if((mp.x()>36 && mp.x()<132) && (mp.y()> -12 && mp.y()<45) ) return str ;
  //else return("OUT OF BOUND AREA" );
  return str;
}
inline QString QgsStatusBarCoordinatesWidget::eveLatLongToMilgridConversion(const QgsPointXY &mp)
{
  
  double std_para1 = 0.0, std_para2 = 0.0, temp1 = 0.0, temp2 = 0.0;
  double central_meridian, central_parallel, theta, deltalongitude;
  double dist_east = 0.0, dist_north = 0.0, pe = 0.0, pn = 0.0;
  double ro = 0.0, r = 0.0, r0 = 0.0, n1 = 0.0, n2 = 0.0, n = 0.0, n0 = 0.0, q0 = 0.0, q = 0.0, q1 = 0.0, q2 = 0.0, l1 = 0.0, zone_constant = 0.0, scale_factor = 0.0, scale_factor_center = 0.0;
  int false_easting, false_northing;
  double ecc = 298.2572, semi_minor_axis = 6356100.228, e1 = 0.0814729809826527, e = 2.718281828, semi_major_axis = 6377301.243;
  double diff = 6.0 / 7.0, pi = 180.0, correct = 0.017453292519943295;//pi=22.f/7.f;
  int var = 0;
  double LAT = mp.y();//30.47691944;//mp.y();
  double LONG = mp.x();//78.01554444;//mp.x();
  QString finaloutput1, finaloutput2;
  int grid_return[4];
  char str_temp1[1];
  char str_temp2[1];
  char str_temp3[9];
  char str_temp4[9];
  double grid_data[9][6] = { { 68, 32.50, 29.65527, 35.31447, 2743196.400, 914398.800 },{ 90, 32.50, 29.65527, 35.31447,  2743196.400, 914398.800 },{ 74, 26, 23.157823, 28.81827, 2743196.400, 914398.800 },{ 90, 26, 23.157823, 28.81827, 2743196.400, 914398.800 },{ 80, 19,  16.1602, 21.82249, 2743196.400, 914398.800 },{ 100, 19,  16.1602, 21.82249, 2743196.400, 914398.800 },{ 80, 12, 9.16283, 14.8269, 2743196.400, 914398.800 },{ 104, 12, 9.16283, 14.8269, 2743196.400, 914398.800 },{ 68, 39.50, 36.2916, 42.65625, 2153866.400, 2368292.900 } };
  //bounding box for various sides polygon
  float grid_3_sides[11][4] = { { 36,60,44,78 },{ 28,60,36,79 },{ 29,79,36,81 },{ 25,82,28,83 },{ 22.50,79,29,82 },{ 22,72,28,79 },{ 20,60,28,72 },{ 25,57,28,60 },{ 15,60,20,72 },{ 15,72,22,85 },{ 15,85,21,90 } };
  float grid_2_sides[11][4] = { { 36,78,37,81 },{ 30,81,37,102 },{ 29,81,30,82 },{ 28,82,30,83 },{ 22,82,25,83 },{ 22,83,30,93 },{ 21,85,22,91 },{ 22.50,93,30,110 },{ 15.50,90,21,91 },{ 15.50,91,22,93 },{ 15.50,93,22.50,110 } };
  float grid_4_sides[9][4] = { { 8,60,15,76 },{ 5,76,15,79.50 },{ 10,79.50,15,82 },{ 8,82,15,90 },{ 8,90,15.50,92 },{ 6,92,15.50,94 },{ 7,94,15.50,98.66 },{ 8,98.66,15.50,105 },{ 7,105,15.50,110 } };
  int row = 0;
  char* p2;
  QChar p2arr[29];
  //the below code works as follows
  //check_domain_X_sides function takes four parameter from grid_X_sides matrix above (where X=2,3 or 4)...check_domain_X_side compare these four values with latitude and longitude and "if latitude and longitude lies in between these four then 1 is returned else 0 is returned"
  //if 1 is retuned then if condition inside for loop becomes true and we get to know that latitude and longitude lies in a grid which is of x sides
  //check_row_X_sides just sees in which grid number it lies as larger grid is divided into many smaller grids
  //for two sides
  for (row = 0; row < 11; row++) {
    var = check_domain_2_sides(grid_2_sides[row][0], grid_2_sides[row][1], grid_2_sides[row][2], grid_2_sides[row][3], LAT, LONG);
    if (var == 1) {
      p2 = check_row_2_sides(row);			//check_row_X_sides just sees in which grid number it lies as larger grid is divided into many smaller grids
      break;
    }
  }
  //for three sides
  if (var != 1) {
    for (row = 0; row < 11; row++) {
      int var = check_domain_3_sides(grid_3_sides[row][0], grid_3_sides[row][1], grid_3_sides[row][2], grid_3_sides[row][3], LAT, LONG);
      if (var == 1) {
        p2 = check_row_3_sides(row);
        break;
      }
    }
  }
  //for four sides
  if (var != 1) {
    for (row = 0; row < 9; row++) {
        int var = check_domain_4_sides(grid_4_sides[row][0], grid_4_sides[row][1], grid_4_sides[row][2], grid_4_sides[row][3], LAT, LONG);
        if (var == 1) {
        p2 = check_row_4_sides(row);
        break;
      }
    }
  }
  //cout<<p2<<"\n";					//print grid number...
  //this code is used for below cde
  row = -1;
  try
  {
  if(p2!=nullptr)
  if (strcmp(p2, "GRID IA") == 0) {
  row = 0;
  }
  else if (strcmp(p2, "GRID IB") == 0) {
  row = 1;
  }
  else if (strcmp(p2, "GRID IIA") == 0) {
  row = 2;
  }
  else if (strcmp(p2, "GRID IIB") == 0) {
  row = 3;
  }
  else if (strcmp(p2, "GRID IIIA") == 0) {
  row = 4;
  }
  else if (strcmp(p2, "GRID IIIB") == 0) {
  row = 5;
  }
  else if (strcmp(p2, "GRID IVA") == 0) {
  row = 6;
  }
  else if (strcmp(p2, "GRID IVB") == 0) {
  row = 7;
  }
  else if (strcmp(p2, "GRID O") == 0) {
  row = 8;
  }
  else { row = -1; }
  if (row == -1)
  {
    finaloutput2 = "OUT OF BOUND AREA";
    return finaloutput2;
  }
  else
  {
    for (int i = 0; i < 29; i++)
    {
    p2arr[i] = ' ';
    }
    for (int i = 0; i < 9; i++)
    {
    p2arr[i] = p2[i];  //*(p2+i)
    }
    central_meridian = grid_data[row][0];
    central_parallel = grid_data[row][1];
    std_para1 = grid_data[row][2];
    std_para2 = grid_data[row][3];
    false_easting = grid_data[row][4];
    false_northing = grid_data[row][5];
    //this is to find q1
    temp2 = (1 - e1*sin(std_para1*correct)) / (1 + e1*sin(std_para1*correct));
    temp1 = pow(temp2, (e1 / 2));
    q1 = tan(((pi / 4) + (std_para1 / 2))*correct);
    q1 = temp1*q1;
    q1 = log(q1);
    //this is to find q2
    temp1 = 0.0;
    temp2 = 0.0;
    temp2 = (1 - e1*sinf(std_para2*correct)) / (1 + e1*sinf(std_para2*correct));
    temp1 = powf(temp2, (e1 / 2));
    q2 = temp1*(tan(((pi / 4) + (std_para2 / 2))*correct));
    q2 = logf(q2);
    //this is to find q
    temp1 = 0.0;
    temp2 = 0.0;
    temp2 = (1 - e1*sinf(LAT*correct)) / (1 + e1*sinf(LAT*correct));
    temp1 = powf(temp2, (e1 / 2));
    q = temp1*(tan(((pi / 4) + (LAT / 2))*correct));
    q = logf(q);
    //this is to find n1
    temp1 = 0.0;
    temp2 = 0.0;
    temp1 = powf(e1, 2.0)*powf(sinf(std_para1*correct), 2);
    n1 = semi_major_axis / sqrt(1 - temp1);
    //this is to find n2
    temp1 = 0.0;
    temp1 = powf(e1, 2.0)*powf(sinf(std_para2*correct), 2);
    n2 = semi_major_axis / sqrt(1 - temp1);
    //this is to find n
    temp1 = 0.0;
    temp1 = powf(e1, 2.0)*powf(sinf(LAT*correct), 2);
    n = semi_major_axis / sqrt(1 - temp1);
    //this is to find l1
    temp1 = 0.0;
    temp1 = logf(n1*cosf(std_para1*correct));
    temp2 = logf(n2*cosf(std_para2*correct));
    temp1 = temp1 - temp2;
    l1 = temp1 / (q2 - q1);
    temp1 = 0.0;
    temp1 = asinf(l1);
    central_parallel = temp1 / correct;  // central parallel by calculation
    //this is to find q0
    temp1 = 0.0;
    temp2 = (1 - e1*sinf(central_parallel*correct)) / (1 + e1*sinf(central_parallel*correct));
    temp1 = powf(temp2, (e1 / 2));
    q0 = temp1*(tan(((pi / 4) + (central_parallel / 2))*correct));
    q0 = logf(q0);
    //this is to find n0
    temp1 = 0.0;
    temp2 = 0.0;
    temp1 = powf(e1, 2.0)*powf(sinf(central_parallel*correct), 2);
    n0 = semi_major_axis / sqrt(1 - temp1);
    //this is to find zone_consant(K)
    zone_constant = (n1*cosf(std_para1*correct)) / (l1*expf(-l1*q1));
    zone_constant = (n2*cosf(std_para2*correct)) / (l1*expf(-l1*q2));
    //this is to find k
    scale_factor = (zone_constant*l1*expf(-l1*q)) / (n*cosf(LAT*correct));
    //this is to find k0
    scale_factor_center = (zone_constant*l1*expf(-l1*q0)) / (n0*cosf(central_parallel*correct));
    //this is to find r0 and r
    r0 = scale_factor_center*n0*(1 / (tanf(central_parallel*correct)));
    r = zone_constant*expf(-l1*q);
    //this is to find dist_east and dist_north
    dist_east = r*sinf(l1*(LONG - central_meridian)*correct); //distance from false easting
    dist_north = r0 - r*cosf(l1*(LONG - central_meridian)*correct);// distance from false northing
    //this is to find pe and pn
    pe = false_easting + dist_east;
    pn = false_northing + dist_north - 34;
    //cout<<pe<<"\n\n";
    //cout<<pn;
    pe = int(pe);
    pn = int(pn);
    int pe1 = pe / 1000;
    int pn1 = pn / 1000;
    int *pointer = checkarray(pe1, pn1);
    int *pointer1 = grid_return;
    grid_return[0] = pointer[0];
    grid_return[1] = pointer[1];
    grid_return[2] = pe;
    grid_return[3] = pn;
    sprintf(str_temp1, "%c", grid_return[0]);
    p2arr[10] = str_temp1[0];
    sprintf(str_temp2, "%c", grid_return[1]);
    p2arr[11] = str_temp2[0];
    p2arr[12] = ' ';
    sprintf(str_temp3, "%d", grid_return[2]);
    sprintf(str_temp4, "%d", grid_return[3]);
    int k = 0;
    k = strlen(str_temp3);
    for (int i = 0; i < k - 1; i++)
    {
    p2arr[13 + i] = str_temp3[i + 2];
    }
    p2arr[11 + k] = ' ';
    int a = 0;
    int getresult = 5;
    a = strlen(str_temp4);
    for (int w = (a - getresult); w < a; w++)
    {
    p2arr[11 + k + 1 + 1 + w] = str_temp4[w];
    }
    for (int y = 0; y < 28; y++)
    {
    finaloutput1[y] = p2arr[y];
    }
    return finaloutput1;
    /*
    return "check";
    */
  }
  }
  catch (...)
  {

  }
}
//all above used functions a defined below
char* QgsStatusBarCoordinatesWidget::check_row_2_sides(int r)
{
  char *a1;
  if (r == 0 || r == 1) {
    a1 = "GRID IB";
    //int pointer = a1;
    return a1;
  }
  else if (r == 2 || r == 3 || r == 4 || r == 5 || r == 6 || r == 7) {
    a1 = "GRID IIB";
    //int pointer = a1;
    return a1;
  }
  else if (r == 8 || r == 9 || r == 10) {
    a1 = "GRID IIIB";
    //int pointer = a1;
    return a1;
  }
  return 0;
}

int QgsStatusBarCoordinatesWidget::check_domain_2_sides(float lat1, float long1, float lat2, float long2, double latitude, double longitude)
{
  if (latitude > lat1&&latitude <= lat2) {
    if (longitude > long1&&longitude <= long2) {
      return 1;
    }
  }
  else {
    return 0;
  }
  return 0;
}

char* QgsStatusBarCoordinatesWidget::check_row_3_sides(int r)
{
  char *a1;
  if (r == 0) {
    a1 = "GRID O";
    return a1;
  }
  else if (r == 1 || r == 2) {
    a1 = "GRID IA";
    return a1;
  }
  else if (r == 3 || r == 4 || r == 5 || r == 6 || r == 7) {
    a1 = "GRID IIA";
    return a1;
  }
  else if (r == 8 || r == 9 || r == 10) {
    a1 = "GRID IIIA";
    return a1;
  }
  return 0;
}

int QgsStatusBarCoordinatesWidget::check_domain_3_sides(float lat1, float long1, float lat2, float long2, double latitude, double longitude)
{
  if (latitude > lat1&&latitude <= lat2) {
    if (longitude >= long1 && longitude <= long2) {
      return 1;
    }
  }
  else {
    return 0;
  }
  return 0;
}
char* QgsStatusBarCoordinatesWidget::check_row_4_sides(int r) {
  char *a1;
  if (r == 0 || r == 1 || r == 2 || r == 3) {
    a1 = "GRID IVA";
    return a1;
  }
  else if (r == 4 || r == 5 || r == 6 || r == 7) {
    a1 = "GRID IVB";
    return a1;
  }
  return 0;
}

int QgsStatusBarCoordinatesWidget::check_domain_4_sides(float lat1, float long1, float lat2, float long2, double latitude, double longitude)
{
  if (latitude >= lat1 && latitude <= lat2) {
    if (longitude >= long1 && longitude <= long2) {
      return 1;
    }
  }
  else {
    return 0;
  }
  return 0;
}

int* QgsStatusBarCoordinatesWidget::checkarray(int pe_local, int pn_local)
{
  static int temp[2];
  char ch = 'A';
  char arr[10][10];
  //this loop is for aray arr
  for (int i = 9; i >= 0; i--)
  {
    for (int j = 0; j < 10; j++)
    {
      if (i == 4 && j == 0)
      {
        ch = 'A';
      }
      else if (j == 5)
      {
        ch = arr[i][0];
      }
      arr[i][j] = ch;
      if (ch == 'H') {
        ch++;
      }
      ch++;
    }
  }
  int pe1_local = int(pe_local / 500);
  int pn1_local = int(pn_local / 500);
  int var = arr[pn1_local][pe1_local];
  int pe2_local = int((pe_local - (pe1_local * 500))) / 100;
  int pn2_local = int((pn_local - (pn1_local * 500))) / 100;
  int var1 = arr[pn2_local][pe2_local];
  int *pointer = temp;
  temp[0] = var;
  temp[1] = var1;
  return pointer;						//returns value of array
  
}
//Nihcas above

void QgsStatusBarCoordinatesWidget::updateCoordinateDisplay()
{
  if (mToggleExtentsViewButton->isChecked())
  {
    return;
  }

  if (mLastCoordinate.isEmpty())
    mLineEdit->clear();
  else
    mLineEdit->setText(QgsCoordinateUtils::formatCoordinateForProject(QgsProject::instance(), mLastCoordinate, mMapCanvas->mapSettings().destinationCrs(),
      static_cast<int>(mMousePrecisionDecimalPlaces)));

  ensureCoordinatesVisible();
}

//Overload updatecordinate
void QgsStatusBarCoordinatesWidget::updateCoordinateDisplayUpdated(const QgsPointXY& Qp)
{
  if (mToggleExtentsViewButton->isChecked())
  {
    return;
  }

  if (mLastCoordinate.isEmpty())
 //   mLineEdit->clear();
    mCoordsGeocord->clear();
  else {
    /* mLineEdit->setText(QgsCoordinateUtils::formatCoordinateForProject(QgsProject::instance(), mLastCoordinate, mMapCanvas->mapSettings().destinationCrs(),
       static_cast<int>(mMousePrecisionDecimalPlaces)));*/

    QgsPointXY p;
    QgsPointXY p1 = QgsPointXY(Qp);
    QgsCoordinateReferenceSystem crsSrc, crsWgs;
    crsSrc = mMapCanvas->mapSettings().destinationCrs();
    if (crsSrc.authid() != "EPSG:4326")
    {
      crsWgs = QgsCoordinateReferenceSystem(4326);
      QgsCoordinateTransform xform = QgsCoordinateTransform(crsSrc, crsWgs, QgsProject::instance());
      p = xform.transform(p1);
      mCoordsGeocord->setText(QgsCoordinateUtils::formatCoordinateForProject(QgsProject::instance(), p, crsWgs, 2));  //precision for latitude and longitude to show on screen
      ensureCoordinatesVisible();
    }
    else
    {
      p = Qp;
      mCoordsGeocord->setText(QgsCoordinateUtils::formatCoordinateForProject(QgsProject::instance(), Qp, mMapCanvas->mapSettings().destinationCrs(),
        mMousePrecisionDecimalPlaces));

      ensureCoordinatesVisible();
    }
    
  }

  ensureCoordinatesVisible();
}

void QgsStatusBarCoordinatesWidget::coordinateDisplaySettingsChanged()
{
  const QgsCoordinateReferenceSystem coordinateCrs = QgsProject::instance()->displaySettings()->coordinateCrs();

  const Qgis::CoordinateOrder projectOrder = QgsProject::instance()->displaySettings()->coordinateAxisOrder();
  const Qgis::CoordinateOrder order = projectOrder == Qgis::CoordinateOrder::Default
                                      ? QgsCoordinateReferenceSystemUtils::defaultCoordinateOrderForCrs( coordinateCrs )
                                      : projectOrder;

  switch ( order )
  {
    case Qgis::CoordinateOrder::XY:
      if ( coordinateCrs.isGeographic() )
        mLineEdit->setToolTip( tr( "Current map coordinate (Longitude, Latitude)" ) );
      else
        mLineEdit->setToolTip( tr( "Current map coordinate (Easting, Northing)" ) );
      break;
    case Qgis::CoordinateOrder::YX:
      if ( coordinateCrs.isGeographic() )
        mLineEdit->setToolTip( tr( "Current map coordinate (Latitude, Longitude)" ) );
      else
        mLineEdit->setToolTip( tr( "Current map coordinate (Northing, Easting)" ) );
      break;
    case Qgis::CoordinateOrder::Default:
      break;
  }

  updateCoordinateDisplay();
}
