/***************************************************************************
    qgsgrassnewmapset.cpp  - New GRASS mapset wizard
                               -------------------
    begin                : October, 2005
    copyright            : (C) 2005 by Radim Blazek
    email                : radim.blazek@gmail.it
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsgrassnewmapset.h"
#include "moc_qgsgrassnewmapset.cpp"
#include "qgsgrassplugin.h"
#include "qgsgrass.h"
#include "qgis.h"

#include "qgisinterface.h"
#include "qgsapplication.h"
#include "qgscoordinatetransform.h"
#include "qgslogger.h"
#include "qgsmapcanvas.h"
#include "qgsproject.h"
#include "qgsprojectionselectiontreewidget.h"
#include "qgslocalec.h"
#include "qgsexception.h"
#include "qgssettings.h"
#include "qgsgui.h"
#include "qgsextentwidget.h"

#include <QCloseEvent>
#include <QFileDialog>
#include <QMessageBox>
#include <QRegExp>
#include <QTextStream>
#include <QRegularExpression>
#include <QRegularExpressionValidator>


extern "C"
{
#if defined( _MSC_VER ) && defined( M_PI_4 )
#undef M_PI_4 //avoid redefinition warning
#endif
#include <grass/gprojects.h>
}

// For bug in GPJ_osr_to_grass()
#include "grass/version.h"
// Prevents some compiler warnings from the version.h include
#ifndef GRASS_VERSION_RELEASE
// When using newer version of GRASS (cvs >= 26.4.2007),
// these variables are #defined instead of being static variables.
QString temp1( GRASS_VERSION_STRING );
QString temp2( GRASS_VERSION_MAJOR );
QString temp3( GRASS_VERSION_MINOR );
QString temp4( GRASS_VERSION_RELEASE );
#endif

QgsGrassNewMapset::QgsGrassNewMapset( QgisInterface *iface, QgsGrassPlugin *plugin, QWidget *parent, Qt::WindowFlags f )
  : QWizard( parent, f )
  , QgsGrassNewMapsetBase()
  , mIface( iface )
  , mPlugin( plugin )
{
  QgsDebugMsgLevel( "QgsGrassNewMapset()", 3 );

  setupUi( this );
  QgsGui::instance()->enableAutoGeometryRestore( this );

  mDirectoryWidget->setStorageMode( QgsFileWidget::GetDirectory );
  mDirectoryWidget->lineEdit()->setShowClearButton( false );
  connect( mDirectoryWidget, &QgsFileWidget::fileChanged, this, &QgsGrassNewMapset::databaseChanged );

  mExtentWidget = new QgsExtentWidget( nullptr, QgsExtentWidget::ExpandedStyle );
  QVBoxLayout *extentLayout = new QVBoxLayout();
  extentLayout->setContentsMargins( 0, 0, 0, 0 );
  extentLayout->addWidget( mExtentWidget );
  mExtentWidgetFrame->setLayout( extentLayout );
  mExtentWidget->setMapCanvas( mIface->mapCanvas() );

  connect( mCreateLocationRadioButton, &QRadioButton::clicked, this, &QgsGrassNewMapset::mCreateLocationRadioButton_clicked );
  connect( mSelectLocationRadioButton, &QRadioButton::clicked, this, &QgsGrassNewMapset::mSelectLocationRadioButton_clicked );
  connect( mLocationComboBox, &QComboBox::editTextChanged, this, &QgsGrassNewMapset::mLocationComboBox_textChanged );
  connect( mLocationLineEdit, &QLineEdit::returnPressed, this, &QgsGrassNewMapset::mLocationLineEdit_returnPressed );
  connect( mLocationLineEdit, &QLineEdit::textChanged, this, &QgsGrassNewMapset::mLocationLineEdit_textChanged );
  connect( mNoProjRadioButton, &QRadioButton::clicked, this, &QgsGrassNewMapset::mNoProjRadioButton_clicked );
  connect( mProjRadioButton, &QRadioButton::clicked, this, &QgsGrassNewMapset::mProjRadioButton_clicked );
  connect( mExtentWidget, &QgsExtentWidget::extentChanged, this, &QgsGrassNewMapset::regionChanged );
  connect( mRegionButton, &QPushButton::clicked, this, &QgsGrassNewMapset::mRegionButton_clicked );
  connect( mMapsetLineEdit, &QLineEdit::returnPressed, this, &QgsGrassNewMapset::mMapsetLineEdit_returnPressed );
  connect( mMapsetLineEdit, &QLineEdit::textChanged, this, &QgsGrassNewMapset::mMapsetLineEdit_textChanged );
  connect( mOpenNewMapsetCheckBox, &QCheckBox::stateChanged, this, &QgsGrassNewMapset::mOpenNewMapsetCheckBox_stateChanged );
#ifdef Q_OS_MAC
  setWizardStyle( QWizard::ClassicStyle );
#endif

  QString mapPath = QStringLiteral( ":/images/grass/world.png" );
  QgsDebugMsgLevel( QString( "mapPath = %1" ).arg( mapPath ), 2 );

  //mPixmap = QPixmap( *(mRegionMap->pixmap()) );
  mPixmap.load( mapPath );
  QgsDebugMsgLevel( QString( "mPixmap.isNull() = %1" ).arg( mPixmap.isNull() ), 3 );

  mRegionsInited = false;

  setError( mDatabaseErrorLabel );
  setError( mLocationErrorLabel );
  setError( mProjErrorLabel );
  setError( mRegionErrorLabel );
  setError( mMapsetErrorLabel );

  // DATABASE
  QgsSettings settings;
  QString gisdbase = settings.value( QStringLiteral( "GRASS/lastGisdbase" ) ).toString();
  if ( gisdbase.isEmpty() )
  {
    gisdbase = QDir::homePath() + QDir::separator() + "grassdata";
  }
  mDirectoryWidget->setFilePath( gisdbase );
  databaseChanged();

  // LOCATION
  const thread_local QRegularExpression rx( "[A-Za-z0-9_.]+" );
  mLocationLineEdit->setValidator( new QRegularExpressionValidator( rx, mLocationLineEdit ) );

  // CRS

  // MAPSET
  mMapsetsListView->clear();
  mMapsetLineEdit->setValidator( new QRegularExpressionValidator( rx, mMapsetLineEdit ) );

  mMapsetsListView->header()->setSectionResizeMode( QHeaderView::ResizeToContents );

  // FINISH
  mOpenNewMapsetCheckBox->setChecked( settings.value( QStringLiteral( "GRASS/newMapsetWizard/openMapset" ), true ).toBool() );
  connect( this, &QWizard::currentIdChanged, this, &QgsGrassNewMapset::pageSelected );
}

QgsGrassNewMapset::~QgsGrassNewMapset() = default;

void QgsGrassNewMapset::databaseChanged()
{
  QgsSettings settings;
  settings.setValue( QStringLiteral( "GRASS/lastGisdbase" ), mDirectoryWidget->filePath() );

  button( QWizard::NextButton )->setEnabled( false );
  setError( mDatabaseErrorLabel );

  if ( gisdbase().isEmpty() )
  {
    //setError( mDatabaseErrorLabel, tr( "Enter path to GRASS database" ) );
    button( QWizard::NextButton )->setEnabled( false );
    return;
  }
  button( QWizard::NextButton )->setEnabled( true );

  if ( !gisdbaseExists() )
  {
    // Do not warn, it may be default $HOME/grassdata, if does not exist, it will be created on finish
    //setError( mDatabaseErrorLabel, tr( "The directory doesn't exist!" ) );
    //return;
  }
  else
  {
    // Check if at least one writable location exists or database is writable
    bool locationExists = false;
    QDir dir( gisdbase() );
    for ( unsigned int i = 0; i < dir.count(); i++ )
    {
      if ( dir[i] == QLatin1String( "." ) || dir[i] == QLatin1String( ".." ) )
        continue;

      QString windName = gisdbase() + "/" + dir[i] + "/PERMANENT/DEFAULT_WIND";
      QString locationName = gisdbase() + "/" + dir[i];
      QFileInfo locationInfo( locationName );

      if ( QFile::exists( windName ) && locationInfo.isWritable() )
      {
        locationExists = true;
        break;
      }
    }

    QFileInfo gisdbaseInfo( gisdbase() );
    if ( locationExists || gisdbaseInfo.isWritable() )
    {
      button( QWizard::NextButton )->setEnabled( true );
    }
    else
    {
      setError( mDatabaseErrorLabel, tr( "No writable locations, the database is not writable!" ) );
    }
  }
}

QString QgsGrassNewMapset::gisdbase() const
{
  return mDirectoryWidget->filePath();
}

bool QgsGrassNewMapset::gisdbaseExists()
{
  QFileInfo databaseInfo( gisdbase() );
  return databaseInfo.exists();
}

/*************************** LOCATION *******************************/
void QgsGrassNewMapset::setLocationPage()
{
  setLocations();
}

