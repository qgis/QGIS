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
#include <iostream>
#include <qdir.h>
#include <qevent.h>
#include <qfile.h>
#include <qfiledialog.h> 
#include <qfileinfo.h>
#include <qsettings.h>
#include <qlistbox.h>
#include <qstringlist.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qspinbox.h>
#include <qmessagebox.h>
#include <qinputdialog.h>
#include <qcursor.h>
#include <qlistview.h>
#include <qheader.h>
#include <qradiobutton.h>
#include <qlayout.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qpen.h>
#include <qdom.h>
#include <qpushbutton.h>
#include <qtextbrowser.h>
#include <qapplication.h>

#include "../../src/qgis.h"
#include "../../src/qgsmapcanvas.h"
#include "../../src/qgsproject.h"
#include "../../src/qgsrect.h"
#include "../../src/qgscoordinatetransform.h"
#include "../../src/qgsspatialrefsys.h"
#include "../../widgets/projectionselector/qgsprojectionselector.h"

#include "../../providers/grass/qgsgrass.h"
#include "qgsgrassnewmapset.h"

bool QgsGrassNewMapset::mRunning = false;

QgsGrassNewMapset::QgsGrassNewMapset ( QgisApp *qgisApp, QgisIface *iface, 
	QgsGrassPlugin *plugin,
    QWidget * parent, const char * name, WFlags f )
:QgsGrassNewMapsetBase ( parent, name, f )
{
#ifdef QGISDEBUG
  std::cerr << "QgsGrassNewMapset()" << std::endl;
#endif

    mRunning = true;
    mQgisApp = qgisApp;
    mIface = iface;
    mProjectionSelector = 0;
    mPreviousPage = -1;
    mRegionModified = false;
    mPixmap = QPixmap( *(mRegionMap->pixmap()) );
    mRegionsInited = false;
    mPlugin = plugin;
    
    setHelpEnabled ( page(DATABASE), false );
    setHelpEnabled ( page(LOCATION), false );
    setHelpEnabled ( page(PROJECTION), false );
    setHelpEnabled ( page(REGION), false );
    setHelpEnabled ( page(MAPSET), false );
    setHelpEnabled ( page(FINISH), false );

    setError ( mDatabaseErrorLabel, "" );
    setError ( mLocationErrorLabel, "" );
    setError ( mProjErrorLabel, "" );
    setError ( mRegionErrorLabel, "" );
    setError ( mMapsetErrorLabel, "" );

    mDatabaseText->setPaletteBackgroundColor ( paletteBackgroundColor() );
    mLocationText->setPaletteBackgroundColor ( paletteBackgroundColor() );
    mRegionText->setPaletteBackgroundColor ( paletteBackgroundColor() );
    mMapsetText->setPaletteBackgroundColor ( paletteBackgroundColor() );
    
    // DATABASE
    QSettings settings;
    QString db = settings.readEntry("/qgis/grass/lastGisdbase");
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
    mTreeListView->setSortColumn(-1); // No sorting
    mTreeListView->setColumnText( 0, "Tree" );
    mTreeListView->addColumn( "Comment" );
    QListViewItem *dbi = new QListViewItem( mTreeListView, "OurDatabase", "Database" );
    dbi->setOpen(true);
    
    // First inserted is last in the view
    QListViewItem *l = new QListViewItem( dbi, "New Zealand", "Location 1" );
    l->setOpen(true);
    QListViewItem *m = new QListViewItem( l, "Klima", "User's mapset");
    m->setOpen(true);
    m = new QListViewItem( l, "PERMANENT", "System mapset" );
    m->setOpen(true);
    
    l = new QListViewItem( dbi, "Mexico", "Location 2" );
    m->setOpen(true);
    m = new QListViewItem( l, "Jirovec", "User's mapset");
    l->setOpen(true);
    m = new QListViewItem( l, "Cimrman", "User's mapset");
    m->setOpen(true);
    m = new QListViewItem( l, "PERMANENT", "System mapset" );
    m->setOpen(true);
    
    // PROJECTION

    // MAPSET
    mMapsetsListView->clear();
    mMapsetsListView->setColumnText( 0, "Mapset" );
    mMapsetsListView->addColumn( "Owner" );

    // FINISH
    setFinishEnabled ( page(FINISH), true );
    
    connect( this, SIGNAL(selected(const QString &)), 
    	     this, SLOT(pageSelected(const QString &)));

}

QgsGrassNewMapset::~QgsGrassNewMapset()
{
#ifdef QGISDEBUG
   std::cerr << "QgsGrassNewMapset::~QgsGrassNewMapset()" << std::endl;
#endif

    mRunning = false;
}
/*************************** DATABASE *******************************/
void QgsGrassNewMapset::browseDatabase()
{
    // TODO: unfortunately QFileDialog does not support 'new' directory
    QFileDialog *fd = new QFileDialog ( mDatabaseLineEdit->text() );
    fd->setMode ( QFileDialog::DirectoryOnly ); 
    
    if ( fd->exec() == QDialog::Accepted )
    {
	mDatabaseLineEdit->setText( fd->selectedFile() );
	databaseChanged();
    }
}

