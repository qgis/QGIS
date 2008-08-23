
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
#include <qdir.h>
#include <qevent.h>
#include <qfile.h>
#include <QFileDialog>
#include <qfileinfo.h>
#include <qsettings.h>
#include <q3listbox.h>
#include <qstringlist.h>
#include <qlabel.h>
#include <QComboBox>
#include <qspinbox.h>
#include <qmessagebox.h>
#include <qinputdialog.h>
#include <qcursor.h>
#include <q3listview.h>
#include <q3header.h>
#include <qradiobutton.h>
#include <qlayout.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qpen.h>
#include <qdom.h>
#include <qpushbutton.h>
#include <q3textbrowser.h>
#include <qapplication.h>
//Added by qt3to4:
#include <QKeyEvent>
#include <QTextStream>
#include <QGridLayout>
#include <QCloseEvent>
#include <Q3Wizard>

#include "qgis.h"
#include "qgisinterface.h"
#include "qgsapplication.h"
#include "qgsmapcanvas.h"
#include "qgsproject.h"
#include "qgsrect.h"
#include "qgscoordinatetransform.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsprojectionselector.h"

#include "qgsgrass.h"
#include "qgsgrassnewmapset.h"

// For bug in GPJ_osr_to_grass()
#include "grass/version.h"
#include "qgslogger.h"
// Prevents some compiler warnings from the version.h include
#ifndef GRASS_VERSION_RELEASE
// When using newer version of GRASS (cvs >= 26.4.2007),
// these variables are #defined instead of being static variables.
QString temp1( GRASS_VERSION_STRING );
QString temp2( GRASS_VERSION_MAJOR );
QString temp3( GRASS_VERSION_MINOR );
QString temp4( GRASS_VERSION_RELEASE );
#endif

#if defined(WIN32)
#include <windows.h>
static QString getShortPath( const QString &path )
{
  TCHAR buf[MAX_PATH];
  GetShortPathName( path.ascii(), buf, MAX_PATH );
  return buf;
}
#endif

bool QgsGrassNewMapset::mRunning = false;