void QgsGrassNewMapset::setLocations()
{
  mLocationComboBox->clear();

  QgsSettings settings;
  QString lastLocation = settings.value( QStringLiteral( "GRASS/lastLocation" ) ).toString();

  if ( gisdbaseExists() )
  {
    // Get available locations with write permissions
    QDir gisdbaseDir( gisdbase() );

    // Add all subdirs containing PERMANENT/DEFAULT_WIND
    int idx = 0;
    int sel = -1;
    for ( unsigned int i = 0; i < gisdbaseDir.count(); i++ )
    {
      if ( gisdbaseDir[i] == QLatin1String( "." ) || gisdbaseDir[i] == QLatin1String( ".." ) )
        continue;

      QString windName = mDirectoryWidget->filePath() + "/" + gisdbaseDir[i] + "/PERMANENT/DEFAULT_WIND";
      QString locationName = mDirectoryWidget->filePath() + "/" + gisdbaseDir[i];
      QFileInfo locationInfo( locationName );

      if ( QFile::exists( windName ) && locationInfo.isWritable() )
      {
        mLocationComboBox->insertItem( -1, QString( gisdbaseDir[i] ) );
        if ( QString( gisdbaseDir[i] ) == lastLocation )
        {
          sel = idx;
        }
        idx++;
      }
    }
    if ( sel >= 0 )
    {
      mLocationComboBox->setCurrentIndex( sel );
    }
  }

  if ( mLocationComboBox->count() == 0 )
  {
    mCreateLocationRadioButton->setChecked( true );
    mSelectLocationRadioButton->setEnabled( false );
  }
  else
  {
    mSelectLocationRadioButton->setEnabled( true );
  }

  locationRadioSwitched(); // calls also checkLocation()
}