void QgsGrassNewMapset::databaseChanged()
{
#ifdef QGISDEBUG
    std::cerr << "QgsGrassNewMapset::databaseChanged()" << std::endl;
#endif
    // TODO: reset next tabs
    //
    QSettings settings;
    settings.writeEntry("/qgis/grass/lastGisdbase", mDatabaseLineEdit->text() );

    setError ( mDatabaseErrorLabel, "" );
    QFileInfo databaseInfo ( mDatabaseLineEdit->text() );

    if ( databaseInfo.exists() )
    {	
        // Check if at least one writable location exists or 
        // database is writable
        bool locationExists = false;
	QDir d ( mDatabaseLineEdit->text() );
	for ( int i = 0; i < d.count(); i++ ) 
	{
	    if ( d[i] == "." || d[i] == ".." ) continue; 

	    QString windName = mDatabaseLineEdit->text() + "/" + d[i] + "/PERMANENT/DEFAULT_WIND";
	    QString locationName = mDatabaseLineEdit->text() + "/" + d[i];
	    QFileInfo locationInfo ( locationName );

	    if ( QFile::exists ( windName ) && locationInfo.isWritable () ) 
	    {
 		locationExists = true;
	        break;
	    }
	}

	if ( locationExists || databaseInfo.isWritable()  )
	{
            setNextEnabled ( page(DATABASE), true );
	}
	else
	{
            setNextEnabled ( page(DATABASE), false );
            setError ( mDatabaseErrorLabel, "No writable "
                "locations, the database not writable!");
	}
    }
    else
    {
        setNextEnabled ( page(DATABASE), false );
	if ( mDatabaseLineEdit->text().stripWhiteSpace().length() > 0 )
	{
            setError ( mDatabaseErrorLabel, "The directory doesn't exist!");
	}
    }
}

/*************************** LOCATION *******************************/
void QgsGrassNewMapset::setLocationPage ( )
{
#ifdef QGISDEBUG
    std::cerr << "QgsGrassNewMapset::setLocationPage" << std::endl;
#endif

    setLocations();
}

void QgsGrassNewMapset::setLocations ( )
{
#ifdef QGISDEBUG
    std::cerr << "QgsGrassNewMapset::setLocations" << std::endl;
#endif

    mLocationComboBox->clear();
    
    QSettings settings;
    QString lastLocation = settings.readEntry("/qgis/grass/lastLocation");

    // Get available locations with write permissions
    QDir d ( mDatabaseLineEdit->text() );

    // Add all subdirs containing PERMANENT/DEFAULT_WIND
    int idx = 0;
    int sel = -1;
    for ( int i = 0; i < d.count(); i++ ) 
    {
	if ( d[i] == "." || d[i] == ".." ) continue; 

	QString windName = mDatabaseLineEdit->text() + "/" + d[i] + "/PERMANENT/DEFAULT_WIND";
	QString locationName = mDatabaseLineEdit->text() + "/" + d[i];
	QFileInfo locationInfo ( locationName );

	if ( QFile::exists ( windName ) && locationInfo.isWritable () ) 
	{
            mLocationComboBox->insertItem ( QString ( d[i] ), -1 );
	    if ( QString ( d[i] ) == lastLocation ) {
		sel = idx;
	    }
	    idx++;
	}
    }
    if ( sel >= 0 ) {
        mLocationComboBox->setCurrentItem(sel);
    }

    if ( mLocationComboBox->count() == 0 )
    {
	mCreateLocationRadioButton->setChecked (true);
	mSelectLocationRadioButton->setEnabled(false);
    }
    else
    {
	mSelectLocationRadioButton->setEnabled(true);
    }

    locationRadioSwitched(); // calls also checkLocation()
}

void QgsGrassNewMapset::locationRadioSwitched()
{
    if ( mSelectLocationRadioButton->isChecked() )
    {
	mLocationComboBox->setEnabled(true);
	mLocationLineEdit->setEnabled(false);
	setAppropriate ( page(PROJECTION), false );
	setAppropriate ( page(REGION), false );
    }
    else
    {
	mLocationComboBox->setEnabled(false);
	mLocationLineEdit->setEnabled(true);
	setAppropriate ( page(PROJECTION), true );
	setAppropriate ( page(REGION), true );
    }
    checkLocation();
}

void QgsGrassNewMapset::checkLocation()
{
    setError ( mLocationErrorLabel, "");
    setNextEnabled ( page(LOCATION), true );

    if ( mCreateLocationRadioButton->isChecked() )
    {
	// TODO?: Check spaces in the name
	
	QString location = mLocationLineEdit->text().stripWhiteSpace();
	
	if ( location.length() > 0 )
	{
	    QDir d ( mDatabaseLineEdit->text() );

	    setNextEnabled ( page(LOCATION), true );

	    for ( int i = 0; i < d.count(); i++ ) 
	    {
		if ( d[i] == "." || d[i] == ".." ) continue; 
		
		if ( d[i] == location ) 
		{
		    setNextEnabled ( page(LOCATION), false );
                    setError ( mLocationErrorLabel, "The location exists!");
		    break;
		}
	    }
	}
	else
	{
	    setNextEnabled ( page(LOCATION), false );
	}
    }
}