QgsGrassNewMapset::QgsGrassNewMapset( QgisInterface *iface,
                                      QgsGrassPlugin *plugin, QWidget * parent,
                                      const char * name, Qt::WFlags f ) :
    Q3Wizard( parent, name, false, f ),
    QgsGrassNewMapsetBase( )
{
  QgsDebugMsg( "QgsGrassNewMapset()" );

  setupUi( this );

  mRunning = true;
  mIface = iface;
  mProjectionSelector = 0;
  mPreviousPage = -1;
  mRegionModified = false;

  QString mapPath = QgsApplication::pkgDataPath() + "/grass/world.png";
  QgsDebugMsg( QString( "mapPath = %1" ).arg( mapPath ) );

  //mPixmap = QPixmap( *(mRegionMap->pixmap()) );
  mPixmap.load( mapPath );
  QgsDebugMsg( QString( "mPixmap.isNull() = %1" ).arg( mPixmap.isNull() ) );

  mRegionsInited = false;
  mPlugin = plugin;

  setHelpEnabled( page( DATABASE ), false );
  setHelpEnabled( page( LOCATION ), false );
  setHelpEnabled( page( PROJECTION ), false );
  setHelpEnabled( page( REGION ), false );
  setHelpEnabled( page( MAPSET ), false );
  setHelpEnabled( page( FINISH ), false );

  setTitle( page( DATABASE ), tr( "GRASS database" ) );
  setTitle( page( LOCATION ), tr( "GRASS location" ) );
  setTitle( page( PROJECTION ), tr( "Projection" ) );
  setTitle( page( REGION ), tr( "Default GRASS Region" ) );
  setTitle( page( MAPSET ), tr( "Mapset" ) );
  setTitle( page( FINISH ), tr( "Create New Mapset" ) );

  setError( mDatabaseErrorLabel, "" );
  setError( mLocationErrorLabel, "" );
  setError( mProjErrorLabel, "" );
  setError( mRegionErrorLabel, "" );
  setError( mMapsetErrorLabel, "" );

  mDatabaseText->setPaletteBackgroundColor( paletteBackgroundColor() );
  mLocationText->setPaletteBackgroundColor( paletteBackgroundColor() );
  mRegionText->setPaletteBackgroundColor( paletteBackgroundColor() );
  mMapsetText->setPaletteBackgroundColor( paletteBackgroundColor() );

  // DATABASE
  QSettings settings;
  QString db = settings.readEntry( "/GRASS/lastGisdbase" );
  if ( !db.isNull() )
  {
    mDatabaseLineEdit->setText( db );
  }
  else
  {
    mDatabaseLineEdit->setText( QDir::currentDirPath() );
  }
  databaseChanged();

  // Create example tree structure
  mTreeListView->clear();
  mTreeListView->setSortColumn( -1 ); // No sorting
  mTreeListView->setColumnText( 0, tr( "Tree" ) );
  mTreeListView->addColumn( tr( "Comment" ) );
  Q3ListViewItem *dbi = new Q3ListViewItem( mTreeListView, "OurDatabase", tr( "Database" ) );
  dbi->setOpen( true );

  // First inserted is last in the view
  Q3ListViewItem *l = new Q3ListViewItem( dbi, "New Zealand", tr( "Location 2" ) );
  l->setOpen( true );
  Q3ListViewItem *m = new Q3ListViewItem( l, "Cimrman", tr( "User's mapset" ) );
  m->setOpen( true );
  m = new Q3ListViewItem( l, "PERMANENT", tr( "System mapset" ) );
  m->setOpen( true );

  l = new Q3ListViewItem( dbi, "Mexico", tr( "Location 1" ) );
  m->setOpen( true );
  m = new Q3ListViewItem( l, "Juan", tr( "User's mapset" ) );
  l->setOpen( true );
  m = new Q3ListViewItem( l, "Alejandra", tr( "User's mapset" ) );
  m->setOpen( true );
  m = new Q3ListViewItem( l, "PERMANENT", tr( "System mapset" ) );
  m->setOpen( true );

  // PROJECTION

  // MAPSET
  mMapsetsListView->clear();
  mMapsetsListView->setColumnText( 0, tr( "Mapset" ) );
  mMapsetsListView->addColumn( tr( "Owner" ) );

  // FINISH
  setFinishEnabled( page( FINISH ), true );

  connect( this, SIGNAL( selected( const QString & ) ),
           this, SLOT( pageSelected( const QString & ) ) );
}

QgsGrassNewMapset::~QgsGrassNewMapset()
{
  QgsDebugMsg( "QgsGrassNewMapset::~QgsGrassNewMapset()" );

  mRunning = false;
}
/*************************** DATABASE *******************************/
void QgsGrassNewMapset::browseDatabase()
{
  // TODO: unfortunately QFileDialog does not support 'new' directory
  QFileDialog *fd = new QFileDialog( this, NULL, mDatabaseLineEdit->text() );
  fd->setMode( QFileDialog::DirectoryOnly );

  if ( fd->exec() == QDialog::Accepted )
  {
    mDatabaseLineEdit->setText( fd->selectedFile() );
    databaseChanged();
  }
}

void QgsGrassNewMapset::databaseChanged()
{
  QgsDebugMsg( "QgsGrassNewMapset::databaseChanged()" );
  // TODO: reset next tabs
  //
  QSettings settings;
  settings.writeEntry( "/GRASS/lastGisdbase", mDatabaseLineEdit->text() );

  setNextEnabled( page( DATABASE ), false );
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
    if ( d[i] == "." || d[i] == ".." ) continue;

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
    setNextEnabled( page( DATABASE ), true );
  }
  else
  {
    setError( mDatabaseErrorLabel, tr( "No writable "
                                       "locations, the database not writable!" ) );
  }
}