void QgsGrassNewMapset::locationRadioSwitched()
{
  if ( mSelectLocationRadioButton->isChecked() )
  {
    mLocationComboBox->setEnabled( true );
    mLocationLineEdit->setEnabled( false );
  }
  else
  {
    mLocationComboBox->setEnabled( false );
    mLocationLineEdit->setEnabled( true );
  }
  checkLocation();
}

int QgsGrassNewMapset::nextId() const
{
  int id = currentId();
  switch ( id )
  {
    case Location:
      if ( mSelectLocationRadioButton->isChecked() )
      {
        id = MapSet;
        break;
      }
      [[fallthrough]];
    case Database:
    case Crs:
    case Region:
    case MapSet:
      id += 1;
      break;
    case Finish:
    default:
      id = -1;
  }
  return id;
}

void QgsGrassNewMapset::checkLocation()
{
  setError( mLocationErrorLabel );
  button( QWizard::NextButton )->setEnabled( true );

  if ( mCreateLocationRadioButton->isChecked() )
  {
    QString location = mLocationLineEdit->text();

    if ( location.isEmpty() )
    {
      button( QWizard::NextButton )->setEnabled( false );
      setError( mLocationErrorLabel, tr( "Enter location name!" ) );
    }
    else
    {
      if ( QFile::exists( gisdbase() + "/" + location ) )
      {
        button( QWizard::NextButton )->setEnabled( false );
        setError( mLocationErrorLabel, tr( "The location exists!" ) );
      }
    }
  }
}

void QgsGrassNewMapset::existingLocationChanged( const QString &text )
{
  Q_UNUSED( text )
}

void QgsGrassNewMapset::newLocationChanged()
{
  checkLocation();
}

/************************** CRS ******************************/
void QgsGrassNewMapset::setProjectionPage()
{
  setGrassProjection();
}

void QgsGrassNewMapset::sridSelected()
{
  projectionSelected();
}

void QgsGrassNewMapset::projectionSelected()
{
  setGrassProjection();
}

void QgsGrassNewMapset::projRadioSwitched()
{
  if ( mNoProjRadioButton->isChecked() )
  {
    mProjectionSelector->setEnabled( false );
  }
  else
  {
    mProjectionSelector->setEnabled( true );
  }

  projectionSelected();
}

void QgsGrassNewMapset::setGrassProjection()
{
  setError( mProjErrorLabel );

  const QgsCoordinateReferenceSystem crs = mProjectionSelector->crs();

  // Not defined
  if ( mNoProjRadioButton->isChecked() )
  {
    mCellHead.proj = PROJECTION_XY;
    mCellHead.zone = 0;
    mProjInfo = nullptr;
    mProjUnits = nullptr;

    button( QWizard::NextButton )->setEnabled( true );
    return;
  }

  // Define projection
  if ( crs.isValid() )
  {
    const QString wkt = crs.toWkt( Qgis::CrsWktVariant::Preferred );
    QgsDebugMsgLevel( QStringLiteral( "wkt = %1" ).arg( crs.toWkt( Qgis::CrsWktVariant::Preferred ) ), 3 );

    G_TRY
    {
      GPJ_wkt_to_grass( &mCellHead, &mProjInfo, &mProjUnits, wkt.toUtf8().constData(), 0 );
    }
    G_CATCH( QgsGrass::Exception & e )
    {
      QgsGrass::warning( tr( "Cannot set projection: %1" ).arg( e.what() ) );
      return;
    }

    if ( !mProjInfo || !mProjUnits )
    {
      setError( mProjErrorLabel, tr( "Selected projection is not supported by GRASS!" ) );
    }
    else
    {
      mProjSrid = crs.authid().toUpper();
      mProjWkt = wkt;
    }
  }
  else // Nothing selected
  {
    mCellHead.proj = PROJECTION_XY;
    mCellHead.zone = 0;
    mProjInfo = nullptr;
    mProjUnits = nullptr;
    mProjSrid.clear();
    mProjWkt.clear();
  }
  button( QWizard::NextButton )->setEnabled( mProjInfo && mProjUnits );
}