void QgsGrassNewMapset::existingLocationChanged(const QString &text )
{
#ifdef QGISDEBUG
    std::cerr << "QgsGrassNewMapset::existingLocationChanged()" << std::endl;
#endif

}

void QgsGrassNewMapset::newLocationChanged()
{
#ifdef QGISDEBUG
    std::cerr << "QgsGrassNewMapset::newLocationChanged()" << std::endl;
#endif
    checkLocation();

}

/************************** PROJECTION ******************************/
void QgsGrassNewMapset::setProjectionPage()
{
#ifdef QGISDEBUG
    std::cerr << "QgsGrassNewMapset::setProjectionPage()" << std::endl;
#endif
}

void QgsGrassNewMapset::sridSelected(QString theSRID)
{
#ifdef QGISDEBUG
    std::cerr << "QgsGrassNewMapset::sridSelected()" << std::endl;
#endif
    projectionSelected();
}

void QgsGrassNewMapset::projectionSelected()
{
#ifdef QGISDEBUG
    std::cerr << "QgsGrassNewMapset::projectionSelected()" << std::endl;
#endif
    setGrassProjection();
}

void QgsGrassNewMapset::projRadioSwitched()
{
#ifdef QGISDEBUG
    std::cerr << "QgsGrassNewMapset::projRadioSwitched" << std::endl;
#endif
    if ( mNoProjRadioButton->isChecked() ) 
    {
	mProjectionSelector->setEnabled ( false );
    } else {
	mProjectionSelector->setEnabled ( true );
    }

    projectionSelected();
}