/*************************** LOCATION *******************************/
void QgsGrassNewMapset::setLocationPage( )
{
  QgsDebugMsg( "QgsGrassNewMapset::setLocationPage" );

  setLocations();
}

void QgsGrassNewMapset::setLocations( )
{
  QgsDebugMsg( "QgsGrassNewMapset::setLocations" );

  mLocationComboBox->clear();

  QSettings settings;
  QString lastLocation = settings.readEntry( "/GRASS/lastLocation" );

  // Get available locations with write permissions
  QDir d( mDatabaseLineEdit->text() );

  // Add all subdirs containing PERMANENT/DEFAULT_WIND
  int idx = 0;
  int sel = -1;
  for ( unsigned int i = 0; i < d.count(); i++ )
  {
    if ( d[i] == "." || d[i] == ".." ) continue;

    QString windName = mDatabaseLineEdit->text() + "/" + d[i] + "/PERMANENT/DEFAULT_WIND";
    QString locationName = mDatabaseLineEdit->text() + "/" + d[i];
    QFileInfo locationInfo( locationName );

    if ( QFile::exists( windName ) && locationInfo.isWritable() )
    {
      mLocationComboBox->insertItem( QString( d[i] ), -1 );
      if ( QString( d[i] ) == lastLocation )
      {
        sel = idx;
      }
      idx++;
    }
  }
  if ( sel >= 0 )
  {
    mLocationComboBox->setCurrentItem( sel );
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
    setAppropriate( page( PROJECTION ), false );
    setAppropriate( page( REGION ), false );
  }
  else
  {
    mLocationComboBox->setEnabled( false );
    mLocationLineEdit->setEnabled( true );
    setAppropriate( page( PROJECTION ), true );
    setAppropriate( page( REGION ), true );
  }
  checkLocation();
}

void QgsGrassNewMapset::checkLocation()
{
  setError( mLocationErrorLabel, "" );
  setNextEnabled( page( LOCATION ), true );

  if ( mCreateLocationRadioButton->isChecked() )
  {
    // TODO?: Check spaces in the name

    QString location = mLocationLineEdit->text().trimmed();

    if ( location.length() ==  0 )
    {
      setNextEnabled( page( LOCATION ), false );
      setError( mLocationErrorLabel, tr( "Enter location name!" ) );
    }
    else
    {
      QDir d( mDatabaseLineEdit->text() );

      for ( unsigned int i = 0; i < d.count(); i++ )
      {
        if ( d[i] == "." || d[i] == ".." ) continue;

        if ( d[i] == location )
        {
          setNextEnabled( page( LOCATION ), false );
          setError( mLocationErrorLabel, tr( "The location exists!" ) );
          break;
        }
      }
    }
  }
}

void QgsGrassNewMapset::existingLocationChanged( const QString &text )
{
  QgsDebugMsg( "QgsGrassNewMapset::existingLocationChanged()" );
}

void QgsGrassNewMapset::newLocationChanged()
{
  QgsDebugMsg( "QgsGrassNewMapset::newLocationChanged()" );
  checkLocation();
}

/************************** PROJECTION ******************************/
void QgsGrassNewMapset::setProjectionPage()
{
  QgsDebugMsg( "QgsGrassNewMapset::setProjectionPage()" );
  setGrassProjection();
}

void QgsGrassNewMapset::sridSelected( QString theSRID )
{
  QgsDebugMsg( "QgsGrassNewMapset::sridSelected()" );
  projectionSelected();
}

void QgsGrassNewMapset::projectionSelected()
{
  QgsDebugMsg( "QgsGrassNewMapset::projectionSelected()" );
  setGrassProjection();
}

