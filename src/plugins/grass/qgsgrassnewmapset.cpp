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

#include "qgisinterface.h"
#include "qgsapplication.h"
#include "qgscoordinatetransform.h"
#include "qgslogger.h"
#include "qgsmapcanvas.h"
#include "qgsproject.h"
#include "qgsprojectionselector.h"

#include <QCloseEvent>
#include <QFileDialog>
#include <QMessageBox>
#include <QRegExp>
#include <QSettings>
#include <QTextStream>


extern "C"
{
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

bool QgsGrassNewMapset::mRunning = false;

QgsGrassNewMapset::QgsGrassNewMapset( QgisInterface *iface,
                                      QgsGrassPlugin *plugin, QWidget * parent,
                                      Qt::WindowFlags f ) :
    QWizard( parent, f ),
    QgsGrassNewMapsetBase()
{
  QgsDebugMsg( "QgsGrassNewMapset()" );

  setupUi( this );
#ifdef Q_OS_MAC
  setWizardStyle( QWizard::ClassicStyle );
#endif

  mRunning = true;
  mIface = iface;
  mProjectionSelector = 0;
  mPreviousPage = -1;
  mRegionModified = false;

  QString mapPath = ":/images/grass/world.png";
  QgsDebugMsg( QString( "mapPath = %1" ).arg( mapPath ) );

  //mPixmap = QPixmap( *(mRegionMap->pixmap()) );
  mPixmap.load( mapPath );
  QgsDebugMsg( QString( "mPixmap.isNull() = %1" ).arg( mPixmap.isNull() ) );

  mRegionsInited = false;
  mPlugin = plugin;

  setError( mDatabaseErrorLabel, "" );
  setError( mLocationErrorLabel, "" );
  setError( mProjErrorLabel, "" );
  setError( mRegionErrorLabel, "" );
  setError( mMapsetErrorLabel, "" );

  const QColor& paletteBackgroundColor = palette().color( backgroundRole() );
  QPalette palette = mDatabaseText->palette();
  palette.setColor( mDatabaseText->backgroundRole(), paletteBackgroundColor );
  mDatabaseText->setPalette( palette );
  palette = mLocationText->palette();
  palette.setColor( mLocationText->backgroundRole(), paletteBackgroundColor );
  mLocationText->setPalette( palette );
  palette = mRegionText->palette();
  palette.setColor( mRegionText->backgroundRole(), paletteBackgroundColor );
  mRegionText->setPalette( palette );
  palette = mMapsetText->palette();
  palette.setColor( mMapsetText->backgroundRole(), paletteBackgroundColor );
  mMapsetText->setPalette( palette );

  // DATABASE
  QSettings settings;
  QString db = settings.value( "/GRASS/lastGisdbase" ).toString();
  if ( !db.isNull() )
  {
    mDatabaseLineEdit->setText( db );
  }
  else
  {
    mDatabaseLineEdit->setText( QDir::currentPath() );
  }
  databaseChanged();

  // Create example tree structure
  mTreeListView->clear();
  QTreeWidgetItem *dbi = new QTreeWidgetItem( mTreeListView, QStringList() << "OurDatabase" << tr( "Database" ) );
  dbi->setExpanded( true );

  QTreeWidgetItem *l = new QTreeWidgetItem( dbi, QStringList() << "Mexico" << tr( "Location 1" ) );
  l->setExpanded( true );
  QTreeWidgetItem *m = new QTreeWidgetItem( l, QStringList() << "PERMANENT" << tr( "System mapset" ) );
  m->setExpanded( true );
  m = new QTreeWidgetItem( l, QStringList() << "Alejandra" << tr( "User's mapset" ) );
  m->setExpanded( true );
  m = new QTreeWidgetItem( l, QStringList() << "Juan" << tr( "User's mapset" ) );
  m->setExpanded( true );

  l = new QTreeWidgetItem( dbi, QStringList() << "New Zealand" << tr( "Location 2" ) );
  l->setExpanded( true );
  m = new QTreeWidgetItem( l, QStringList() << "PERMANENT" << tr( "System mapset" ) );
  m->setExpanded( true );
  m = new QTreeWidgetItem( l, QStringList() << "Cimrman" << tr( "User's mapset" ) );
  m->setExpanded( true );

  // LOCATION
  QRegExp rx;
  rx.setPattern( "[A-Za-z0-9_.]+" );
  mLocationLineEdit->setValidator( new QRegExpValidator( rx, mLocationLineEdit ) );

  // CRS

  // MAPSET
  mMapsetsListView->clear();
  mMapsetLineEdit->setValidator( new QRegExpValidator( rx, mMapsetLineEdit ) );

  // FINISH

  connect( this, SIGNAL( currentIdChanged( int ) ),
           this, SLOT( pageSelected( int ) ) );
}

QgsGrassNewMapset::~QgsGrassNewMapset()
{
  QgsDebugMsg( "entered." );

  mRunning = false;
}
/*************************** DATABASE *******************************/
void QgsGrassNewMapset::browseDatabase()
{
  QString selectedDir = QFileDialog::getExistingDirectory( this, NULL, mDatabaseLineEdit->text() );
  if ( selectedDir.isEmpty() )
    return;

  mDatabaseLineEdit->setText( selectedDir );
  databaseChanged();
}

void QgsGrassNewMapset::databaseChanged()
{
  QgsDebugMsg( "entered." );
  // TODO: reset next tabs
  //
  QSettings settings;
  settings.setValue( "/GRASS/lastGisdbase", mDatabaseLineEdit->text() );

  button( QWizard::NextButton )->setEnabled( false );
  setError( mDatabaseErrorLabel, "" );

  QString database = mDatabaseLineEdit->text().trimmed();

  if ( database.length() == 0 )
  {
    setError( mDatabaseErrorLabel, tr( "Enter path to GRASS database" ) );
    return;
  }

  QFileInfo databaseInfo( mDatabaseLineEdit->text() );

  if ( !databaseInfo.exists() )
  {
    setError( mDatabaseErrorLabel, tr( "The directory doesn't exist!" ) );
    return;
  }

  // Check if at least one writable location exists or
  // database is writable
  bool locationExists = false;
  QDir d( mDatabaseLineEdit->text() );
  for ( unsigned int i = 0; i < d.count(); i++ )
  {
    if ( d[i] == "." || d[i] == ".." )
      continue;

    QString windName = mDatabaseLineEdit->text() + "/" + d[i] + "/PERMANENT/DEFAULT_WIND";
    QString locationName = mDatabaseLineEdit->text() + "/" + d[i];
    QFileInfo locationInfo( locationName );

    if ( QFile::exists( windName ) && locationInfo.isWritable() )
    {
      locationExists = true;
      break;
    }
  }

  if ( locationExists || databaseInfo.isWritable() )
  {
    button( QWizard::NextButton )->setEnabled( true );
  }
  else
  {
    setError( mDatabaseErrorLabel, tr( "No writable locations, the database is not writable!" ) );
  }
}

/*************************** LOCATION *******************************/
void QgsGrassNewMapset::setLocationPage()
{
  QgsDebugMsg( "entered." );

  setLocations();
}

void QgsGrassNewMapset::setLocations()
{
  QgsDebugMsg( "entered." );

  mLocationComboBox->clear();

  QSettings settings;
  QString lastLocation = settings.value( "/GRASS/lastLocation" ).toString();

  // Get available locations with write permissions
  QDir d( mDatabaseLineEdit->text() );

  // Add all subdirs containing PERMANENT/DEFAULT_WIND
  int idx = 0;
  int sel = -1;
  for ( unsigned int i = 0; i < d.count(); i++ )
  {
    if ( d[i] == "." || d[i] == ".." )
      continue;

    QString windName = mDatabaseLineEdit->text() + "/" + d[i] + "/PERMANENT/DEFAULT_WIND";
    QString locationName = mDatabaseLineEdit->text() + "/" + d[i];
    QFileInfo locationInfo( locationName );

    if ( QFile::exists( windName ) && locationInfo.isWritable() )
    {
      mLocationComboBox->insertItem( -1, QString( d[i] ) );
      if ( QString( d[i] ) == lastLocation )
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
    case LOCATION:
      if ( mSelectLocationRadioButton->isChecked() )
      {
        id = MAPSET;
        break;
      }
    case DATABASE:
    case CRS:
    case REGION:
    case MAPSET:
      id += 1;
      break;
    case FINISH:
    default:
      id = -1;
  }
  return id;
}

void QgsGrassNewMapset::checkLocation()
{
  setError( mLocationErrorLabel, "" );
  button( QWizard::NextButton )->setEnabled( true );

  if ( mCreateLocationRadioButton->isChecked() )
  {
    // TODO?: Check spaces in the name

    QString location = mLocationLineEdit->text().trimmed();

    if ( location.length() ==  0 )
    {
      button( QWizard::NextButton )->setEnabled( false );
      setError( mLocationErrorLabel, tr( "Enter location name!" ) );
    }
    else
    {
      if ( QFile::exists( mDatabaseLineEdit->text() + "/" + location ) )
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
  QgsDebugMsg( "entered." );
}

void QgsGrassNewMapset::newLocationChanged()
{
  QgsDebugMsg( "entered." );
  checkLocation();
}

/************************** CRS ******************************/
void QgsGrassNewMapset::setProjectionPage()
{
  QgsDebugMsg( "entered." );
  setGrassProjection();
}

void QgsGrassNewMapset::sridSelected( QString theSRID )
{
  Q_UNUSED( theSRID );
  QgsDebugMsg( "entered." );
  projectionSelected();
}

void QgsGrassNewMapset::projectionSelected()
{
  QgsDebugMsg( "entered." );
  setGrassProjection();
}

void QgsGrassNewMapset::projRadioSwitched()
{
  QgsDebugMsg( "entered." );
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
  QgsDebugMsg( "entered." );
  setError( mProjErrorLabel, "" );

  QString proj4 = mProjectionSelector->selectedProj4String();

  // Not defined
  if ( mNoProjRadioButton->isChecked() )
  {
    mCellHead.proj = PROJECTION_XY;
    mCellHead.zone = 0;
    mProjInfo = 0;
    mProjUnits = 0;

    button( QWizard::NextButton )->setEnabled( true );
    return;
  }

  // Define projection
  if ( !proj4.isEmpty() )
  {
    QgsDebugMsg( QString( "proj4 = %1" ).arg( proj4.toLocal8Bit().constData() ) );

    OGRSpatialReferenceH hCRS = NULL;
    hCRS = OSRNewSpatialReference( NULL );
    int errcode;
    const char *oldlocale = setlocale( LC_NUMERIC, NULL );
    setlocale( LC_NUMERIC, "C" );
    errcode = OSRImportFromProj4( hCRS, proj4.toUtf8() );
    setlocale( LC_NUMERIC, oldlocale );
    if ( errcode != OGRERR_NONE )
    {
      QgsDebugMsg( QString( "OGR can't parse PROJ.4-style parameter string:\n%1\nOGR Error code was %2" ).arg( proj4 ).arg( errcode ) );

      mCellHead.proj = PROJECTION_XY;
      mCellHead.zone = 0;
      mProjInfo = 0;
      mProjUnits = 0;
    }
    else
    {
      char *wkt = NULL;

      QgsDebugMsg( QString( "OSRIsGeographic = %1" ).arg( OSRIsGeographic( hCRS ) ) );
      QgsDebugMsg( QString( "OSRIsProjected = %1" ).arg( OSRIsProjected( hCRS ) ) );

      if (( errcode = OSRExportToWkt( hCRS, &wkt ) ) != OGRERR_NONE )
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
      QgsDebugMsg( QString( "mProjInfo = %1" ).arg( QString::number(( qulonglong )mProjInfo, 16 ).toLocal8Bit().constData() ) );

      OGRFree( wkt );
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
    mProjInfo = 0;
    mProjUnits = 0;
  }
  button( QWizard::NextButton )->setEnabled( mProjInfo && mProjUnits );
}

/**************************** REGION ********************************/
void QgsGrassNewMapset::setRegionPage()
{
  QgsDebugMsg( "entered." );

  // Set defaults
  if ( !mRegionModified )
  {
    setGrassRegionDefaults();
  }

  // Create new projection
  QgsCoordinateReferenceSystem newSrs;
  if ( mProjRadioButton->isChecked() )
  {
    QgsDebugMsg( QString( "selectedCrsId() = %1" ).arg( mProjectionSelector->selectedCrsId() ) );

    if ( mProjectionSelector->selectedCrsId() > 0 )
    {
      newSrs.createFromSrsId( mProjectionSelector->selectedCrsId() );
      if ( ! newSrs.isValid() )
      {
        QMessageBox::warning( 0, tr( "Warning" ), tr( "Cannot create projection." ) );
      }
    }
  }

  // Reproject previous region if it was modified
  // and if previous and current projection is valid
  if ( mRegionModified && newSrs.isValid() && mSrs.isValid()
       && newSrs.srsid() != mSrs.srsid() )
  {
    QgsCoordinateTransform trans( mSrs, newSrs );

    double n = mNorthLineEdit->text().toDouble();
    double s = mSouthLineEdit->text().toDouble();
    double e = mEastLineEdit->text().toDouble();
    double w = mWestLineEdit->text().toDouble();

    std::vector<QgsPoint> points;

    // TODO: this is not perfect
    points.push_back( QgsPoint( w, s ) );
    points.push_back( QgsPoint( e, n ) );

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
      mNorthLineEdit->setText( QString::number( points[1].y() ) );
      mSouthLineEdit->setText( QString::number( points[0].y() ) );
      mEastLineEdit->setText( QString::number( points[1].x() ) );
      mWestLineEdit->setText( QString::number( points[0].x() ) );
    }
    else
    {
      QMessageBox::warning( 0, tr( "Warning" ), tr( "Cannot reproject previously set region, default region set." ) );

      setGrassRegionDefaults();
    }
  }

  // Set current region projection
  mSrs = newSrs;

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

    if ( ext.xMinimum() >= ext.xMaximum() || ext.yMinimum() >= ext.yMaximum() )
    {
      mCurrentRegionButton->setEnabled( false );
    }
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

  int srsid = QgsProject::instance()->readNumEntry(
                "SpatialRefSys", "/ProjectCRSID", 0 );

  QgsDebugMsg( QString( "current project srsid = %1" ).arg( srsid ) );

  QgsRectangle ext = mIface->mapCanvas()->extent();
  bool extSet = false;
  if ( ext.xMinimum() < ext.xMaximum() && ext.yMinimum() < ext.yMaximum() )
  {
    extSet = true;
  }

  if ( extSet &&
       ( mNoProjRadioButton->isChecked() ||
         ( mProjRadioButton->isChecked()
           && srsid == mProjectionSelector->selectedCrsId() )
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
    mNorthLineEdit->setText( "1000" );
    mSouthLineEdit->setText( "0" );
    mEastLineEdit->setText( "1000" );
    mWestLineEdit->setText( "0" );
  }
  else if ( mCellHead.proj == PROJECTION_LL )
  {
    mNorthLineEdit->setText( "90" );
    mSouthLineEdit->setText( "-90" );
    mEastLineEdit->setText( "180" );
    mWestLineEdit->setText( "-180" );
  }
  else
  {
    mNorthLineEdit->setText( "100000" );
    mSouthLineEdit->setText( "-100000" );
    mEastLineEdit->setText( "100000" );
    mWestLineEdit->setText( "-100000" );
  }
  mRegionModified = false;
}

void QgsGrassNewMapset::regionChanged()
{
  QgsDebugMsg( "entered." );

  mRegionModified = true;
  checkRegion();
  drawRegion();
}

void QgsGrassNewMapset::checkRegion()
{
  QgsDebugMsg( "entered." );

  bool err = false;

  setError( mRegionErrorLabel, "" );
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

  mCellHead.rows   = ( int )(( n - s ) / res );
  mCellHead.rows3  = ( int )(( n - s ) / res3 );
  mCellHead.cols   = ( int )(( e - w ) / res );
  mCellHead.cols3  = ( int )(( e - w ) / res3 );
  mCellHead.depths = 1;

  mCellHead.ew_res  = res;
  mCellHead.ew_res3 = res3;
  mCellHead.ns_res  = res;
  mCellHead.ns_res3 = res3;
  mCellHead.tb_res  = 1.;
  mCellHead.zone = 0;

  button( QWizard::NextButton )->setEnabled( true );
}

void QgsGrassNewMapset::loadRegions()
{
  QgsDebugMsg( "entered." );

  QString path = QgsApplication::pkgDataPath() + "/grass/locations.gml";
  QgsDebugMsg( QString( "load:%1" ).arg( path.toLocal8Bit().constData() ) );

  QFile file( path );

  if ( !file.exists() )
  {
    QMessageBox::warning( 0, tr( "Warning" ),
                          tr( "Regions file (%1) not found." ).arg( path ) );
    return;
  }
  if ( ! file.open( QIODevice::ReadOnly ) )
  {
    QMessageBox::warning( 0, tr( "Warning" ),
                          tr( "Cannot open locations file (%1)" ).arg( path ) );
    return;
  }

  QDomDocument doc( "gml:FeatureCollection" );
  QString err;
  int line, column;

  if ( !doc.setContent( &file,  &err, &line, &column ) )
  {
    QString errmsg = tr( "Cannot read locations file (%1):" ).arg( path )
                     + tr( "\n%1\nat line %2 column %3" ).arg( err ).arg( line ).arg( column );
    QgsDebugMsg( errmsg );
    QMessageBox::warning( 0, tr( "Warning" ), errmsg );
    file.close();
    return;
  }

  QDomElement docElem = doc.documentElement();
  QDomNodeList nodes = docElem.elementsByTagName( "gml:featureMember" );

  for ( int i = 0; i < nodes.count(); i++ )
  {
    QDomNode node = nodes.item( i );

    if ( node.isNull() )
    {
      continue;
    }

    QDomElement elem = node.toElement();
    QDomNodeList nameNodes = elem.elementsByTagName( "gml:name" );
    if ( nameNodes.count() == 0 )
      continue;
    if ( nameNodes.item( 0 ).isNull() )
      continue;

    QDomElement nameElem = nameNodes.item( 0 ).toElement();
    if ( nameElem.text().isNull() )
      continue;

    QDomNodeList envNodes = elem.elementsByTagName( "gml:Envelope" );
    if ( envNodes.count() == 0 )
      continue;
    if ( envNodes.item( 0 ).isNull() )
      continue;
    QDomElement envElem = envNodes.item( 0 ).toElement();

    QDomNodeList coorNodes = envElem.elementsByTagName( "gml:coordinates" );
    if ( coorNodes.count() == 0 )
      continue;
    if ( coorNodes.item( 0 ).isNull() )
      continue;
    QDomElement coorElem = coorNodes.item( 0 ).toElement();
    if ( coorElem.text().isNull() )
      continue;

    QStringList coor = coorElem.text().split( " ", QString::SkipEmptyParts );
    if ( coor.size() != 2 )
    {
      QgsDebugMsg( QString( "Cannot parse coordinates: %1" ).arg( coorElem.text() ) );
      continue;
    }

    QStringList ll = coor[0].split( ",", QString::SkipEmptyParts );
    QStringList ur = coor[1].split( ",", QString::SkipEmptyParts );
    if ( ll.size() != 2 || ur.size() != 2 )
    {
      QgsDebugMsg( QString( "Cannot parse coordinates: %1" ).arg( coorElem.text() ) );
      continue;
    }

    // Add region
    mRegionsComboBox->addItem( nameElem.text() );

    QgsPoint llp( ll[0].toDouble(), ll[1].toDouble() );
    mRegionsPoints.push_back( llp );
    QgsPoint urp( ur[0].toDouble(), ur[1].toDouble() );
    mRegionsPoints.push_back( urp );
  }

  file.close();
}

void QgsGrassNewMapset::setSelectedRegion()
{
  QgsDebugMsg( "entered." );

  // mRegionsPoints are in EPSG 4326 = LL WGS84
  int index = 2 * mRegionsComboBox->currentIndex();

  std::vector<QgsPoint> points;
  // corners ll lr ur ul
  points.push_back( QgsPoint( mRegionsPoints[index] ) );
  points.push_back( QgsPoint( mRegionsPoints[index+1].x(),
                              mRegionsPoints[index].y() ) );
  points.push_back( QgsPoint( mRegionsPoints[index+1] ) );
  points.push_back( QgsPoint( mRegionsPoints[index].x(),
                              mRegionsPoints[index+1].y() ) );

  // Convert to currently selected coordinate system


  // Warning: seems that crashes if source == dest
  if ( mProjectionSelector->selectedCrsId() != GEOCRS_ID )
  {
    // Warning: QgsCoordinateReferenceSystem::EpsgCrsId is broken (using epsg_id)
    //QgsCoordinateReferenceSystem source ( 4326, QgsCoordinateReferenceSystem::EpsgCrsId );
    QgsCoordinateReferenceSystem source( GEOCRS_ID, QgsCoordinateReferenceSystem::InternalCrsId );

    if ( !source.isValid() )
    {
      QMessageBox::warning( 0, tr( "Warning" ),
                            tr( "Cannot create QgsCoordinateReferenceSystem" ) );
      return;
    }

    QgsCoordinateReferenceSystem dest( mProjectionSelector->selectedCrsId(),
                                       QgsCoordinateReferenceSystem::InternalCrsId );

    if ( !dest.isValid() )
    {
      QMessageBox::warning( 0, tr( "Warning" ),
                            tr( "Cannot create QgsCoordinateReferenceSystem" ) );
      return;
    }

    QgsCoordinateTransform trans( source, dest );

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
      QMessageBox::warning( 0, tr( "Warning" ),
                            tr( "Cannot reproject selected region." ) );
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
  QgsDebugMsg( "entered." );

  QgsRectangle ext = mIface->mapCanvas()->extent();

  int srsid = QgsProject::instance()->readNumEntry(
                "SpatialRefSys", "/ProjectCRSID", 0 );

  QgsCoordinateReferenceSystem srs( srsid, QgsCoordinateReferenceSystem::InternalCrsId );
  QgsDebugMsg( QString( "current project srsid = %1" ).arg( srsid ) );
  QgsDebugMsg( QString( "srs.isValid() = %1" ).arg( srs.isValid() ) );

  std::vector<QgsPoint> points;

  // TODO: this is not perfect
  points.push_back( QgsPoint( ext.xMinimum(), ext.yMinimum() ) );
  points.push_back( QgsPoint( ext.xMaximum(), ext.yMaximum() ) );

  // TODO add a method, this code is copy-paste from setSelectedRegion
  if ( srs.isValid() && mSrs.isValid()
       && srs.srsid() != mSrs.srsid() )
  {
    QgsCoordinateTransform trans( srs, mSrs );

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
      QMessageBox::warning( 0, tr( "Warning" ), tr( "Cannot reproject region" ) );
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
  QgsDebugMsg( "entered." );

  QPixmap pm = mPixmap;
  mRegionMap->setPixmap( pm );
}

void QgsGrassNewMapset::drawRegion()
{
  QgsDebugMsg( "entered." );

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
    if (( 180 - w ) < ( e + 180 ) )
    {
      w -= 360;
    }
    else
    {
      e += 360;
    }
  }

  std::vector<QgsPoint> tpoints; // ll lr ur ul ll
  tpoints.push_back( QgsPoint( w, s ) );
  tpoints.push_back( QgsPoint( e, s ) );
  tpoints.push_back( QgsPoint( e, n ) );
  tpoints.push_back( QgsPoint( w, n ) );
  tpoints.push_back( QgsPoint( w, s ) );


  // Because of possible shift +/- 360 in LL we have to split
  // the lines at least in 3 parts
  std::vector<QgsPoint> points; //
  for ( int i = 0; i < 4; i++ )
  {
    for ( int j = 0; j < 3; j++ )
    {
      double x = tpoints[i].x();
      double y = tpoints[i].y();
      double dx = ( tpoints[i+1].x() - x ) / 3;
      double dy = ( tpoints[i+1].y() - y ) / 3;
      QgsDebugMsg( QString( "dx = %1 x = %2" ).arg( dx ).arg( x + j*dx ) );
      points.push_back( QgsPoint( x + j*dx, y + j*dy ) );

    }
  }
  points.push_back( QgsPoint( points[0] ) ); // close polygon

  // Warning: seems that crashes if source == dest
  if ( mProjectionSelector->selectedCrsId() != GEOCRS_ID )
  {
    QgsCoordinateReferenceSystem source( mProjectionSelector->selectedCrsId(),
                                         QgsCoordinateReferenceSystem::InternalCrsId );

    if ( !source.isValid() )
    {
      QMessageBox::warning( 0, tr( "Warning" ),
                            tr( "Cannot create QgsCoordinateReferenceSystem" ) );
      return;
    }

    QgsCoordinateReferenceSystem dest( GEOCRS_ID, QgsCoordinateReferenceSystem::InternalCrsId );

    if ( !dest.isValid() )
    {
      QMessageBox::warning( 0, tr( "Warning" ),
                            tr( "Cannot create QgsCoordinateReferenceSystem" ) );
      return;
    }

    QgsCoordinateTransform trans( source, dest );


    bool ok = true;
    for ( int i = 0; i < 13; i++ )
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

      try
      {
        points[i] = trans.transform( points[i] );
        QgsDebugMsg( QString( " --> %1,%2" ).arg( points[i].x() ).arg( points[i].y() ) );
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
      QgsDebugMsg( "Cannot reproject region." );
      return;
    }
  }

  for ( int shift = -360; shift <= 360; shift += 360 )
  {
    for ( int i = 0; i < 12; i++ )
    {
      double x1 = points[i].x();
      double x2 = points[i+1].x();

      if ( qAbs( x2 - x1 ) > 150 )
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
      p.drawLine( 180 + shift + ( int )x1, 90 - ( int )points[i].y(),
                  180 + shift + ( int )x2, 90 - ( int )points[i+1].y() );
    }
  }

  p.end();

  mRegionMap->setPixmap( pm );
}

/**************************** MAPSET ********************************/
void QgsGrassNewMapset::setMapsets()
{
  QgsDebugMsg( "entered." );
  mMapsetsListView->clear();

  if ( mCreateLocationRadioButton->isChecked() )
  {
    mMapsetsFrame->hide();
    return;
  }
  else
  {
    mMapsetsFrame->show();
  }

  // Get available mapsets
  QString locationPath = mDatabaseLineEdit->text() + "/" + mLocationComboBox->currentText();
  QDir d( locationPath );

  // Add all subdirs containing WIND
  for ( unsigned int i = 0; i < d.count(); i++ )
  {
    if ( d[i] == "." || d[i] == ".." )
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
  QgsDebugMsg( "entered." );

  button( QWizard::NextButton )->setEnabled( false );
  setError( mMapsetErrorLabel, "" );

  QString mapset = mMapsetLineEdit->text().trimmed();

  // TODO?: Check spaces in the name
  if ( mapset.length() == 0 )
  {
    setError( mMapsetErrorLabel, tr( "Enter mapset name." ) );
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
void QgsGrassNewMapset::setFinishPage()
{
  QgsDebugMsg( "entered." );

  mDatabaseLabel->setText( tr( "Database: " ) + mDatabaseLineEdit->text() );

  QString location;
  if ( mSelectLocationRadioButton->isChecked() )
  {
    location = mLocationComboBox->currentText();
  }
  else
  {
    location = mLocationLineEdit->text().trimmed();
  }
  mLocationLabel->setText( tr( "Location: " ) + location );

  mMapsetLabel->setText( tr( "Mapset: " ) + mMapsetLineEdit->text() );
}

void QgsGrassNewMapset::createMapset()
{
  QgsDebugMsg( "entered." );

  // TODO: handle all possible errors better, especially
  //       half created location/mapset

  QString location;

  if ( mCreateLocationRadioButton->isChecked() )
  {
    location = mLocationLineEdit->text().trimmed();

    // TODO: add QgsGrass::setLocation or G_make_location with
    //       database path
    if ( !QgsGrass::activeMode() ) // because it calls private QgsGrass::init()
    {
      QMessageBox::warning( this, tr( "Create mapset" ),
                            tr( "Cannot activate grass" ) );
      return;
    }

#if defined(WIN32)
    G__setenv(( char * ) "GISDBASE", QgsGrass::shortPath( mDatabaseLineEdit->text() ).toUtf8().data() );
#else
    G__setenv(( char * ) "GISDBASE", mDatabaseLineEdit->text().toUtf8().data() );
#endif

    int ret = 0;

    try
    {
      ret = G_make_location( location.toUtf8().data(), &mCellHead, mProjInfo, mProjUnits, stdout );
    }
    catch ( QgsGrass::Exception &e )
    {
      ret = -1;
      Q_UNUSED( e );
    }

    if ( ret != 0 )
    {
      QMessageBox::warning( this, tr( "Create location" ),
                            tr( "Cannot create new location: %1" ).arg( QgsGrass::errorMessage() ) );
      return;
    }

    // Location created -> reset widgets
    setLocations();
    mSelectLocationRadioButton->setChecked( true );
    mLocationComboBox->setItemText( mLocationComboBox->currentIndex(), location );
    mLocationLineEdit->setText( "" );
    locationRadioSwitched(); // calls also checkLocation()
  }
  else
  {
    location = mLocationComboBox->currentText();
  }

  // Create mapset
  QString mapset = mMapsetLineEdit->text().trimmed();

  if ( mapset != "PERMANENT" )
  {
    QString locationPath = mDatabaseLineEdit->text() + "/" + location;
    QDir d( locationPath );

    if ( !d.mkdir( mapset ) )
    {
      QMessageBox::warning( this, tr( "Create mapset" ),
                            tr( "Cannot create new mapset directory" ) );
      return;
    }

    // Copy WIND Better way to copy file in Qt?
    QStringList lines;
    QFile in( locationPath + "/PERMANENT/DEFAULT_WIND" );
    if ( !in.open( QIODevice::ReadOnly ) )
    {
      QMessageBox::warning( this, tr( "Create mapset" ), tr( "Cannot open DEFAULT_WIND" ) );
      return;
    }

    QFile out( locationPath + "/" + mapset + "/WIND" );
    if ( !out.open( QIODevice::WriteOnly ) )
    {
      QMessageBox::warning( this, tr( "Create mapset" ), tr( "Cannot open WIND" ) );
      return;
    }
    QTextStream stream( &out );

    //QTextStream stream( &file );
    QString line;
    char buf[100];
    while ( in.readLine( buf, 100 ) != -1 )
    {
      stream << buf;
    }

    in.close();
    out.close();
  }

  QString err = QgsGrass::openMapset(
                  mDatabaseLineEdit->text(), location, mapset );

  if ( err.length() > 0 )
  {
    QMessageBox::information( this, tr( "New mapset" ),
                              tr( "New mapset successfully created, but cannot be opened: %1" ).arg( err ) );
  }
  else
  {
    QMessageBox::information( this, tr( "New mapset" ),
                              tr( "New mapset successfully created and set as current working mapset." ) );

    mPlugin->mapsetChanged();
  }

  deleteLater();
}

void QgsGrassNewMapset::accept()
{
  QgsDebugMsg( "entered." );

  createMapset();
}

/********************************************************************/
void QgsGrassNewMapset::setError( QLabel *line, const QString &err )
{
  QgsDebugMsg( "entered." );

  if ( err.length() > 0 )
  {
    line->setText( err );
    line->show();
  }
  else
  {
    line->setText( "" );
    line->hide();
  }
}

// Warning: we have to catch key press otherwise QWizard goes always
// to next page if Key_Enter is pressed
void QgsGrassNewMapset::keyPressEvent( QKeyEvent * e )
{
  Q_UNUSED( e );
// QgsDebugMsg(QString("key = %1").arg(e->key()));
}

void QgsGrassNewMapset::pageSelected( int index )
{
  QgsDebugMsg( QString( "title = %1" ).arg( page( index ) ? page( index )->title() : "(null)" ) );

  switch ( index )
  {
    case LOCATION:
      if ( mPreviousPage == DATABASE )
      {
        setLocationPage();
      }
      break;

    case CRS:
      // Projection selector
      if ( !mProjectionSelector )
      {
        QGridLayout *projectionLayout = new QGridLayout( mProjectionFrame );

        mProjectionSelector = new QgsProjectionSelector( mProjectionFrame, "Projection", 0 );
        mProjectionSelector->setEnabled( false );
        projectionLayout->addWidget( mProjectionSelector, 0, 0 );

        mProjectionSelector->show();

        connect( mProjectionSelector, SIGNAL( sridSelected( QString ) ),
                 this, SLOT( sridSelected( QString ) ) );

        // Se current QGIS projection
        int srsid = QgsProject::instance()->readNumEntry(
                      "SpatialRefSys", "/ProjectCRSID", 0 );

        QgsCoordinateReferenceSystem srs( srsid, QgsCoordinateReferenceSystem::InternalCrsId );
        QgsDebugMsg( QString( "current project srsid = %1" ).arg( srsid ) );
        QgsDebugMsg( QString( "srs.isValid() = %1" ).arg( srs.isValid() ) );
        if ( srs.isValid() )
        {
          mProjectionSelector->setSelectedCrsId( srsid );
          mProjRadioButton->setChecked( true );
          projRadioSwitched();
        }
      }
      if ( mPreviousPage == LOCATION )
      {
        setProjectionPage();
      }
      break;

    case REGION:
      if ( !mRegionsInited )
      {
        loadRegions();
        mRegionsInited = true;
      }

      if ( mPreviousPage == CRS )
      {
        setRegionPage();
      }

      break;

    case MAPSET:
      if ( mPreviousPage == LOCATION || mPreviousPage == REGION )
      {
        setMapsets();
        mapsetChanged();
      }
      break;

    case FINISH:
      setFinishPage();
      break;
  }
  mPreviousPage = index;
}

bool QgsGrassNewMapset::isRunning( void )
{
  return mRunning;
}

void QgsGrassNewMapset::close( void )
{
  QgsDebugMsg( "entered." );

  hide();
  mRunning = false;
  deleteLater();
}

void QgsGrassNewMapset::closeEvent( QCloseEvent *e )
{
  QgsDebugMsg( "entered." );

  e->accept();
  close();
}