void QgsGrassNewMapset::setGrassProjection()
{
#ifdef QGISDEBUG
    std::cerr << "QgsGrassNewMapset::setGrassProjection()" << std::endl;
#endif
    setError ( mProjErrorLabel, "");

    QString proj4 = mProjectionSelector->getCurrentProj4String();

    // Not defined
    if ( mNoProjRadioButton->isChecked() )
    {
	mCellHead.proj = PROJECTION_XY;
	mCellHead.zone = 0;
	mProjInfo = 0;
	mProjUnits = 0;

        setNextEnabled ( page(PROJECTION), true );
	return;
    }

    // Define projection
    if ( !proj4.isNull() ) 
    {
#ifdef QGISDEBUG
	std::cerr << "proj4 = " << proj4 << std::endl;
#endif
		    
	OGRSpatialReferenceH hSRS = NULL;
	hSRS = OSRNewSpatialReference(NULL);
	int errcode;
	if ( (errcode = OSRImportFromProj4(hSRS, proj4.ascii())) != OGRERR_NONE) {
	    std::cerr << "OGR can't parse PROJ.4-style parameter string:\n" << proj4.ascii()
		<< "\nOGR Error code was " << errcode << std::endl;

	    mCellHead.proj = PROJECTION_XY;
	    mCellHead.zone = 0;
	    mProjInfo = 0;
	    mProjUnits = 0;
	}
	else
	{
#ifdef QGISDEBUG
	    std::cerr << "OSRIsGeographic = " << OSRIsGeographic( hSRS ) << std::endl;
	    std::cerr << "OSRIsProjected = " << OSRIsProjected( hSRS ) << std::endl;

	    char *wkt = NULL;	
	    if ((errcode = OSRExportToWkt(hSRS, &wkt)) != OGRERR_NONE) 
	    {
	        std::cerr << "OGR can't get WKT-style parameter string\n"
		     << "OGR Error code was " << errcode << std::endl;
	    }
	    else
	    {
	        std::cerr << "wkt = " << wkt << std::endl;
	    }
#endif
	    // Note: GPJ_osr_to_grass() defaults in PROJECTION_XY if projection
	    //       cannot be set
	    // TODO: necessity of (void **)hSRS is maybe bug in GRASS library?
	    int ret = GPJ_osr_to_grass ( &mCellHead, &mProjInfo, &mProjUnits, 
				    (void **)hSRS, 0);
	    
	    // Note: I seems that GPJ_osr_to_grass()returns always 1, 
	    // 	 -> test if mProjInfo was set
	    
#ifdef QGISDEBUG
	    std::cerr << "ret = " << ret << std::endl;
	    std::cerr << "mProjInfo = " << mProjInfo << std::endl;
#endif
	}

	if ( !mProjInfo || !mProjUnits )
	{
            setError ( mProjErrorLabel, "Selected projection is not supported by GRASS!");
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
        setNextEnabled ( page(PROJECTION), true );
    } 
    else
    {
        setNextEnabled ( page(PROJECTION), false );
    } 
}

/**************************** REGION ********************************/
void QgsGrassNewMapset::setRegionPage()
{
#ifdef QGISDEBUG
    std::cerr << "QgsGrassNewMapset::setRegionPage()" << std::endl;
#endif
  
    // Set defaults
    if ( !mRegionModified ) 
    {		
        setGrassRegionDefaults();
    }

    // Create new projection
    QgsSpatialRefSys newSrs;
    if ( mProjRadioButton->isChecked() ) 
    { 
#ifdef QGISDEBUG
        std::cerr << "getCurrentSRSID() = " << mProjectionSelector->getCurrentSRSID() << std::endl;
#endif

        if ( mProjectionSelector->getCurrentSRSID() > 0 )
        {
            newSrs.createFromSrsId ( mProjectionSelector->getCurrentSRSID() );
	    if (  ! newSrs.isValid() )
	    {
		QMessageBox::warning( 0, "Warning", 
		       "Cannot create projection." );
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
	points.push_back( QgsPoint(w,s) );
	points.push_back( QgsPoint(e,n) );
	
        bool ok = true;
	for ( int i = 0; i < 2; i++ ) 
	{ 
            try
            {
	        points[i] = trans.transform ( points[i] );
            }
            catch(QgsCsException &cse)
            {
		std::cerr << "Cannot transform point" << std::endl;
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
	    QMessageBox::warning( 0, "Warning", "Cannot reproject "
                  "previously set region, default region set." );

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
    } 
    else 
    {
        mRegionMap->show();
	mCurrentRegionButton->show();
        mRegionsComboBox->show();
        mRegionButton->show();

	QgsRect ext = mQgisApp->getMapCanvas()->extent();

	if ( ext.xMin() >= ext.xMax() || ext.yMin() >= ext.yMax() )
	{
	    mCurrentRegionButton->setEnabled(false);
	}
    }
    
    checkRegion();
    drawRegion();
}

void QgsGrassNewMapset::setGrassRegionDefaults()
{
#ifdef QGISDEBUG
    std::cerr << "QgsGrassNewMapset::setGrassRegionDefaults() mCellHead.proj = " << mCellHead.proj << std::endl;
#endif
    
    int srsid = QgsProject::instance()->readNumEntry(
                      "SpatialRefSys","/ProjectSRSID",0);

#ifdef QGISDEBUG
    std::cerr << "current project srsid = " << srsid << std::endl;
#endif
        
    QgsRect ext = mQgisApp->getMapCanvas()->extent();
    bool extSet = false;
    if ( ext.xMin() < ext.xMax() && ext.yMin() < ext.yMax() )
    {
        extSet = true;
    }

    if ( extSet && 
         ( mNoProjRadioButton->isChecked() ||
           ( mProjRadioButton->isChecked()  
             && srsid == mProjectionSelector->getCurrentSRSID() ) 
         )
       )
    {
	mNorthLineEdit->setText( QString::number(ext.yMax()) );
	mSouthLineEdit->setText( QString::number(ext.yMin()) );
	mEastLineEdit->setText( QString::number(ext.xMax()) );
	mWestLineEdit->setText( QString::number(ext.xMin()) );
    }
    else if ( mCellHead.proj == PROJECTION_XY ) 
    {
	mNorthLineEdit->setText("1000");
	mSouthLineEdit->setText("0");
	mEastLineEdit->setText("1000");
	mWestLineEdit->setText("0");
    }
    else if ( mCellHead.proj == PROJECTION_LL ) 
    {
	mNorthLineEdit->setText("90");
	mSouthLineEdit->setText("-90");
	mEastLineEdit->setText("180");
	mWestLineEdit->setText("-180");
    }
    else 
    {
	mNorthLineEdit->setText("100000");
	mSouthLineEdit->setText("-100000");
	mEastLineEdit->setText("100000");
	mWestLineEdit->setText("-100000");
    }
    mRegionModified = false; 
} 

void QgsGrassNewMapset::regionChanged()
{
#ifdef QGISDEBUG
    std::cerr << "QgsGrassNewMapset::regionChanged()" << std::endl;
#endif
    
    mRegionModified = true;
    checkRegion();
    drawRegion();
}

void QgsGrassNewMapset::checkRegion()
{
#ifdef QGISDEBUG
    std::cerr << "QgsGrassNewMapset::checkRegion()" << std::endl;
#endif

    bool err = false;

    setError ( mRegionErrorLabel, "");
    setNextEnabled ( page(REGION), false );

    if (   mNorthLineEdit->text().stripWhiteSpace().length() == 0 
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
        setError ( mRegionErrorLabel, "North must be greater than south");
	err = true;
    }	
    if ( e <= w && mCellHead.proj != PROJECTION_LL ) 
    {
        setError ( mRegionErrorLabel, "East must be greater than west");
	err = true;
    }	

    if ( err ) return;
	
    mCellHead.north = n;
    mCellHead.south = s;
    mCellHead.east = e;
    mCellHead.west = w;
    mCellHead.top = 1.;
    mCellHead.bottom = 0.;

    double res = (e - w)/1000; // reasonable resolution 
    double res3 = res/10.;

    mCellHead.rows   = (int) ( (n-s)/res );
    mCellHead.rows3  = (int) ( (n-s)/res3 );
    mCellHead.cols   = (int) ( (e-w)/res );
    mCellHead.cols3  = (int) ( (e-w)/res3 );
    mCellHead.depths = 1;
    
    mCellHead.ew_res  = res;
    mCellHead.ew_res3 = res3;
    mCellHead.ns_res  = res;
    mCellHead.ns_res3 = res3;
    mCellHead.tb_res  = 1.;
    mCellHead.zone = 0;

    setNextEnabled ( page(REGION), true );
}

void QgsGrassNewMapset::loadRegions()
{
#ifdef QGISDEBUG
    std::cerr << "QgsGrassNewMapset::loadRegions()" << std::endl;
#endif

#if defined(WIN32) || defined(Q_OS_MACX)
    QString appDir = qApp->applicationDirPath();
#else
    QString appDir = PREFIX;
#endif

    QString path = appDir + "/share/qgis/grass/locations.gml";
#ifdef QGISDEBUG
    std::cerr << "load:" << path << std::endl;
#endif

    QFile file ( path );

    if ( !file.exists() ) {
        QMessageBox::warning( 0, "Warning", 
                   "Regions file (" + path + ") not found." );
        return;
    }
    if ( ! file.open( IO_ReadOnly ) ) {
        QMessageBox::warning( 0, "Warning", 
                   "Cannot open locations file (" + path +")" );
        return;
    }

    QDomDocument doc ( "gml:FeatureCollection" );
    QString err;
    int line, column;

    if ( !doc.setContent( &file,  &err, &line, &column ) ) {
        QString errmsg = "Cannot read locations file (" + path + "):\n" 
                         + err + "\nat line " + QString::number(line) 
                         + " column " + QString::number(column);
        std::cerr << errmsg.local8Bit() << std::endl;
        QMessageBox::warning( 0, "Warning", errmsg );
        file.close();
        return;
    }

    QDomElement docElem = doc.documentElement();
    QDomNodeList nodes = docElem.elementsByTagName ( "gml:featureMember" );

    for ( int i = 0; i < nodes.count(); i++ ) 
    {
        QDomNode node = nodes.item(i);

        if ( node.isNull() ) {
	    continue;
        }

        QDomElement elem = node.toElement();
	QDomNodeList nameNodes = elem.elementsByTagName ( "gml:name" );
        if ( nameNodes.count() == 0 ) continue;
        if ( nameNodes.item(0).isNull() ) continue;

        QDomElement nameElem = nameNodes.item(0).toElement();
        if ( nameElem.text().isNull() ) continue;

	QDomNodeList envNodes = elem.elementsByTagName ( "gml:Envelope" );
        if ( envNodes.count() == 0 ) continue;
        if ( envNodes.item(0).isNull() ) continue;
        QDomElement envElem = envNodes.item(0).toElement();

	QDomNodeList coorNodes = envElem.elementsByTagName ( "gml:coordinates" );
        if ( coorNodes.count() == 0 ) continue;
        if ( coorNodes.item(0).isNull() ) continue;
        QDomElement coorElem = coorNodes.item(0).toElement();
        if ( coorElem.text().isNull() ) continue;
        QStringList coor = QStringList::split ( " ", coorElem.text() );
        QStringList ll = QStringList::split ( ",", coor[0] );
        QStringList ur = QStringList::split ( ",", coor[1] );

        // Add region
        mRegionsComboBox->insertItem ( nameElem.text() );

        QgsPoint llp ( ll[0].toDouble(), ll[1].toDouble() );
        mRegionsPoints.push_back( llp );
        QgsPoint urp ( ur[0].toDouble(), ur[1].toDouble() );
        mRegionsPoints.push_back( urp );
    }

    file.close();
}

void QgsGrassNewMapset::setSelectedRegion()
{
#ifdef QGISDEBUG
    std::cerr << "QgsGrassNewMapset::setSelectedRegion()" << std::endl;
#endif

    // mRegionsPoints are in EPSG 4326 = LL WGS84
    int index = 2 * mRegionsComboBox->currentItem();

    std::vector<QgsPoint> points; 
    // corners ll lr ur ul
    points.push_back( QgsPoint(mRegionsPoints[index]) );
    points.push_back( QgsPoint(mRegionsPoints[index+1].x(), 
			       mRegionsPoints[index].y()) );
    points.push_back( QgsPoint(mRegionsPoints[index+1]) );
    points.push_back( QgsPoint(mRegionsPoints[index].x(), 
			       mRegionsPoints[index+1].y()) );

    // Convert to currently selected coordinate system

    
    // Warning: seems that crashes if source == dest
    if ( mProjectionSelector->getCurrentSRSID() != 2585 )
    {
	// Warning: QgsSpatialRefSys::EPSG is broken (using epsg_id)
	//QgsSpatialRefSys source ( 4326, QgsSpatialRefSys::EPSG );
	QgsSpatialRefSys source ( 2585, QgsSpatialRefSys::QGIS_SRSID );

	if ( !source.isValid() ) 
	{
	    QMessageBox::warning( 0, "Warning", 
		       "Cannot create QgsSpatialRefSys" );
	    return;
	}

	QgsSpatialRefSys dest ( mProjectionSelector->getCurrentSRSID(), 
				QgsSpatialRefSys::QGIS_SRSID );

	if ( !dest.isValid() ) 
	{
	    QMessageBox::warning( 0, "Warning", 
		       "Cannot create QgsSpatialRefSys" );
	    return;
	}

	QgsCoordinateTransform trans( source, dest );
	
        bool ok = true;
	for ( int i = 0; i < 4; i++ ) 
	{ 
#ifdef QGISDEBUG
	    std::cerr << points[i].x() << "," << points[i].y() << "->" << std::endl;
#endif
            try
            {
	        points[i] = trans.transform ( points[i] );
#ifdef QGISDEBUG
	        std::cerr << points[i].x() << "," << points[i].y() << std::endl;
#endif
            }
            catch(QgsCsException &cse)
            {
		std::cerr << "Cannot transform point" << std::endl;
                ok = false;
                break;
            }
	}

        if ( !ok )
        {
	    QMessageBox::warning( 0, "Warning", 
		       "Cannot reproject selected region." );
            return;
        }
    }

    double n, s, e, w;

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

    mNorthLineEdit->setText( QString::number(n) );
    mSouthLineEdit->setText( QString::number(s) );
    mEastLineEdit->setText ( QString::number(e) );
    mWestLineEdit->setText ( QString::number(w) );

    mRegionModified = true;
    checkRegion();
    drawRegion();
}

void QgsGrassNewMapset::setCurrentRegion()
{
#ifdef QGISDEBUG
    std::cerr << "QgsGrassNewMapset::setCurrentRegion()" << std::endl;
#endif

    QgsRect ext = mQgisApp->getMapCanvas()->extent();

    int srsid = QgsProject::instance()->readNumEntry(
	   "SpatialRefSys","/ProjectSRSID",0);

    QgsSpatialRefSys srs( srsid, QgsSpatialRefSys::QGIS_SRSID );
#ifdef QGISDEBUG
    std::cerr << "current project srsid = " << srsid << std::endl;
    std::cerr << "srs.isValid() = " << srs.isValid() << std::endl;
#endif

    std::vector<QgsPoint> points; 

    // TODO: this is not perfect
    points.push_back( QgsPoint(ext.xMin(),ext.yMin()) );
    points.push_back( QgsPoint(ext.xMax(),ext.yMax()) );

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
	        points[i] = trans.transform ( points[i] );
            }
            catch(QgsCsException &cse)
            {
		std::cerr << "Cannot transform point" << std::endl;
                ok = false;
                break;
            }
	}

        if ( !ok )
        {
	    QMessageBox::warning( 0, "Warning", "Cannot reproject region" );
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
    std::cerr << "setCurrentRegion - End" << std::endl;
}

void QgsGrassNewMapset::clearRegion()
{
#ifdef QGISDEBUG
    std::cerr << "QgsGrassNewMapset::clearRegion()" << std::endl;
#endif

    QPixmap pm = mPixmap;
    mRegionMap->setPixmap( pm );
}

void QgsGrassNewMapset::drawRegion()
{
#ifdef QGISDEBUG
    std::cerr << "QgsGrassNewMapset::drawRegion()" << std::endl;
#endif

    QPixmap pm = mPixmap;
    mRegionMap->setPixmap( pm );

    if ( mCellHead.proj == PROJECTION_XY ) return; 

    QPainter p ( &pm );
    p.setPen( QPen(QColor(255,0,0),3) );

    double n = mNorthLineEdit->text().toDouble();
    double s = mSouthLineEdit->text().toDouble();
    double e = mEastLineEdit->text().toDouble();
    double w = mWestLineEdit->text().toDouble();

    // Shift if LL and W > E
    if ( mCellHead.proj == PROJECTION_LL && w > e ) 
    {
	if ( (180-w) < (e+180)  ) 
        {
            w -= 360;
        }
        else
        {
            e += 360;
        }
    }
	    
    std::vector<QgsPoint> tpoints; // ll lr ur ul ll
    tpoints.push_back( QgsPoint(w,s) );
    tpoints.push_back( QgsPoint(e,s) );
    tpoints.push_back( QgsPoint(e,n) );
    tpoints.push_back( QgsPoint(w,n) );
    tpoints.push_back( QgsPoint(w,s) );


    // Because of possible shift +/- 360 in LL we have to split 
    // the lines at least in 3 parts
    std::vector<QgsPoint> points; //
    for ( int i = 0; i < 4; i++ ) 
    { 
        for ( int j = 0; j < 3; j++ ) 
        {
	    double x = tpoints[i].x();
	    double y = tpoints[i].y();
	    double dx = (tpoints[i+1].x()-x)/3;
	    double dy = (tpoints[i+1].y()-y)/3;
#ifdef QGISDEBUG
    	    std::cerr << "dx = " << dx <<  " x = " << x+j*dx << std::endl;
#endif
    	    points.push_back( QgsPoint( x+j*dx, y+j*dy) );
        
        }
    }
    points.push_back( QgsPoint( points[0] ) ); // close polygon

    // Warning: seems that crashes if source == dest
    if ( mProjectionSelector->getCurrentSRSID() != 2585 )
    {
	QgsSpatialRefSys source ( mProjectionSelector->getCurrentSRSID(), 
				QgsSpatialRefSys::QGIS_SRSID );

	if ( !source.isValid() ) 
	{
	    QMessageBox::warning( 0, "Warning", 
		       "Cannot create QgsSpatialRefSys" );
	    return;
	}

	QgsSpatialRefSys dest ( 2585, QgsSpatialRefSys::QGIS_SRSID );

	if ( !dest.isValid() ) 
	{
	    QMessageBox::warning( 0, "Warning", 
		       "Cannot create QgsSpatialRefSys" );
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
		if ( points[i].y() >= 89.9 )  points[i].setY(89.9);
		if ( points[i].y() <= -89.9 )  points[i].setY(-89.9);
	    }

#ifdef QGISDEBUG
	    std::cerr << points[i].x() << "," << points[i].y() << " -> ";
#endif

            try
            {
                points[i] = trans.transform ( points[i] );
#ifdef QGISDEBUG
	        std::cerr << points[i].x() << "," << points[i].y() << std::endl;
#endif
            }
            catch(QgsCsException &cse)
            {
		std::cerr << "Cannot transform point" << std::endl;
                ok = false;
                break;
            }
	}

        if ( !ok )
        {
	    std::cerr << "Cannot reproject region." << std::endl;
            return;
        }
    }
            
    for ( int shift = -360; shift <= 360; shift+=360 )
    {
	for ( int i = 0; i < 12; i++ ) 
	{
	    double x1 = points[i].x(); 
	    double x2 = points[i+1].x(); 
    	    
	    if ( fabs(x2-x1) > 150 ) 
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
	    p.drawLine ( 180+shift+(int)x1, 90-(int)points[i].y(), 
			 180+shift+(int)x2, 90-(int)points[i+1].y() );
	}
    }
    std::cerr << "<<<<<<<<<<" << std::endl;

    p.end();
    std::cerr << "<<<<<<<<<<" << std::endl;

    mRegionMap->setPixmap( pm );
    std::cerr << "<<<<<<<<<<" << std::endl;
}

/**************************** MAPSET ********************************/
void QgsGrassNewMapset::setMapsets()
{
#ifdef QGISDEBUG
    std::cerr << "QgsGrassNewMapset::setMapsets" << std::endl;
#endif
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
    QDir d ( locationPath );

    // Add all subdirs containing WIND
    QListViewItem *lvi;
    for ( int i = 0; i < d.count(); i++ ) 
    {
	if ( d[i] == "." || d[i] == ".." ) continue; 

	QString mapsetPath = locationPath + "/" + d[i];
	QString windPath = mapsetPath + "/WIND";
	QFileInfo mapsetInfo ( mapsetPath );

	if ( QFile::exists ( windPath ) ) 
	{
            lvi = new QListViewItem( mMapsetsListView, d[i], mapsetInfo.owner() );
	}
    }
}

void QgsGrassNewMapset::mapsetChanged()
{
#ifdef QGISDEBUG
    std::cerr << "QgsGrassNewMapset::mapsetChanged()" << std::endl;
#endif
    
    setNextEnabled ( page(MAPSET), false );
    setError ( mMapsetErrorLabel, "");
    
    QString mapset = mMapsetLineEdit->text().stripWhiteSpace();
    
    // TODO?: Check spaces in the name
    if ( mapset.length() > 0 )
    {
	// Check if exists
	if ( mSelectLocationRadioButton->isChecked() ) 
	{
	    bool exists = false;
       	    QString locationPath = mDatabaseLineEdit->text() + "/" + mLocationComboBox->currentText();
            QDir d ( locationPath );

	    for ( int i = 0; i < d.count(); i++ ) 
	    {
		if ( d[i] == "." || d[i] == ".." ) continue; 

	        if ( d[i] == mapset ) 
		{
    		    setError ( mMapsetErrorLabel, "The mapset already exists");
		    exists = true;
		    break;
		}
	    }

	    if ( !exists )
	    {
                setNextEnabled ( page(MAPSET), true );
	    }
	}
	else
	{
            setNextEnabled ( page(MAPSET), true );
	}
    }
    else
    {
        setNextEnabled ( page(MAPSET), false );
    }
}

/**************************** FINISH ********************************/
void QgsGrassNewMapset::setFinishPage()
{
#ifdef QGISDEBUG
    std::cerr << "QgsGrassNewMapset::setFinish()" << std::endl;
#endif

    mDatabaseLabel->setText ( "Database: " + mDatabaseLineEdit->text() );

    QString location;
    if ( mSelectLocationRadioButton->isChecked() )
    {	
	location = mLocationComboBox->currentText();
    } 
    else
    {
	location = mLocationLineEdit->text().stripWhiteSpace();
    }
    mLocationLabel->setText ( "Location: " + location );

    mMapsetLabel->setText ( "Mapset: " + mMapsetLineEdit->text() );

    setFinishEnabled ( page(FINISH), true );
}

void QgsGrassNewMapset::createMapset()
{
#ifdef QGISDEBUG
    std::cerr << "QgsGrassNewMapset::createMapset()" << std::endl;
#endif

    // TODO: handle all possible errors better, especially 
    //       half created location/mapset

    QString location;
    
    if ( mCreateLocationRadioButton->isChecked() )
    {
	location = mLocationLineEdit->text().stripWhiteSpace();

	// TODO: add QgsGrass::setLocation or G_make_location with
	//       database path
	QgsGrass::activeMode(); // because it calls private gsGrass::init()
	G__setenv( "GISDBASE", (char *) mDatabaseLineEdit->text().ascii() );
	
	QgsGrass::resetError();
	int ret = G_make_location( (char *) location.ascii(), &mCellHead,
			    mProjInfo, mProjUnits, stdout );

	if ( ret != 0 )
	{
	    QMessageBox::warning (this, "Create location", 
		         "Cannot create new location: " 
			 + QgsGrass::getErrorMessage() );
	    
	    return;
	} 
	else
	{
	    // Location created -> reset widgets
	    setLocations();	
    	    mSelectLocationRadioButton->setChecked(true);
	    mLocationComboBox->setCurrentText ( location ); 
	    mLocationLineEdit->setText("");
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
	QDir d ( locationPath );

	if ( !d.mkdir(mapset) )
	{
	    QMessageBox::warning (this, "Create mapset", 
		         "Cannot create new mapset dircetory" );
	    
	    return;
	} 

	// Copy WIND Better way to copy file in Qt?
        QStringList lines;
        QFile in ( locationPath + "/PERMANENT/DEFAULT_WIND" );
        if ( !in.open( IO_ReadOnly ) ) 
	{
	    QMessageBox::warning (this, "Create mapset", "Cannot open DEFAULT_WIND" ); 
	    return;
	}
	    
        QFile out ( locationPath + "/" + mapset + "/WIND" );
        if ( !out.open( IO_WriteOnly ) ) 
	{
	    QMessageBox::warning (this, "Create mapset", "Cannot open WIND" ); 
	    return;
	}
	QTextStream stream ( &out );

        //QTextStream stream( &file );
        QString line;
        while ( in.readLine( line, 100 ) != -1 ) {
	    stream << line;
        }
	
        in.close();
        out.close();
    }

    QString err = QgsGrass::openMapset ( 
                mDatabaseLineEdit->text(), location, mapset );

    if ( err.length() > 0 ) 
    {
        QMessageBox::information ( this, "New mapset",
	    "New mapset successfully created, but cannot be "
            "opened: " + err );
    }
    else
    {
        QMessageBox::information ( this, "New mapset",
	    "New mapset successfully created and set "
            "as current working mapset." );
        
        mPlugin->mapsetChanged();
    }

    delete this;
}

void QgsGrassNewMapset::accept()
{
#ifdef QGISDEBUG
    std::cerr << "QgsGrassNewMapset::accept()" << std::endl;
#endif

    createMapset();
}

/********************************************************************/
void QgsGrassNewMapset::setError( QLabel *line, const QString &err )
{
#ifdef QGISDEBUG
    std::cerr << "QgsGrassNewMapset::setError(): " << std::endl;
#endif

    if ( err.length() > 0 )
    {
        line->setText(err);
        line->show();
    } 
    else
    {
        line->setText("");
        line->hide();
    }
}

// Warning: we have to catch key press otherwise QWizard goes always 
// to next page if Key_Enter is pressed
void QgsGrassNewMapset::keyPressEvent ( QKeyEvent * e )
{
#ifdef QGISDEBUG
    //std::cerr << "QgsGrassNewMapset::keyPressEvent() key = " << e->key() << std::endl;
#endif
}

void QgsGrassNewMapset::pageSelected( const QString & title )
{
#ifdef QGISDEBUG
    std::cerr << "QgsGrassNewMapset::pageSelected(): " << title << std::endl;
#endif

    int index = indexOf ( currentPage () );

#ifdef QGISDEBUG
    std::cerr << "index = " << index << std::endl;
#endif

    switch (index)
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
	
		mProjectionSelector = new QgsProjectionSelector ( mProjectionFrame, "Projection", 0 );
		mProjectionSelector->setEnabled ( false );
		projectionLayout->addWidget ( mProjectionSelector, 0 , 0 );

		mProjectionSelector->show();
		
		// Warning: QgsProjectionSelector::sridSelected() is not implemented!
		//          -> use lstCoordinateSystems directly
		//connect( mProjectionSelector, SIGNAL(sridSelected(QString)), 
		//	 this, SLOT(sridSelected(QString)));
		connect( mProjectionSelector->lstCoordinateSystems, SIGNAL(selectionChanged ()), 
			 this, SLOT(projectionSelected()));


		// Se current QGIS projection
                int srsid = QgsProject::instance()->readNumEntry(
                       "SpatialRefSys","/ProjectSRSID",0);
    
                QgsSpatialRefSys srs( srsid, QgsSpatialRefSys::QGIS_SRSID );
#ifdef QGISDEBUG
                std::cerr << "current project srsid = " << srsid << std::endl;
                std::cerr << "srs.isValid() = " << srs.isValid() << std::endl;
#endif
                if ( srs.isValid() )
		{
		    mProjectionSelector->setSelectedSRSID ( srsid );
                    mProjRadioButton->setChecked(true); 
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

bool QgsGrassNewMapset::isRunning(void)
{
    return mRunning;
}

void QgsGrassNewMapset::close(void)
{
#ifdef QGISDEBUG
    std::cerr << "QgsGrassNewMapset::close()" << std::endl;
#endif

    hide();
    mRunning = false;
    delete this; 
}

void QgsGrassNewMapset::closeEvent(QCloseEvent *e)
{
#ifdef QGISDEBUG
    std::cerr << "QgsGrassNewMapset::closeEvent()" << std::endl;
#endif

    e->accept();
    close();
}