/**************************** REGION ********************************/
void QgsGrassNewMapset::setRegionPage()
{
  // Set defaults
  if ( !mRegionModified )
  {
    setGrassRegionDefaults();
  }

  // Create new projection
  QgsCoordinateReferenceSystem newCrs;
  if ( mProjRadioButton->isChecked() )
  {
    QgsDebugMsgLevel( QString( "selectedCrsId() = %1" ).arg( mProjectionSelector->crs().srsid() ), 2 );

    if ( mProjectionSelector->crs().isValid() )
    {
      mCrs = mProjectionSelector->crs();
    }
  }

  mExtentWidget->setOutputCrs( mCrs );

  // Enable / disable region selection widgets
  if ( mNoProjRadioButton->isChecked() )
  {
    mRegionMap->hide();
    mExtentWidgetFrame->hide();
    mRegionsComboBox->hide();
    mRegionButton->hide();
    mSetRegionFrame->hide();
  }
  else
  {
    mRegionMap->show();
    mExtentWidgetFrame->show();
    mRegionsComboBox->show();
    mRegionButton->show();
    mSetRegionFrame->show();
  }

  checkRegion();

  if ( !mNoProjRadioButton->isChecked() )
  {
    drawRegion();
  }
}

void QgsGrassNewMapset::setGrassRegionDefaults()
{
  QgsDebugMsgLevel( QString( "mCellHead.proj = %1" ).arg( mCellHead.proj ), 3 );

  QgsCoordinateReferenceSystem canvasCrs = mIface->mapCanvas()->mapSettings().destinationCrs();
  QgsDebugMsgLevel( "srs = " + canvasCrs.toWkt(), 3 );

  QgsRectangle ext = mIface->mapCanvas()->extent();
  bool extSet = false;
  if ( ext.xMinimum() < ext.xMaximum() && ext.yMinimum() < ext.yMaximum() )
  {
    extSet = true;
  }

  const QgsCoordinateReferenceSystem selectedCrs = mProjectionSelector->crs();

  QgsRectangle defaultExtent;
  if ( extSet && ( mNoProjRadioButton->isChecked() || ( mProjRadioButton->isChecked() && canvasCrs == selectedCrs ) ) )
  {
    defaultExtent = ext;
  }
  else if ( !selectedCrs.bounds().isEmpty() )
  {
    const QgsRectangle boundsWgs84 = selectedCrs.bounds();
    QgsCoordinateTransform fromWgs84Transform( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) ), selectedCrs, QgsProject::instance()->transformContext() );
    fromWgs84Transform.setBallparkTransformsAreAppropriate( true );

    try
    {
      defaultExtent = fromWgs84Transform.transformBoundingBox( boundsWgs84 );
    }
    catch ( QgsCsException & )
    {
    }
  }
  if ( defaultExtent.isEmpty() )
  {
    if ( mCellHead.proj == PROJECTION_XY )
    {
      defaultExtent = QgsRectangle( 0, 0, 1000, 1000 );
    }
    else if ( mCellHead.proj == PROJECTION_LL )
    {
      defaultExtent = QgsRectangle( -180, -90, 180, 90 );
    }
    else
    {
      defaultExtent = QgsRectangle( -100000, -100000, 100000, 100000 );
    }
  }
  mExtentWidget->setOutputExtentFromUser( defaultExtent, mProjectionSelector->crs() );

  mRegionModified = false;
}

void QgsGrassNewMapset::regionChanged()
{
  mRegionModified = true;
  checkRegion();
  drawRegion();
}

void QgsGrassNewMapset::checkRegion()
{
  bool err = false;

  setError( mRegionErrorLabel );
  button( QWizard::NextButton )->setEnabled( false );

  const QgsRectangle extent = mExtentWidget->outputExtent();
  double n = extent.yMaximum();
  double s = extent.yMinimum();
  double e = extent.xMaximum();
  double w = extent.xMinimum();

  if ( n <= s )
  {
    setError( mRegionErrorLabel, tr( "North must be greater than south" ) );
    err = true;
  }
  if ( e <= w && mCellHead.proj != PROJECTION_LL )
  {
    setError( mRegionErrorLabel, tr( "East must be greater than west" ) );
    err = true;
  }

  if ( err )
    return;

  mCellHead.north = n;
  mCellHead.south = s;
  mCellHead.east = e;
  mCellHead.west = w;
  mCellHead.top = 1.;
  mCellHead.bottom = 0.;

  double res = ( e - w ) / 1000; // reasonable resolution
  double res3 = res / 10.;

  mCellHead.rows = ( int ) ( ( n - s ) / res );
  mCellHead.rows3 = ( int ) ( ( n - s ) / res3 );
  mCellHead.cols = ( int ) ( ( e - w ) / res );
  mCellHead.cols3 = ( int ) ( ( e - w ) / res3 );
  mCellHead.depths = 1;

  mCellHead.ew_res = res;
  mCellHead.ew_res3 = res3;
  mCellHead.ns_res = res;
  mCellHead.ns_res3 = res3;
  mCellHead.tb_res = 1.;
  // Do not override zone, it was set in setGrassProjection()
  //mCellHead.zone = 0;

  button( QWizard::NextButton )->setEnabled( true );
}

