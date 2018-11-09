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

#include "cpl_conv.h"
#include "ogr_srs_api.h"

#include <QCloseEvent>
#include <QFileDialog>
#include <QMessageBox>
#include <QRegExp>
#include <QTextStream>


extern "C"
{
#if defined(_MSC_VER) && defined(M_PI_4)
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

bool QgsGrassNewMapset::sRunning = false;

QgsGrassNewMapset::QgsGrassNewMapset( QgisInterface *iface,
                                      QgsGrassPlugin *plugin, QWidget *parent,
                                      Qt::WindowFlags f )
  : QWizard( parent, f )
  , QgsGrassNewMapsetBase()
{
  QgsDebugMsg( "QgsGrassNewMapset()" );

  setupUi( this );
  connect( mDatabaseButton, &QPushButton::clicked, this, &QgsGrassNewMapset::mDatabaseButton_clicked );
  connect( mDatabaseLineEdit, &QLineEdit::returnPressed, this, &QgsGrassNewMapset::mDatabaseLineEdit_returnPressed );
  connect( mDatabaseLineEdit, &QLineEdit::textChanged, this, &QgsGrassNewMapset::mDatabaseLineEdit_textChanged );
  connect( mCreateLocationRadioButton, &QRadioButton::clicked, this, &QgsGrassNewMapset::mCreateLocationRadioButton_clicked );
  connect( mSelectLocationRadioButton, &QRadioButton::clicked, this, &QgsGrassNewMapset::mSelectLocationRadioButton_clicked );
  connect( mLocationComboBox, &QComboBox::editTextChanged, this, &QgsGrassNewMapset::mLocationComboBox_textChanged );
  connect( mLocationLineEdit, &QLineEdit::returnPressed, this, &QgsGrassNewMapset::mLocationLineEdit_returnPressed );
  connect( mLocationLineEdit, &QLineEdit::textChanged, this, &QgsGrassNewMapset::mLocationLineEdit_textChanged );
  connect( mNoProjRadioButton, &QRadioButton::clicked, this, &QgsGrassNewMapset::mNoProjRadioButton_clicked );
  connect( mProjRadioButton, &QRadioButton::clicked, this, &QgsGrassNewMapset::mProjRadioButton_clicked );
  connect( mNorthLineEdit, &QLineEdit::returnPressed, this, &QgsGrassNewMapset::mNorthLineEdit_returnPressed );
  connect( mNorthLineEdit, &QLineEdit::textChanged, this, &QgsGrassNewMapset::mNorthLineEdit_textChanged );
  connect( mSouthLineEdit, &QLineEdit::returnPressed, this, &QgsGrassNewMapset::mSouthLineEdit_returnPressed );
  connect( mSouthLineEdit, &QLineEdit::textChanged, this, &QgsGrassNewMapset::mSouthLineEdit_textChanged );
  connect( mEastLineEdit, &QLineEdit::returnPressed, this, &QgsGrassNewMapset::mEastLineEdit_returnPressed );
  connect( mEastLineEdit, &QLineEdit::textChanged, this, &QgsGrassNewMapset::mEastLineEdit_textChanged );
  connect( mWestLineEdit, &QLineEdit::returnPressed, this, &QgsGrassNewMapset::mWestLineEdit_returnPressed );
  connect( mWestLineEdit, &QLineEdit::textChanged, this, &QgsGrassNewMapset::mWestLineEdit_textChanged );
  connect( mCurrentRegionButton, &QPushButton::clicked, this, &QgsGrassNewMapset::mCurrentRegionButton_clicked );
  connect( mRegionButton, &QPushButton::clicked, this, &QgsGrassNewMapset::mRegionButton_clicked );
  connect( mMapsetLineEdit, &QLineEdit::returnPressed, this, &QgsGrassNewMapset::mMapsetLineEdit_returnPressed );
  connect( mMapsetLineEdit, &QLineEdit::textChanged, this, &QgsGrassNewMapset::mMapsetLineEdit_textChanged );
  connect( mOpenNewMapsetCheckBox, &QCheckBox::stateChanged, this, &QgsGrassNewMapset::mOpenNewMapsetCheckBox_stateChanged );
#ifdef Q_OS_MAC
  setWizardStyle( QWizard::ClassicStyle );
#endif

  sRunning = true;
  mIface = iface;
  mProjectionSelector = nullptr;
  mPreviousPage = -1;
  mRegionModified = false;

  QString mapPath = QStringLiteral( ":/images/grass/world.png" );
  QgsDebugMsg( QString( "mapPath = %1" ).arg( mapPath ) );

  //mPixmap = QPixmap( *(mRegionMap->pixmap()) );
  mPixmap.load( mapPath );
  QgsDebugMsg( QString( "mPixmap.isNull() = %1" ).arg( mPixmap.isNull() ) );

  mRegionsInited = false;
  mPlugin = plugin;

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
  mDatabaseLineEdit->setText( gisdbase );
  databaseChanged();

  // LOCATION
  QRegExp rx( "[A-Za-z0-9_.]+" );
  mLocationLineEdit->setValidator( new QRegExpValidator( rx, mLocationLineEdit ) );

  // CRS

  // MAPSET
  mMapsetsListView->clear();
  mMapsetLineEdit->setValidator( new QRegExpValidator( rx, mMapsetLineEdit ) );

  mMapsetsListView->header()->setSectionResizeMode( QHeaderView::ResizeToContents );

  // FINISH
  mOpenNewMapsetCheckBox->setChecked( settings.value( QStringLiteral( "GRASS/newMapsetWizard/openMapset" ), true ).toBool() );
  connect( this, &QWizard::currentIdChanged, this, &QgsGrassNewMapset::pageSelected );

  restoreGeometry( settings.value( QStringLiteral( "Windows/QgsGrassNewMapset/geometry" ) ).toByteArray() );
}

QgsGrassNewMapset::~QgsGrassNewMapset()
{
  QgsSettings settings;
  settings.setValue( QStringLiteral( "Windows/QgsGrassNewMapset/geometry" ), saveGeometry() );
  sRunning = false;
}
/*************************** DATABASE *******************************/
void QgsGrassNewMapset::browseDatabase()
{
  QString selectedDir = QFileDialog::getExistingDirectory( this, nullptr, mDatabaseLineEdit->text() );
  if ( selectedDir.isEmpty() )
  {
    return;
  }

  mDatabaseLineEdit->setText( selectedDir );
  databaseChanged();
}

void QgsGrassNewMapset::databaseChanged()
{

  QgsSettings settings;
  settings.setValue( QStringLiteral( "GRASS/lastGisdbase" ), mDatabaseLineEdit->text() );

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

QString QgsGrassNewMapset::gisdbase()
{
  return mDatabaseLineEdit->text().trimmed();
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

      QString windName = mDatabaseLineEdit->text() + "/" + gisdbaseDir[i] + "/PERMANENT/DEFAULT_WIND";
      QString locationName = mDatabaseLineEdit->text() + "/" + gisdbaseDir[i];
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
      FALLTHROUGH
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
  Q_UNUSED( text );
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

  QString proj4 = mProjectionSelector->crs().toProj4();

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
  if ( !proj4.isEmpty() )
  {
    QgsDebugMsg( QString( "proj4 = %1" ).arg( proj4.toLocal8Bit().constData() ) );

    OGRSpatialReferenceH hCRS = nullptr;
    hCRS = OSRNewSpatialReference( nullptr );
    int errcode;

    {
      QgsLocaleNumC l;
      errcode = OSRImportFromProj4( hCRS, proj4.toUtf8() );
    }

    if ( errcode != OGRERR_NONE )
    {
      QgsDebugMsg( QString( "OGR can't parse PROJ-style parameter string:\n%1\nOGR Error code was %2" ).arg( proj4 ).arg( errcode ) );

      mCellHead.proj = PROJECTION_XY;
      mCellHead.zone = 0;
      mProjInfo = nullptr;
      mProjUnits = nullptr;
    }
    else
    {
      char *wkt = nullptr;

      QgsDebugMsg( QString( "OSRIsGeographic = %1" ).arg( OSRIsGeographic( hCRS ) ) );
      QgsDebugMsg( QString( "OSRIsProjected = %1" ).arg( OSRIsProjected( hCRS ) ) );

      if ( ( errcode = OSRExportToWkt( hCRS, &wkt ) ) != OGRERR_NONE )
      {
        QgsDebugMsg( QString( "OGR can't get Wkt-style parameter string\nOGR Error code was %1" ).arg( errcode ) );
      }
      else
      {
        QgsDebugMsg( QString( "wkt = %1" ).arg( wkt ) );
      }

      // Note: GPJ_osr_to_grass() defaults in PROJECTION_XY if projection
      //       cannot be set

      // There was a bug in GRASS, it is present in 6.0.x line
      int ret = GPJ_wkt_to_grass( &mCellHead, &mProjInfo, &mProjUnits, wkt, 0 );

      // Note: It seems that GPJ_osr_to_grass()returns always 1,
      //   -> test if mProjInfo was set

      Q_UNUSED( ret );
      QgsDebugMsg( QString( "ret = %1" ).arg( ret ) );
      QgsDebugMsg( QString( "mProjInfo = %1" ).arg( QString::number( ( qulonglong )mProjInfo, 16 ).toLocal8Bit().constData() ) );

      CPLFree( wkt );
    }

    if ( !mProjInfo || !mProjUnits )
    {
      setError( mProjErrorLabel, tr( "Selected projection is not supported by GRASS!" ) );
    }
  }
  else // Nothing selected
  {
    mCellHead.proj = PROJECTION_XY;
    mCellHead.zone = 0;
    mProjInfo = nullptr;
    mProjUnits = nullptr;
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
    QgsDebugMsg( QString( "selectedCrsId() = %1" ).arg( mProjectionSelector->crs().srsid() ) );

    if ( mProjectionSelector->crs().srsid() > 0 )
    {
      newCrs = mProjectionSelector->crs();
      if ( ! newCrs.isValid() )
      {
        QgsGrass::warning( tr( "Cannot create projection." ) );
      }
    }
  }

  // Reproject previous region if it was modified
  // and if previous and current projection is valid
  if ( mRegionModified && newCrs.isValid() && mCrs.isValid()
       && newCrs.srsid() != mCrs.srsid() )
  {
    QgsCoordinateTransform trans( mCrs, newCrs, QgsProject::instance() );

    double n = mNorthLineEdit->text().toDouble();
    double s = mSouthLineEdit->text().toDouble();
    double e = mEastLineEdit->text().toDouble();
    double w = mWestLineEdit->text().toDouble();

    std::vector<QgsPointXY> points;

    // TODO: this is not perfect
    points.push_back( QgsPointXY( w, s ) );
    points.push_back( QgsPointXY( e, n ) );

    bool ok = true;
    for ( int i = 0; i < 2; i++ )
    {
      try
      {
        points[i] = trans.transform( points[i] );
      }
      catch ( QgsCsException &cse )
      {
        Q_UNUSED( cse );
        QgsDebugMsg( "Cannot transform point" );
        ok = false;
        break;
      }
    }

    if ( ok )
    {
      int precision = newCrs.mapUnits() == QgsUnitTypes::DistanceDegrees ? 6 : 1;
      mNorthLineEdit->setText( qgsDoubleToString( points[1].y(), precision ) );
      mSouthLineEdit->setText( qgsDoubleToString( points[0].y(), precision ) );
      mEastLineEdit->setText( qgsDoubleToString( points[1].x(), precision ) );
      mWestLineEdit->setText( qgsDoubleToString( points[0].x(), precision ) );
    }
    else
    {
      QgsGrass::warning( tr( "Cannot reproject previously set region, default region set." ) );
      setGrassRegionDefaults();
    }
  }

  // Set current region projection
  mCrs = newCrs;

  // Enable / disable region selection widgets
  if ( mNoProjRadioButton->isChecked() )
  {
    mRegionMap->hide();
    mCurrentRegionButton->hide();
    mRegionsComboBox->hide();
    mRegionButton->hide();
    mSetRegionFrame->hide();
  }
  else
  {
    mRegionMap->show();
    mCurrentRegionButton->show();
    mRegionsComboBox->show();
    mRegionButton->show();
    mSetRegionFrame->show();

    QgsRectangle ext = mIface->mapCanvas()->extent();

    mCurrentRegionButton->setEnabled( !ext.isEmpty() );
  }

  checkRegion();

  if ( !mNoProjRadioButton->isChecked() )
  {
    drawRegion();
  }
}

void QgsGrassNewMapset::setGrassRegionDefaults()
{
  QgsDebugMsg( QString( "mCellHead.proj = %1" ).arg( mCellHead.proj ) );

  QgsCoordinateReferenceSystem srs = mIface->mapCanvas()->mapSettings().destinationCrs();
  QgsDebugMsg( "srs = " + srs.toWkt() );

  QgsRectangle ext = mIface->mapCanvas()->extent();
  bool extSet = false;
  if ( ext.xMinimum() < ext.xMaximum() && ext.yMinimum() < ext.yMaximum() )
  {
    extSet = true;
  }

  if ( extSet &&
       ( mNoProjRadioButton->isChecked() ||
         ( mProjRadioButton->isChecked()
           && srs.srsid() == mProjectionSelector->crs().srsid() )
       )
     )
  {
    mNorthLineEdit->setText( QString::number( ext.yMaximum() ) );
    mSouthLineEdit->setText( QString::number( ext.yMinimum() ) );
    mEastLineEdit->setText( QString::number( ext.xMaximum() ) );
    mWestLineEdit->setText( QString::number( ext.xMinimum() ) );
  }
  else if ( mCellHead.proj == PROJECTION_XY )
  {
    mNorthLineEdit->setText( QStringLiteral( "1000" ) );
    mSouthLineEdit->setText( QStringLiteral( "0" ) );
    mEastLineEdit->setText( QStringLiteral( "1000" ) );
    mWestLineEdit->setText( QStringLiteral( "0" ) );
  }
  else if ( mCellHead.proj == PROJECTION_LL )
  {
    mNorthLineEdit->setText( QStringLiteral( "90" ) );
    mSouthLineEdit->setText( QStringLiteral( "-90" ) );
    mEastLineEdit->setText( QStringLiteral( "180" ) );
    mWestLineEdit->setText( QStringLiteral( "-180" ) );
  }
  else
  {
    mNorthLineEdit->setText( QStringLiteral( "100000" ) );
    mSouthLineEdit->setText( QStringLiteral( "-100000" ) );
    mEastLineEdit->setText( QStringLiteral( "100000" ) );
    mWestLineEdit->setText( QStringLiteral( "-100000" ) );
  }
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

  if ( mNorthLineEdit->text().trimmed().length() == 0
       || mSouthLineEdit->text().trimmed().length() == 0
       || mEastLineEdit->text().trimmed().length() == 0
       || mWestLineEdit->text().trimmed().length() == 0 )
  {
    return;
  }

  double n = mNorthLineEdit->text().toDouble();
  double s = mSouthLineEdit->text().toDouble();
  double e = mEastLineEdit->text().toDouble();
  double w = mWestLineEdit->text().toDouble();

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

  mCellHead.rows   = ( int )( ( n - s ) / res );
  mCellHead.rows3  = ( int )( ( n - s ) / res3 );
  mCellHead.cols   = ( int )( ( e - w ) / res );
  mCellHead.cols3  = ( int )( ( e - w ) / res3 );
  mCellHead.depths = 1;

  mCellHead.ew_res  = res;
  mCellHead.ew_res3 = res3;
  mCellHead.ns_res  = res;
  mCellHead.ns_res3 = res3;
  mCellHead.tb_res  = 1.;
  // Do not override zone, it was set in setGrassProjection()
  //mCellHead.zone = 0;

  button( QWizard::NextButton )->setEnabled( true );
}

void QgsGrassNewMapset::loadRegions()
{

  QString path = QgsApplication::pkgDataPath() + "/grass/locations.gml";
  QgsDebugMsg( QString( "load:%1" ).arg( path.toLocal8Bit().constData() ) );

  QFile file( path );

  if ( !file.exists() )
  {
    QgsGrass::warning( tr( "Regions file (%1) not found." ).arg( path ) );
    return;
  }
  if ( ! file.open( QIODevice::ReadOnly ) )
  {
    QgsGrass::warning( tr( "Cannot open locations file (%1)" ).arg( path ) );
    return;
  }

  QDomDocument doc( QStringLiteral( "gml:FeatureCollection" ) );
  QString err;
  int line, column;

  if ( !doc.setContent( &file,  &err, &line, &column ) )
  {
    QString errmsg = tr( "Cannot read locations file (%1):" ).arg( path )
                     + tr( "\n%1\nat line %2 column %3" ).arg( err ).arg( line ).arg( column );
    QgsDebugMsg( errmsg );
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

    QStringList coor = coorElem.text().split( QStringLiteral( " " ), QString::SkipEmptyParts );
    if ( coor.size() != 2 )
    {
      QgsDebugMsg( QString( "Cannot parse coordinates: %1" ).arg( coorElem.text() ) );
      continue;
    }

    QStringList ll = coor[0].split( QStringLiteral( "," ), QString::SkipEmptyParts );
    QStringList ur = coor[1].split( QStringLiteral( "," ), QString::SkipEmptyParts );
    if ( ll.size() != 2 || ur.size() != 2 )
    {
      QgsDebugMsg( QString( "Cannot parse coordinates: %1" ).arg( coorElem.text() ) );
      continue;
    }

    // Add region
    mRegionsComboBox->addItem( nameElem.text() );

    QgsPointXY llp( ll[0].toDouble(), ll[1].toDouble() );
    mRegionsPoints.push_back( llp );
    QgsPointXY urp( ur[0].toDouble(), ur[1].toDouble() );
    mRegionsPoints.push_back( urp );
  }
  mRegionsComboBox->setCurrentIndex( -1 );

  file.close();
}

void QgsGrassNewMapset::setSelectedRegion()
{

  // mRegionsPoints are in EPSG 4326 = LL WGS84
  int index = 2 * mRegionsComboBox->currentIndex();

  std::vector<QgsPointXY> points;
  // corners ll lr ur ul
  points.push_back( QgsPointXY( mRegionsPoints[index] ) );
  points.push_back( QgsPointXY( mRegionsPoints[index + 1].x(),
                                mRegionsPoints[index].y() ) );
  points.push_back( QgsPointXY( mRegionsPoints[index + 1] ) );
  points.push_back( QgsPointXY( mRegionsPoints[index].x(),
                                mRegionsPoints[index + 1].y() ) );

  // Convert to currently selected coordinate system


  // Warning: seems that crashes if source == dest
  if ( mProjectionSelector->crs().srsid() != GEOCRS_ID )
  {
    // Warning: QgsCoordinateReferenceSystem::EpsgCrsId is broken (using epsg_id)
    //QgsCoordinateReferenceSystem source ( 4326, QgsCoordinateReferenceSystem::EpsgCrsId );
    QgsCoordinateReferenceSystem source = QgsCoordinateReferenceSystem::fromSrsId( GEOCRS_ID );

    if ( !source.isValid() )
    {
      QgsGrass::warning( tr( "Cannot create QgsCoordinateReferenceSystem" ) );
      return;
    }

    QgsCoordinateReferenceSystem dest = mProjectionSelector->crs();

    if ( !dest.isValid() )
    {
      QgsGrass::warning( tr( "Cannot create QgsCoordinateReferenceSystem" ) );
      return;
    }

    QgsCoordinateTransform trans( source, dest, QgsProject::instance() );

    bool ok = true;
    for ( int i = 0; i < 4; i++ )
    {
      QgsDebugMsg( QString( "%1,%2->" ).arg( points[i].x() ).arg( points[i].y() ) );
      try
      {
        points[i] = trans.transform( points[i] );
        QgsDebugMsg( QString( "%1,%2" ).arg( points[i].x() ).arg( points[i].y() ) );
      }
      catch ( QgsCsException &cse )
      {
        Q_UNUSED( cse );
        QgsDebugMsg( "Cannot transform point" );
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

  mNorthLineEdit->setText( QString::number( n ) );
  mSouthLineEdit->setText( QString::number( s ) );
  mEastLineEdit->setText( QString::number( e ) );
  mWestLineEdit->setText( QString::number( w ) );

  mRegionModified = true;
  checkRegion();
  drawRegion();
}

void QgsGrassNewMapset::setCurrentRegion()
{

  QgsRectangle ext = mIface->mapCanvas()->extent();

  QgsCoordinateReferenceSystem srs = mIface->mapCanvas()->mapSettings().destinationCrs();
  QgsDebugMsg( "srs = " + srs.toWkt() );

  std::vector<QgsPointXY> points;

  // TODO: this is not perfect
  points.push_back( QgsPointXY( ext.xMinimum(), ext.yMinimum() ) );
  points.push_back( QgsPointXY( ext.xMaximum(), ext.yMaximum() ) );

  // TODO add a method, this code is copy-paste from setSelectedRegion
  if ( srs.isValid() && mCrs.isValid()
       && srs.srsid() != mCrs.srsid() )
  {
    QgsCoordinateTransform trans( srs, mCrs, QgsProject::instance() );

    bool ok = true;
    for ( int i = 0; i < 2; i++ )
    {
      try
      {
        points[i] = trans.transform( points[i] );
      }
      catch ( QgsCsException &cse )
      {
        Q_UNUSED( cse );
        QgsDebugMsg( "Cannot transform point" );
        ok = false;
        break;
      }
    }

    if ( !ok )
    {
      QgsGrass::warning( tr( "Cannot reproject region" ) );
      return;
    }
  }
  mNorthLineEdit->setText( QString::number( points[1].y() ) );
  mSouthLineEdit->setText( QString::number( points[0].y() ) );
  mEastLineEdit->setText( QString::number( points[1].x() ) );
  mWestLineEdit->setText( QString::number( points[0].x() ) );

  mRegionModified = true;
  checkRegion();
  drawRegion();
  QgsDebugMsg( "setCurrentRegion - End" );
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

  QgsDebugMsg( QString( "pm.isNull() = %1" ).arg( pm.isNull() ) );
  QPainter p( &pm );
  p.setPen( QPen( QColor( 255, 0, 0 ), 3 ) );

  double n = mNorthLineEdit->text().toDouble();
  double s = mSouthLineEdit->text().toDouble();
  double e = mEastLineEdit->text().toDouble();
  double w = mWestLineEdit->text().toDouble();

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
      QgsDebugMsg( QString( "dx = %1 x = %2" ).arg( dx ).arg( x + j * dx ) );
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
      QgsGrass::warning( tr( "Cannot create QgsCoordinateReferenceSystem" ) );
      return;
    }

    QgsCoordinateReferenceSystem dest = QgsCoordinateReferenceSystem::fromSrsId( GEOCRS_ID );

    if ( !dest.isValid() )
    {
      QgsGrass::warning( tr( "Cannot create QgsCoordinateReferenceSystem" ) );
      return;
    }

    QgsCoordinateTransform trans( source, dest, QgsProject::instance() );

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

      QgsDebugMsg( QString( "%1,%2" ).arg( points[i].x() ).arg( points[i].y() ) );

      // exclude points if transformation failed
      try
      {
        points[i] = trans.transform( points[i] );
        QgsDebugMsg( QString( " --> %1,%2" ).arg( points[i].x() ).arg( points[i].y() ) );
      }
      catch ( QgsCsException &cse )
      {
        Q_UNUSED( cse );
        QgsDebugMsg( "Cannot transform point" );
        points.removeAt( i );
      }
    }

    if ( points.size() < 3 )
    {
      QgsDebugMsg( "Cannot reproject region." );
      return;
    }
  }

  for ( int shift = -360; shift <= 360; shift += 360 )
  {
    for ( int i = 0; i < 12; i++ )
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
      p.drawLine( 180 + shift + static_cast<int>( x1 ), 90 - static_cast<int>( points[i].y() ),
                  180 + shift + static_cast<int>( x2 ), 90 - static_cast<int>( points[i + 1].y() ) );
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
  QString locationPath = mDatabaseLineEdit->text() + "/" + mLocationComboBox->currentText();
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
    QString locationPath = mDatabaseLineEdit->text() + "/" + mLocationComboBox->currentText();
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
  Q_UNUSED( state );
  QgsSettings settings;
  settings.setValue( QStringLiteral( "GRASS/newMapsetWizard/openMapset" ), mOpenNewMapsetCheckBox->isChecked() );
}

void QgsGrassNewMapset::setFinishPage()
{

  mDatabaseLabel->setText( tr( "Database" ) + " : " + mDatabaseLineEdit->text() );

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
    QgsDebugMsg( "create gisdbase " + gisdbase() );
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
      ret = G_make_location( location.toUtf8().constData(), &mCellHead, mProjInfo, mProjUnits );
    }
    G_CATCH( QgsGrass::Exception & e )
    {
      Q_UNUSED( e );
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
                      mDatabaseLineEdit->text(), location, mapset );

    if ( !error.isEmpty() )
    {
      QMessageBox::information( this, tr( "New mapset" ),
                                tr( "New mapset successfully created, but cannot be opened: %1" ).arg( error ) );
    }
    else
    {
      QMessageBox::information( this, tr( "New mapset" ),
                                tr( "New mapset successfully created and set as current working mapset." ) );

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
  Q_UNUSED( e );
// QgsDebugMsg(QString("key = %1").arg(e->key()));
}

void QgsGrassNewMapset::pageSelected( int index )
{
  QgsDebugMsg( QString( "title = %1" ).arg( page( index ) ? page( index )->title() : "(null)" ) );

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

        mProjectionSelector = new QgsProjectionSelectionTreeWidget( mProjectionFrame );
        mProjectionSelector->setEnabled( false );
        projectionLayout->addWidget( mProjectionSelector, 0, 0 );

        mProjectionSelector->show();

        connect( mProjectionSelector, &QgsProjectionSelectionTreeWidget::crsSelected,
                 this, &QgsGrassNewMapset::sridSelected );

        QgsCoordinateReferenceSystem  srs = mIface->mapCanvas()->mapSettings().destinationCrs();
        QgsDebugMsg( "srs = " + srs.toWkt() );

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

bool QgsGrassNewMapset::isRunning( void )
{
  return sRunning;
}

void QgsGrassNewMapset::close( void )
{

  hide();
  sRunning = false;
  deleteLater();
}

void QgsGrassNewMapset::closeEvent( QCloseEvent *e )
{

  e->accept();
  close();
}