void QgsGrassNewMapset::projRadioSwitched()
{
  QgsDebugMsg( "QgsGrassNewMapset::projRadioSwitched" );
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
  QgsDebugMsg( "QgsGrassNewMapset::setGrassProjection()" );
  setError( mProjErrorLabel, "" );

  QString proj4 = mProjectionSelector->getSelectedProj4String();

  // Not defined
  if ( mNoProjRadioButton->isChecked() )
  {
    mCellHead.proj = PROJECTION_XY;
    mCellHead.zone = 0;
    mProjInfo = 0;
    mProjUnits = 0;

    setNextEnabled( page( PROJECTION ), true );
    return;
  }

  // Define projection
  if ( !proj4.isNull() )
  {
    QgsDebugMsg( QString( "proj4 = %1" ).arg( proj4.local8Bit().data() ) );

    OGRSpatialReferenceH hCRS = NULL;
    hCRS = OSRNewSpatialReference( NULL );
    int errcode;
    const char *oldlocale = setlocale( LC_NUMERIC, NULL );
    setlocale( LC_NUMERIC, "C" );
    errcode = OSRImportFromProj4( hCRS, proj4.ascii() );
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
#ifdef QGISDEBUG
      QgsDebugMsg( QString( "OSRIsGeographic = %1" ).arg( OSRIsGeographic( hCRS ) ) );
      QgsDebugMsg( QString( "OSRIsProjected = %1" ).arg( OSRIsProjected( hCRS ) ) );

      char *wkt = NULL;
      if (( errcode = OSRExportToWkt( hCRS, &wkt ) ) != OGRERR_NONE )
      {
        QgsDebugMsg( QString( "OGR can't get WKT-style parameter string\nOGR Error code was %1" ).arg( errcode ) );
      }
      else
      {
        QgsDebugMsg( QString( "wkt = %1" ).arg( wkt ) );
      }
#endif

      int ret;
      // Note: GPJ_osr_to_grass() defaults in PROJECTION_XY if projection
      //       cannot be set

      // There was a bug in GRASS, it is present in 6.0.x line
#if GRASS_VERSION_MAJOR == 6 && GRASS_VERSION_MINOR >= 1
      ret = GPJ_osr_to_grass( &mCellHead, &mProjInfo,
                              &mProjUnits, hCRS, 0 );
#else
      // Buggy version:
      ret = GPJ_osr_to_grass( &mCellHead, &mProjInfo,
                              &mProjUnits, ( void ** )hCRS, 0 );
#endif

      // Note: It seems that GPJ_osr_to_grass()returns always 1,
      //   -> test if mProjInfo was set

      QgsDebugMsg( QString( "ret = %1" ).arg( ret ) );
      QgsDebugMsg( QString( "mProjInfo = %1" ).arg( QString::number(( qulonglong )mProjInfo, 16 ).toLocal8Bit().constData() ) );
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
  if ( mProjInfo && mProjUnits )
  {
    setNextEnabled( page( PROJECTION ), true );
  }
  else
  {
    setNextEnabled( page( PROJECTION ), false );
  }
}

/**************************** REGION ********************************/
void QgsGrassNewMapset::setRegionPage()
{
  QgsDebugMsg( "QgsGrassNewMapset::setRegionPage()" );

  // Set defaults
  if ( !mRegionModified )
  {
    setGrassRegionDefaults();
  }

  // Create new projection
  QgsCoordinateReferenceSystem newSrs;
  if ( mProjRadioButton->isChecked() )
  {
    QgsDebugMsg( QString( "getSelectedCRSID() = %1" ).arg( mProjectionSelector->getSelectedCRSID() ) );

    if ( mProjectionSelector->getSelectedCRSID() > 0 )
    {
      newSrs.createFromSrsId( mProjectionSelector->getSelectedCRSID() );
      if ( ! newSrs.isValid() )
      {
        QMessageBox::warning( 0, tr( "Warning" ),
                              tr( "Cannot create projection." ) );
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
      QMessageBox::warning( 0, tr( "Warning" ), tr( "Cannot reproject "
                            "previously set region, default region set." ) );

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

    QgsRect ext = mIface->getMapCanvas()->extent();

    if ( ext.xMin() >= ext.xMax() || ext.yMin() >= ext.yMax() )
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
  QgsDebugMsg( QString( "QgsGrassNewMapset::setGrassRegionDefaults() mCellHead.proj = %1" ).arg( mCellHead.proj ) );

  int srsid = QgsProject::instance()->readNumEntry(
                "SpatialRefSys", "/ProjectCRSID", 0 );

  QgsDebugMsg( QString( "current project srsid = %1" ).arg( srsid ) );

  QgsRect ext = mIface->getMapCanvas()->extent();
  bool extSet = false;
  if ( ext.xMin() < ext.xMax() && ext.yMin() < ext.yMax() )
  {
    extSet = true;
  }

  if ( extSet &&
       ( mNoProjRadioButton->isChecked() ||
         ( mProjRadioButton->isChecked()
           && srsid == mProjectionSelector->getSelectedCRSID() )
       )
     )
  {
    mNorthLineEdit->setText( QString::number( ext.yMax() ) );
    mSouthLineEdit->setText( QString::number( ext.yMin() ) );
    mEastLineEdit->setText( QString::number( ext.xMax() ) );
    mWestLineEdit->setText( QString::number( ext.xMin() ) );
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
  QgsDebugMsg( "QgsGrassNewMapset::regionChanged()" );

  mRegionModified = true;
  checkRegion();
  drawRegion();
}

void QgsGrassNewMapset::checkRegion()
{
  QgsDebugMsg( "QgsGrassNewMapset::checkRegion()" );

  bool err = false;

  setError( mRegionErrorLabel, "" );
  setNextEnabled( page( REGION ), false );

  if ( mNorthLineEdit->text().stripWhiteSpace().length() == 0
       || mSouthLineEdit->text().stripWhiteSpace().length() == 0
       || mEastLineEdit->text().stripWhiteSpace().length() == 0
       || mWestLineEdit->text().stripWhiteSpace().length() == 0 )
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

  if ( err ) return;

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

  setNextEnabled( page( REGION ), true );
}

void QgsGrassNewMapset::loadRegions()
{
  QgsDebugMsg( "QgsGrassNewMapset::loadRegions()" );

  QString path = QgsApplication::pkgDataPath() + "/grass/locations.gml";
  QgsDebugMsg( QString( "load:%1" ).arg( path.local8Bit().data() ) );

  QFile file( path );

  if ( !file.exists() )
  {
    QMessageBox::warning( 0, tr( "Warning" ),
                          tr( "Regions file (" ) + path + tr( ") not found." ) );
    return;
  }
  if ( ! file.open( QIODevice::ReadOnly ) )
  {
    QMessageBox::warning( 0, tr( "Warning" ),
                          tr( "Cannot open locations file (" ) + path + tr( ")" ) );
    return;
  }

  QDomDocument doc( "gml:FeatureCollection" );
  QString err;
  int line, column;

  if ( !doc.setContent( &file,  &err, &line, &column ) )
  {
    QString errmsg = tr( "Cannot read locations file (" ) + path + tr( "):\n" )
                     + err + tr( "\nat line " ) + QString::number( line )
                     + tr( " column " ) + QString::number( column );
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
    if ( nameNodes.count() == 0 ) continue;
    if ( nameNodes.item( 0 ).isNull() ) continue;

    QDomElement nameElem = nameNodes.item( 0 ).toElement();
    if ( nameElem.text().isNull() ) continue;

    QDomNodeList envNodes = elem.elementsByTagName( "gml:Envelope" );
    if ( envNodes.count() == 0 ) continue;
    if ( envNodes.item( 0 ).isNull() ) continue;
    QDomElement envElem = envNodes.item( 0 ).toElement();

    QDomNodeList coorNodes = envElem.elementsByTagName( "gml:coordinates" );
    if ( coorNodes.count() == 0 ) continue;
    if ( coorNodes.item( 0 ).isNull() ) continue;
    QDomElement coorElem = coorNodes.item( 0 ).toElement();
    if ( coorElem.text().isNull() ) continue;

    QStringList coor = QStringList::split( " ", coorElem.text() );
    if ( coor.size() != 2 )
    {
      QgsDebugMsg( QString( "Cannot parse coordinates: %1" ).arg( coorElem.text() ) );
      continue;
    }

    QStringList ll = QStringList::split( ",", coor[0] );
    QStringList ur = QStringList::split( ",", coor[1] );
    if ( ll.size() != 2 || ur.size() != 2 )
    {
      QgsDebugMsg( QString( "Cannot parse coordinates: %1" ).arg( coorElem.text() ) );
      continue;
    }

    // Add region
    mRegionsComboBox->insertItem( nameElem.text() );

    QgsPoint llp( ll[0].toDouble(), ll[1].toDouble() );
    mRegionsPoints.push_back( llp );
    QgsPoint urp( ur[0].toDouble(), ur[1].toDouble() );
    mRegionsPoints.push_back( urp );
  }

  file.close();
}

void QgsGrassNewMapset::setSelectedRegion()
{
  QgsDebugMsg( "QgsGrassNewMapset::setSelectedRegion()" );

  // mRegionsPoints are in EPSG 4326 = LL WGS84
  int index = 2 * mRegionsComboBox->currentItem();

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
  if ( mProjectionSelector->getSelectedCRSID() != 2585 )
  {
    // Warning: QgsCoordinateReferenceSystem::EPSG is broken (using epsg_id)
    //QgsCoordinateReferenceSystem source ( 4326, QgsCoordinateReferenceSystem::EPSG );
    QgsCoordinateReferenceSystem source( 2585, QgsCoordinateReferenceSystem::QGIS_CRSID );

    if ( !source.isValid() )
    {
      QMessageBox::warning( 0, tr( "Warning" ),
                            tr( "Cannot create QgsCoordinateReferenceSystem" ) );
      return;
    }

    QgsCoordinateReferenceSystem dest( mProjectionSelector->getSelectedCRSID(),
                                       QgsCoordinateReferenceSystem::QGIS_CRSID );

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

    if ( n > 90 ) n = 90;
    if ( s < -90 ) s = -90;
    /*
    if ( e > 180 ) e = 180;
    if ( w < -180 ) w = 180;
    */
  }
  else
  {
    for ( int i = 0; i < 4; i++ )
    {
      if ( i == 0 || points[i].y() > n ) n = points[i].y();
      if ( i == 0 || points[i].y() < s ) s = points[i].y();
      if ( i == 0 || points[i].x() > e ) e = points[i].x();
      if ( i == 0 || points[i].x() < w ) w = points[i].x();
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
  QgsDebugMsg( "QgsGrassNewMapset::setCurrentRegion()" );

  QgsRect ext = mIface->getMapCanvas()->extent();

  int srsid = QgsProject::instance()->readNumEntry(
                "SpatialRefSys", "/ProjectCRSID", 0 );

  QgsCoordinateReferenceSystem srs( srsid, QgsCoordinateReferenceSystem::QGIS_CRSID );
  QgsDebugMsg( QString( "current project srsid = %1" ).arg( srsid ) );
  QgsDebugMsg( QString( "srs.isValid() = %1" ).arg( srs.isValid() ) );

  std::vector<QgsPoint> points;

  // TODO: this is not perfect
  points.push_back( QgsPoint( ext.xMin(), ext.yMin() ) );
  points.push_back( QgsPoint( ext.xMax(), ext.yMax() ) );

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
  QgsDebugMsg( "QgsGrassNewMapset::clearRegion()" );

  QPixmap pm = mPixmap;
  mRegionMap->setPixmap( pm );
}

void QgsGrassNewMapset::drawRegion()
{
  QgsDebugMsg( "QgsGrassNewMapset::drawRegion()" );

  QPixmap pm = mPixmap;
  mRegionMap->setPixmap( pm );

  if ( mCellHead.proj == PROJECTION_XY ) return;

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
  if ( mProjectionSelector->getSelectedCRSID() != 2585 )
  {
    QgsCoordinateReferenceSystem source( mProjectionSelector->getSelectedCRSID(),
                                         QgsCoordinateReferenceSystem::QGIS_CRSID );

    if ( !source.isValid() )
    {
      QMessageBox::warning( 0, tr( "Warning" ),
                            tr( "Cannot create QgsCoordinateReferenceSystem" ) );
      return;
    }

    QgsCoordinateReferenceSystem dest( 2585, QgsCoordinateReferenceSystem::QGIS_CRSID );

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
        if ( points[i].y() >= 89.9 )  points[i].setY( 89.9 );
        if ( points[i].y() <= -89.9 )  points[i].setY( -89.9 );
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

      if ( fabs( x2 - x1 ) > 150 )
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
  QgsDebugMsg( "QgsGrassNewMapset::setMapsets" );
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
  Q3ListViewItem *lvi;
  for ( unsigned int i = 0; i < d.count(); i++ )
  {
    if ( d[i] == "." || d[i] == ".." ) continue;

    QString mapsetPath = locationPath + "/" + d[i];
    QString windPath = mapsetPath + "/WIND";
    QFileInfo mapsetInfo( mapsetPath );

    if ( QFile::exists( windPath ) )
    {
      lvi = new Q3ListViewItem( mMapsetsListView, d[i], mapsetInfo.owner() );
    }
  }
}

void QgsGrassNewMapset::mapsetChanged()
{
  QgsDebugMsg( "QgsGrassNewMapset::mapsetChanged()" );

  setNextEnabled( page( MAPSET ), false );
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
    bool exists = false;
    QString locationPath = mDatabaseLineEdit->text() + "/" + mLocationComboBox->currentText();
    QDir d( locationPath );

    for ( unsigned int i = 0; i < d.count(); i++ )
    {
      if ( d[i] == "." || d[i] == ".." ) continue;

      if ( d[i] == mapset )
      {
        setError( mMapsetErrorLabel, tr( "The mapset already exists" ) );
        exists = true;
        break;
      }
    }

    if ( !exists )
    {
      setNextEnabled( page( MAPSET ), true );
    }
  }
  else
  {
    setNextEnabled( page( MAPSET ), true );
  }
}

/**************************** FINISH ********************************/
void QgsGrassNewMapset::setFinishPage()
{
  QgsDebugMsg( "QgsGrassNewMapset::setFinish()" );

  mDatabaseLabel->setText( tr( "Database: " ) + mDatabaseLineEdit->text() );

  QString location;
  if ( mSelectLocationRadioButton->isChecked() )
  {
    location = mLocationComboBox->currentText();
  }
  else
  {
    location = mLocationLineEdit->text().stripWhiteSpace();
  }
  mLocationLabel->setText( tr( "Location: " ) + location );

  mMapsetLabel->setText( tr( "Mapset: " ) + mMapsetLineEdit->text() );

  setFinishEnabled( page( FINISH ), true );
}

void QgsGrassNewMapset::createMapset()
{
  QgsDebugMsg( "QgsGrassNewMapset::createMapset()" );

  // TODO: handle all possible errors better, especially
  //       half created location/mapset

  QString location;

  if ( mCreateLocationRadioButton->isChecked() )
  {
    location = mLocationLineEdit->text().stripWhiteSpace();

    // TODO: add QgsGrass::setLocation or G_make_location with
    //       database path
    QgsGrass::activeMode(); // because it calls private gsGrass::init()
#if defined(WIN32)
    G__setenv(( char * )"GISDBASE", ( char * ) getShortPath( mDatabaseLineEdit->text() ).ascii() );
#else
    G__setenv(( char * )"GISDBASE", ( char * ) mDatabaseLineEdit->text().ascii() );
#endif

    QgsGrass::resetError();
    int ret = G_make_location(( char * ) location.ascii(), &mCellHead,
                              mProjInfo, mProjUnits, stdout );

    if ( ret != 0 )
    {
      QMessageBox::warning( this, tr( "Create location" ),
                            tr( "Cannot create new location: " )
                            + QgsGrass::getErrorMessage() );

      return;
    }
    else
    {
      // Location created -> reset widgets
      setLocations();
      mSelectLocationRadioButton->setChecked( true );
      mLocationComboBox->setCurrentText( location );
      mLocationLineEdit->setText( "" );
      locationRadioSwitched(); // calls also checkLocation()
    }
  }
  else
  {
    location = mLocationComboBox->currentText();
  }

  // Create mapset
  QString mapset = mMapsetLineEdit->text().stripWhiteSpace();

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
                              tr( "New mapset successfully created, but cannot be "
                                  "opened: " ) + err );
  }
  else
  {
    QMessageBox::information( this, tr( "New mapset" ),
                              tr( "New mapset successfully created and set "
                                  "as current working mapset." ) );

    mPlugin->mapsetChanged();
  }

  delete this;
}

void QgsGrassNewMapset::accept()
{
  QgsDebugMsg( "QgsGrassNewMapset::accept()" );

  createMapset();
}

/********************************************************************/
void QgsGrassNewMapset::setError( QLabel *line, const QString &err )
{
  QgsDebugMsg( "QgsGrassNewMapset::setError(): " );

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
#ifdef QGISDEBUG
// QgsDebugMsg(QString("QgsGrassNewMapset::keyPressEvent() key = %1").arg(e->key()));
#endif
}

void QgsGrassNewMapset::pageSelected( const QString & title )
{
  QgsDebugMsg( QString( "QgsGrassNewMapset::pageSelected(): %1" ).arg( title.local8Bit().data() ) );

  int index = indexOf( currentPage() );

  QgsDebugMsg( QString( "index = %1" ).arg( index ) );

  switch ( index )
  {
    case LOCATION:
      if ( mPreviousPage == DATABASE )
      {
        setLocationPage();
      }
      break;

    case PROJECTION:
      // Projection selector
      if ( !mProjectionSelector )
      {
        QGridLayout *projectionLayout = new QGridLayout( mProjectionFrame, 1, 1 );

        mProjectionSelector = new QgsProjectionSelector( mProjectionFrame, "Projection", 0 );
        mProjectionSelector->setEnabled( false );
        projectionLayout->addWidget( mProjectionSelector, 0 , 0 );

        mProjectionSelector->show();

        connect( mProjectionSelector, SIGNAL( sridSelected( QString ) ),
                 this, SLOT( sridSelected( QString ) ) );

        // Se current QGIS projection
        int srsid = QgsProject::instance()->readNumEntry(
                      "SpatialRefSys", "/ProjectCRSID", 0 );

        QgsCoordinateReferenceSystem srs( srsid, QgsCoordinateReferenceSystem::QGIS_CRSID );
        QgsDebugMsg( QString( "current project srsid = %1" ).arg( srsid ) );
        QgsDebugMsg( QString( "srs.isValid() = %1" ).arg( srs.isValid() ) );
        if ( srs.isValid() )
        {
          mProjectionSelector->setSelectedCRSID( srsid );
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

      if ( mPreviousPage == PROJECTION )
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
  QgsDebugMsg( "QgsGrassNewMapset::close()" );

  hide();
  mRunning = false;
  delete this;
}

void QgsGrassNewMapset::closeEvent( QCloseEvent *e )
{
  QgsDebugMsg( "QgsGrassNewMapset::closeEvent()" );

  e->accept();
  close();
}