void QgsGrassNewMapset::loadRegions()
{
  QString path = QgsApplication::pkgDataPath() + "/grass/locations.gml";
  QgsDebugMsgLevel( QString( "load:%1" ).arg( path.toLocal8Bit().constData() ), 2 );

  QFile file( path );

  if ( !file.exists() )
  {
    QgsGrass::warning( tr( "Regions file (%1) not found." ).arg( path ) );
    return;
  }
  if ( !file.open( QIODevice::ReadOnly ) )
  {
    QgsGrass::warning( tr( "Cannot open locations file (%1)" ).arg( path ) );
    return;
  }

  QDomDocument doc( QStringLiteral( "gml:FeatureCollection" ) );
  QString err;
  int line, column;

  if ( !doc.setContent( &file, &err, &line, &column ) )
  {
    QString errmsg = tr( "Cannot read locations file (%1):" ).arg( path )
                     + tr( "\n%1\nat line %2 column %3" ).arg( err ).arg( line ).arg( column );
    QgsDebugError( errmsg );
    QgsGrass::warning( errmsg );
    file.close();
    return;
  }

  QDomElement docElem = doc.documentElement();
  QDomNodeList nodes = docElem.elementsByTagName( QStringLiteral( "gml:featureMember" ) );

  for ( int i = 0; i < nodes.count(); i++ )
  {
    QDomNode node = nodes.item( i );

    if ( node.isNull() )
    {
      continue;
    }

    QDomElement elem = node.toElement();
    QDomNodeList nameNodes = elem.elementsByTagName( QStringLiteral( "gml:name" ) );
    if ( nameNodes.count() == 0 )
      continue;
    if ( nameNodes.item( 0 ).isNull() )
      continue;

    QDomElement nameElem = nameNodes.item( 0 ).toElement();
    if ( nameElem.text().isNull() )
      continue;

    QDomNodeList envNodes = elem.elementsByTagName( QStringLiteral( "gml:Envelope" ) );
    if ( envNodes.count() == 0 )
      continue;
    if ( envNodes.item( 0 ).isNull() )
      continue;
    QDomElement envElem = envNodes.item( 0 ).toElement();

    QDomNodeList coorNodes = envElem.elementsByTagName( QStringLiteral( "gml:coordinates" ) );
    if ( coorNodes.count() == 0 )
      continue;
    if ( coorNodes.item( 0 ).isNull() )
      continue;
    QDomElement coorElem = coorNodes.item( 0 ).toElement();
    if ( coorElem.text().isNull() )
      continue;

    QStringList coor = coorElem.text().split( QStringLiteral( " " ), Qt::SkipEmptyParts );
    if ( coor.size() != 2 )
    {
      QgsDebugError( QString( "Cannot parse coordinates: %1" ).arg( coorElem.text() ) );
      continue;
    }

    QStringList ll = coor[0].split( QStringLiteral( "," ), Qt::SkipEmptyParts );
    QStringList ur = coor[1].split( QStringLiteral( "," ), Qt::SkipEmptyParts );
    if ( ll.size() != 2 || ur.size() != 2 )
    {
      QgsDebugError( QString( "Cannot parse coordinates: %1" ).arg( coorElem.text() ) );
      continue;
    }

    const QgsRectangle rect( ll[0].toDouble(), ll[1].toDouble(), ur[0].toDouble(), ur[1].toDouble() );

    // Add region
    mRegionsComboBox->addItem( nameElem.text(), QVariant::fromValue( rect ) );
  }
  mRegionsComboBox->setCurrentIndex( -1 );

  file.close();
}

void QgsGrassNewMapset::setSelectedRegion()
{
  if ( mRegionsComboBox->currentIndex() < 0 )
    return;

  const QgsRectangle currentRect = mRegionsComboBox->currentData().value<QgsRectangle>();

  std::vector<QgsPointXY> points;
  // corners ll lr ur ul
  points.push_back( QgsPointXY( currentRect.xMinimum(), currentRect.yMinimum() ) );
  points.push_back( QgsPointXY( currentRect.xMaximum(), currentRect.yMinimum() ) );
  points.push_back( QgsPointXY( currentRect.xMaximum(), currentRect.yMaximum() ) );
  points.push_back( QgsPointXY( currentRect.xMinimum(), currentRect.yMaximum() ) );

  // Convert to currently selected coordinate system

  // Warning: seems that crashes if source == dest
  if ( mProjectionSelector->crs().srsid() != GEOCRS_ID )
  {
    const QgsCoordinateReferenceSystem source( QStringLiteral( "EPSG:4326" ) );
    if ( !source.isValid() )
    {
      QgsGrass::warning( tr( "Cannot create Coordinate Reference System" ) );
      return;
    }

    QgsCoordinateReferenceSystem dest = mProjectionSelector->crs();

    if ( !dest.isValid() )
    {
      QgsGrass::warning( tr( "Cannot create Coordinate Reference System" ) );
      return;
    }

    QgsCoordinateTransform trans( source, dest, QgsProject::instance() );

    bool ok = true;
    for ( int i = 0; i < 4; i++ )
    {
      QgsDebugMsgLevel( QString( "%1,%2->" ).arg( points[i].x() ).arg( points[i].y() ), 3 );
      try
      {
        points[i] = trans.transform( points[i] );
        QgsDebugMsgLevel( QString( "%1,%2" ).arg( points[i].x() ).arg( points[i].y() ), 3 );
      }
      catch ( QgsCsException &cse )
      {
        Q_UNUSED( cse )
        QgsDebugError( "Cannot transform point" );
        ok = false;
        break;
      }
    }

    if ( !ok )
    {
      QgsGrass::warning( tr( "Cannot reproject selected region." ) );
      return;
    }
  }

  double n = -90.0, s = 90.0, e = -180.0, w = 180.0;

  if ( mCellHead.proj == PROJECTION_LL )
  {
    n = points[2].y();
    s = points[0].y();
    e = points[1].x();
    w = points[0].x();

    if ( n > 90 )
      n = 90;
    if ( s < -90 )
      s = -90;
#if 0
    if ( e > 180 )
      e = 180;
    if ( w < -180 )
      w = 180;
#endif
  }
  else
  {
    for ( int i = 0; i < 4; i++ )
    {
      if ( i == 0 || points[i].y() > n )
        n = points[i].y();
      if ( i == 0 || points[i].y() < s )
        s = points[i].y();
      if ( i == 0 || points[i].x() > e )
        e = points[i].x();
      if ( i == 0 || points[i].x() < w )
        w = points[i].x();
    }
  }

  const QgsRectangle newRegion( e, s, w, n );
  mExtentWidget->setOutputExtentFromUser( newRegion, mProjectionSelector->crs() );

  mRegionModified = true;
  checkRegion();
  drawRegion();
}

void QgsGrassNewMapset::setCurrentRegion()
{
  mRegionModified = true;
  checkRegion();
  drawRegion();
  QgsDebugMsgLevel( "setCurrentRegion - End", 3 );
}

void QgsGrassNewMapset::clearRegion()
{
  QPixmap pm = mPixmap;
  mRegionMap->setPixmap( pm );
}

void QgsGrassNewMapset::drawRegion()
{
  QPixmap pm = mPixmap;
  mRegionMap->setPixmap( pm );

  if ( mCellHead.proj == PROJECTION_XY )
    return;

  QgsDebugMsgLevel( QString( "pm.isNull() = %1" ).arg( pm.isNull() ), 3 );
  QPainter p( &pm );
  p.setPen( QPen( QColor( 255, 0, 0 ), 3 ) );

  const QgsRectangle extent = mExtentWidget->outputExtent();
  double n = extent.yMaximum();
  double s = extent.yMinimum();
  double e = extent.xMaximum();
  double w = extent.xMinimum();

  // Shift if LL and W > E
  if ( mCellHead.proj == PROJECTION_LL && w > e )
  {
    if ( ( 180 - w ) < ( e + 180 ) )
    {
      w -= 360;
    }
    else
    {
      e += 360;
    }
  }

  QList<QgsPointXY> tpoints; // ll lr ur ul ll
  tpoints << QgsPointXY( w, s );
  tpoints << QgsPointXY( e, s );
  tpoints << QgsPointXY( e, n );
  tpoints << QgsPointXY( w, n );
  tpoints << QgsPointXY( w, s );


  // Because of possible shift +/- 360 in LL we have to split
  // the lines at least in 3 parts
  QList<QgsPointXY> points; //
  for ( int i = 0; i < 4; i++ )
  {
    for ( int j = 0; j < 3; j++ )
    {
      double x = tpoints[i].x();
      double y = tpoints[i].y();
      double dx = ( tpoints[i + 1].x() - x ) / 3;
      double dy = ( tpoints[i + 1].y() - y ) / 3;
      QgsDebugMsgLevel( QString( "dx = %1 x = %2" ).arg( dx ).arg( x + j * dx ), 3 );
      points << QgsPointXY( x + j * dx, y + j * dy );
    }
  }
  points << points[0]; // close polygon

  // Warning: seems that crashes if source == dest
  if ( mProjectionSelector->crs().srsid() != GEOCRS_ID )
  {
    QgsCoordinateReferenceSystem source = mProjectionSelector->crs();

    if ( !source.isValid() )
    {
      QgsGrass::warning( tr( "Cannot create Coordinate Reference System" ) );
      return;
    }

    QgsCoordinateReferenceSystem dest = QgsCoordinateReferenceSystem::fromSrsId( GEOCRS_ID );

    if ( !dest.isValid() )
    {
      QgsGrass::warning( tr( "Cannot create Coordinate Reference System" ) );
      return;
    }

    QgsCoordinateTransform trans( source, dest, QgsProject::instance() );
    trans.setAllowFallbackTransforms( true );
    trans.setBallparkTransformsAreAppropriate( true );

    for ( int i = points.size() - 1; i >= 0; i-- )
    {
      // Warning: I found that with some projections (e.g. Abidjan 1987)
      // if N = 90 or S = -90 the coordinate projected to
      // WGS84 is nonsense (156.983,89.9988 regardless x) ->
      // use 89.9 - for draw it is not so important
      if ( mCellHead.proj == PROJECTION_LL )
      {
        if ( points[i].y() >= 89.9 )
          points[i].setY( 89.9 );
        if ( points[i].y() <= -89.9 )
          points[i].setY( -89.9 );
      }

      QgsDebugMsgLevel( QString( "%1,%2" ).arg( points[i].x() ).arg( points[i].y() ), 3 );

      // exclude points if transformation failed
      try
      {
        points[i] = trans.transform( points[i] );
        QgsDebugMsgLevel( QString( " --> %1,%2" ).arg( points[i].x() ).arg( points[i].y() ), 3 );
      }
      catch ( QgsCsException &cse )
      {
        Q_UNUSED( cse )
        QgsDebugError( "Cannot transform point" );
        points.removeAt( i );
      }
    }

    if ( points.size() < 3 )
    {
      QgsDebugError( "Cannot reproject region." );
      return;
    }
  }

  for ( int shift = -360; shift <= 360; shift += 360 )
  {
    for ( int i = 0; i < points.size() - 1; i++ )
    {
      double x1 = points[i].x();
      double x2 = points[i + 1].x();

      if ( std::fabs( x2 - x1 ) > 150 )
      {
        if ( x2 < x1 )
        {
          x2 += 360;
        }
        else
        {
          x2 -= 360;
        }
      }
      p.drawLine( 180 + shift + static_cast<int>( x1 ), 90 - static_cast<int>( points[i].y() ), 180 + shift + static_cast<int>( x2 ), 90 - static_cast<int>( points[i + 1].y() ) );
    }
  }

  p.end();

  mRegionMap->setPixmap( pm );
}

/**************************** MAPSET ********************************/
void QgsGrassNewMapset::setMapsets()
{
  mMapsetsListView->clear();

  if ( mCreateLocationRadioButton->isChecked() )
  {
    mMapsetsLabel->hide();
    mMapsetsListView->hide();
    return;
  }
  else
  {
    mMapsetsLabel->show();
    mMapsetsListView->show();
  }

  // Get available mapsets
  QString locationPath = mDirectoryWidget->filePath() + "/" + mLocationComboBox->currentText();
  QDir d( locationPath );

  // Add all subdirs containing WIND
  for ( unsigned int i = 0; i < d.count(); i++ )
  {
    if ( d[i] == QLatin1String( "." ) || d[i] == QLatin1String( ".." ) )
      continue;

    QString mapsetPath = locationPath + "/" + d[i];
    QString windPath = mapsetPath + "/WIND";
    QFileInfo mapsetInfo( mapsetPath );

    if ( QFile::exists( windPath ) )
    {
      new QTreeWidgetItem( mMapsetsListView, QStringList() << d[i] << mapsetInfo.owner() );
    }
  }
}

void QgsGrassNewMapset::mapsetChanged()
{
  button( QWizard::NextButton )->setEnabled( false );
  setError( mMapsetErrorLabel );

  QString mapset = mMapsetLineEdit->text().trimmed();

  if ( mapset.isEmpty() )
  {
    //setError( mMapsetErrorLabel, tr( "Enter mapset name." ) );
    return;
  }

  // Check if exists
  if ( mSelectLocationRadioButton->isChecked() )
  {
    QString locationPath = mDirectoryWidget->filePath() + "/" + mLocationComboBox->currentText();
    if ( QFile::exists( locationPath + "/" + mapset ) )
    {
      setError( mMapsetErrorLabel, tr( "The mapset already exists" ) );
    }
    else
    {
      button( QWizard::NextButton )->setEnabled( true );
    }
  }
  else
  {
    button( QWizard::NextButton )->setEnabled( true );
  }
}

/**************************** FINISH ********************************/
void QgsGrassNewMapset::mOpenNewMapsetCheckBox_stateChanged( int state )
{
  Q_UNUSED( state )
  QgsSettings settings;
  settings.setValue( QStringLiteral( "GRASS/newMapsetWizard/openMapset" ), mOpenNewMapsetCheckBox->isChecked() );
}

void QgsGrassNewMapset::setFinishPage()
{
  mDatabaseLabel->setText( tr( "Database" ) + " : " + mDirectoryWidget->filePath() );

  QString location;
  if ( mSelectLocationRadioButton->isChecked() )
  {
    location = mLocationComboBox->currentText();
  }
  else
  {
    location = mLocationLineEdit->text();
  }
  mLocationLabel->setText( tr( "Location" ) + " : " + location );

  mMapsetLabel->setText( tr( "Mapset" ) + " : " + mMapsetLineEdit->text() );
}

void QgsGrassNewMapset::createMapset()
{
  // TODO: handle all possible errors better, especially half created location/mapset

  if ( !gisdbaseExists() )
  {
    QgsDebugMsgLevel( "create gisdbase " + gisdbase(), 3 );
    QDir gisdbaseDir( gisdbase() );
    QString dirName = gisdbaseDir.dirName();
    gisdbaseDir.cdUp();
    if ( !gisdbaseDir.mkdir( dirName ) )
    {
      QgsGrass::warning( tr( "Cannot create new GRASS database directory" ) + gisdbase() );
      return;
    }
  }

  QString location;
  if ( mCreateLocationRadioButton->isChecked() )
  {
    location = mLocationLineEdit->text();

    QgsGrass::setLocation( gisdbase(), location );

    int ret = 0;
    QString error;
    G_TRY
    {
      ret = G_make_location_crs( location.toUtf8().constData(), &mCellHead, mProjInfo, mProjUnits, mProjSrid.toUtf8().constData(), mProjWkt.toUtf8().constData() );
    }
    G_CATCH( QgsGrass::Exception & e )
    {
      Q_UNUSED( e )
      error = QString( e.what() );
    }

    if ( ret != 0 )
    {
      QgsGrass::warning( tr( "Cannot create new location: %1" ).arg( error ) );
      return;
    }

    // Location created -> reset widgets
    setLocations();
    mSelectLocationRadioButton->setChecked( true );
    mLocationComboBox->setItemText( mLocationComboBox->currentIndex(), location );
    mLocationLineEdit->setText( QString() );
    locationRadioSwitched(); // calls also checkLocation()
  }
  else
  {
    location = mLocationComboBox->currentText();
  }

  // Create mapset
  QString mapset = mMapsetLineEdit->text();

  if ( mapset != QLatin1String( "PERMANENT" ) )
  {
    QString error;
    QgsGrass::createMapset( gisdbase(), location, mapset, error );
    if ( !error.isEmpty() )
    {
      QgsGrass::warning( tr( "Cannot create new mapset: %1" ).arg( error ) );
      return;
    }
  }

  if ( mOpenNewMapsetCheckBox->isChecked() )
  {
    QString error = QgsGrass::openMapset(
      mDirectoryWidget->filePath(), location, mapset
    );

    if ( !error.isEmpty() )
    {
      QMessageBox::information( this, tr( "New mapset" ), tr( "New mapset successfully created, but cannot be opened: %1" ).arg( error ) );
    }
    else
    {
      QMessageBox::information( this, tr( "New mapset" ), tr( "New mapset successfully created and set as current working mapset." ) );

      mPlugin->mapsetChanged();
    }
  }
  else
  {
    QMessageBox::information( this, tr( "New mapset" ), tr( "New mapset successfully created" ) );
  }

  deleteLater();
}

void QgsGrassNewMapset::accept()
{
  createMapset();
}

/********************************************************************/
void QgsGrassNewMapset::setError( QLabel *line, const QString &err )
{
  if ( !err.isEmpty() )
  {
    line->setText( err );
    QPalette palette = line->palette();
    palette.setColor( QPalette::WindowText, Qt::red );
    line->setPalette( palette );
    line->show();
  }
  else
  {
    line->setText( QString() );
    line->hide();
  }
}

// Warning: we have to catch key press otherwise QWizard goes always
// to next page if Key_Enter is pressed
void QgsGrassNewMapset::keyPressEvent( QKeyEvent *e )
{
  Q_UNUSED( e )
  // QgsDebugMsgLevel(QString("key = %1").arg(e->key()), 3);
}

void QgsGrassNewMapset::pageSelected( int index )
{
  QgsDebugMsgLevel( QString( "title = %1" ).arg( page( index ) ? page( index )->title() : "(null)" ), 3 );

  switch ( index )
  {
    case Location:
      if ( mPreviousPage == Database )
      {
        setLocationPage();
      }
      break;

    case Crs:
      // Projection selector
      if ( !mProjectionSelector )
      {
        QGridLayout *projectionLayout = new QGridLayout( mProjectionFrame );
        projectionLayout->setContentsMargins( 0, 0, 0, 0 );

        mProjectionSelector = new QgsProjectionSelectionTreeWidget( mProjectionFrame );
        mProjectionSelector->setEnabled( false );
        projectionLayout->addWidget( mProjectionSelector, 0, 0 );

        mProjectionSelector->show();

        connect( mProjectionSelector, &QgsProjectionSelectionTreeWidget::crsSelected, this, &QgsGrassNewMapset::sridSelected );

        QgsCoordinateReferenceSystem srs = mIface->mapCanvas()->mapSettings().destinationCrs();
        QgsDebugMsgLevel( "srs = " + srs.toWkt(), 3 );

        if ( srs.isValid() )
        {
          mProjectionSelector->setCrs( srs );
          mProjRadioButton->setChecked( true );
          projRadioSwitched();
        }
      }
      if ( mPreviousPage == Location )
      {
        setProjectionPage();
      }
      break;

    case Region:
      if ( !mRegionsInited )
      {
        loadRegions();
        mRegionsInited = true;
      }

      if ( mPreviousPage == Crs )
      {
        setRegionPage();
      }

      break;

    case MapSet:
      if ( mPreviousPage == Location || mPreviousPage == Region )
      {
        setMapsets();
        mapsetChanged();
      }
      break;

    case Finish:
      setFinishPage();
      break;
  }
  mPreviousPage = index;
}
