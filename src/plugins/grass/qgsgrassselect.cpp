/***************************************************************************
    qgsgrassselect.cpp  -  Select GRASS layer dialog
                             -------------------
    begin                : March, 2004
    copyright            : (C) 2004 by Radim Blazek
    email                : blazek@itc.it
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
#include <qfile.h>
#include <QFileDialog> 
#include <qsettings.h>
#include <qpixmap.h>
#include <q3listbox.h>
#include <qstringlist.h>
#include <qlabel.h>
#include <QComboBox>
#include <qmessagebox.h>
#include <qinputdialog.h>
#include <qsettings.h>

extern "C" {
#include <grass/gis.h>
#include <grass/Vect.h>
}

#include "../../src/providers/grass/qgsgrass.h"
#include "qgsgrassselect.h"

QgsGrassSelect::QgsGrassSelect(int type):QgsGrassSelectBase()
{
#ifdef QGISDEBUG
  std::cerr << "QgsGrassSelect() type = " << type << std::endl;
#endif

  setupUi(this);

  if ( first ) {
    if ( QgsGrass::activeMode() ) {
      lastGisdbase = QgsGrass::getDefaultGisdbase();
      lastLocation = QgsGrass::getDefaultLocation();
      lastMapset = QgsGrass::getDefaultMapset();
    } else {
      QSettings settings;
      lastGisdbase = settings.readEntry("/GRASS/lastGisdbase");
      //check we got something from qsettings otherwise default to users home dir
      if (lastGisdbase.isEmpty())
      {
        QDir home = QDir::home();
        lastGisdbase = QString( home.path() );
      }
    }
    first = false;
  }
    QgsGrassSelect::type = type;

    switch ( type ) 
    {
	case QgsGrassSelect::VECTOR:
	    setCaption ( tr("Select GRASS Vector Layer") );
            break;

	case QgsGrassSelect::RASTER:
	    /* Remove layer combo box */
	    Layer->hide();
	    elayer->hide();
	    setCaption ( tr("Select GRASS Raster Layer") );
	    break;

	case QgsGrassSelect::MAPCALC:
	    /* Remove layer combo box */
	    Layer->hide();
	    elayer->hide();
	    setCaption ( tr("Select GRASS mapcalc schema") );
	    break;
	    
	case QgsGrassSelect::MAPSET:
	    Layer->hide();
	    elayer->hide();
	    MapName->hide();
	    emap->hide();
	    setCaption ( tr("Select GRASS Mapset") );
	    break;
    }
	    
    egisdbase->setText(lastGisdbase);

    setLocations();

    restorePosition();
}

QgsGrassSelect::~QgsGrassSelect()
{
    saveWindowLocation();
}

void QgsGrassSelect::restorePosition()
{
  optionsFrame->adjustSize ();
  adjustSize ();
  
  QSettings settings;
  restoreGeometry(settings.value("/GRASS/windows/select/geometry").toByteArray());
}

void QgsGrassSelect::saveWindowLocation()
{
  QSettings settings;
  settings.setValue("/GRASS/windows/select/geometry", saveGeometry());
} 

bool QgsGrassSelect::first = true;
QString QgsGrassSelect::lastGisdbase;
QString QgsGrassSelect::lastLocation;
QString QgsGrassSelect::lastMapset;
QString QgsGrassSelect::lastVectorMap;
QString QgsGrassSelect::lastRasterMap;
QString QgsGrassSelect::lastLayer;
QString QgsGrassSelect::lastMapcalc;

void QgsGrassSelect::setLocations()
{
    elocation->clear();
    emapset->clear();
    emap->clear();
    elayer->clear();

    QDir d = QDir( egisdbase->text() );

    int idx = 0;
    int sel = -1;
    // Add all subdirs containing PERMANENT/DEFAULT_WIND
    for ( unsigned int i = 0; i < d.count(); i++ ) {
	if ( d[i] == "." || d[i] == ".." ) continue; 

	QString ldpath = egisdbase->text() + "/" + d[i];

        /* TODO: G_is_location() was added to GRASS 6.1 06-05-24,
           enable its use after some period (others do update) */
        /*
        if ( QgsGrass::versionMajor() > 6 || QgsGrass::versionMinor() > 0 )
        { 
            if ( !G_is_location( ldpath.toLocal8Bit().constData() ) ) continue;
        }
        else
        {
        */
	    QString chf = egisdbase->text() + "/" + d[i] + "/PERMANENT/DEFAULT_WIND";
	    if ( !QFile::exists ( chf ) ) continue;
        //}
             	    
	// if type is MAPSET check also if at least one mapset owned by user exists
        if  ( QgsGrassSelect::type == QgsGrassSelect::MAPSET )
	{
	    bool exists = false;
	    
	    QDir ld = QDir( ldpath );
         
	    for ( unsigned int j = 0; j < ld.count(); j++ ) 
            {
                if ( !QgsGrass::isMapset( ldpath + "/" + ld[j] ) ) continue;
		
		QFileInfo info ( ldpath + "/" + ld[j] );
		if ( !info.isWritable() ) continue;

		// TODO: check if owner == user: how to get uer name in QT
		
		exists = true;
		break;
	    }
	
            if ( !exists ) continue;
	}
    
	elocation->insertItem ( QString ( d[i] ), -1 );
	if ( QString ( d[i] ) == lastLocation ) {
	    sel = idx;
	}
	idx++;
    }
    if ( sel >= 0 ) {
        elocation->setCurrentItem(sel);
    }

    setMapsets();
}


void QgsGrassSelect::setMapsets()
{
    #ifdef QGISDEBUG
    std::cerr << "setMapsets()" << std::endl;
    #endif
    
    emapset->clear();
    emap->clear();
    elayer->clear();

    if ( elocation->count() < 1 ) return;

    // Location directory    
    QString ldpath = egisdbase->text() + "/" + elocation->currentText();
    QDir ld = QDir( ldpath );

    int idx = 0;
    int sel = -1;

    // Go through all subdirs and add all subdirs from vector/ 
    for ( unsigned int i = 0; i < ld.count(); i++ ) 
    {
        if ( QgsGrass::isMapset( ldpath + "/" + ld[i] ) ) 
        {
	    emapset->insertItem ( ld[i], -1 );
	    if ( ld[i] == lastMapset ) {
		sel = idx;
	    }
	    idx++;
	}
    }
    if ( sel >= 0 ) {
	emapset->setCurrentItem(sel);
    }

    setMaps();
}

void QgsGrassSelect::setMaps()
{
    #ifdef QGISDEBUG
    std::cerr << "setMaps()" << std::endl;
    #endif

    // Replaced by text box to enable wild cards
    emap->clear();
    elayer->clear();

    if ( emapset->count() < 1 ) return;

    // Mapset directory    
    QString ldpath = egisdbase->text() + "/" + elocation->currentText() + "/" + emapset->currentText();
    QDir ld = QDir( ldpath );

    int idx = 0;
    int sel = -1;

    if (type == VECTOR ) // vector
    {
        QStringList list = QgsGrass::vectors ( egisdbase->text(), 
                              elocation->currentText(), emapset->currentText() );
	
	for ( int j = 0; j < list.count(); j++ ) 
        {
	    emap->insertItem ( list[j], -1 );
	    if ( list[j] == lastVectorMap ) sel = idx;
	    idx++;
	}
        
    } 
    else if ( type == RASTER ) 
    {
	/* add cells */
        QStringList list = QgsGrass::rasters ( egisdbase->text(), 
                              elocation->currentText(), emapset->currentText() );
	
	for ( int j = 0; j < list.count(); j++ ) 
        {
	    emap->insertItem ( list[j], -1 );
	    if ( list[j] == lastRasterMap ) sel = idx;
	    idx++;
	}

	/* add groups */
        // TODO add QgsGrass::groups ( use G_list( G_ELEMENT_GROUP) )
	QDir md = QDir( ldpath + "/group/" );
	md.setFilter (QDir::Dirs);
	
	for ( unsigned int j = 0; j < md.count(); j++ ) {
	    if ( md[j] == "." || md[j] == ".." ) continue; 

	    QString m = QString( md[j] + " (GROUP)" );
	    emap->insertItem ( m, -1 );
	    if ( m == lastRasterMap ) {
		sel = idx;
	    }
	    idx++;
	}
    }
    else if (type == MAPCALC ) 
    {
	QDir md = QDir( ldpath + "/mapcalc/" );
	md.setFilter (QDir::Files);
	
	for ( unsigned int j = 0; j < md.count(); j++ ) {
	    QString m = QString( md[j] );
	    emap->insertItem ( m, -1 );
	    if ( m == lastMapcalc ) {
		sel = idx;
	    }
	    idx++;
	}
    }
    if ( sel >= 0 ) {
	emap->setCurrentItem(sel);
    } 
    /*
    else 
    {
    	emap->clearEdit(); // set box line empty
    }
    */

    setLayers();
}

void QgsGrassSelect::setLayers()
{
    #ifdef QGISDEBUG    
    std::cerr << "setLayers()" << std::endl;
    #endif
    
    elayer->clear();
    
    if (type != VECTOR ) return;
    if ( emap->count() < 1 ) return;

    QStringList layers = vectorLayers ( egisdbase->text(),
              elocation->currentText(), emapset->currentText(),
              emap->currentText().ascii() );
    
    int idx = 0;
    int sel = -1;
    for ( int i = 0; i < layers.count(); i++ ) 
    {
	elayer->insertItem ( layers[i], -1 );
	if ( layers[i] == lastLayer ) sel = idx;
	idx++;
    }
	
    // if last used layer has not been found
    // make default a map from layer 1
    if (sel == -1)
    {
      for ( int j = 0; j < layers.count(); j++ )
      {
        if (layers[j].left(1) == "1")
        {
          sel = j;
          break;
        }
      }
    }
    
    if ( sel >= 0 ) {
        elayer->setCurrentItem(sel);
    } else {
	elayer->clearEdit(); // set box line empty
    }

    if ( elayer->count() == 1 ) {
	elayer->setDisabled(true);
    } else {
	elayer->setDisabled(false);
    }
}

QStringList QgsGrassSelect::vectorLayers ( QString gisdbase,
          QString location, QString mapset, QString mapName )
{
    QStringList list;

    // Set location
    QgsGrass::setLocation ( gisdbase, location);

    /* Open vector */
    QgsGrass::resetError();
    Vect_set_open_level (2);
    struct Map_info map;
    int level;
    
    // Mechanism to recover from fatal errors in GRASS
    // Since fatal error routine in GRASS >= 6.3 terminates the process,
    // we use setjmp() to set recovery place in case of a fatal error.
    // Call to setjmp() returns 0 first time. In case of fatal error,
    // our error routine uses longjmp() to come back to this context,
    // this time setjmp() will return non-zero value and we can continue...
    if (setjmp(QgsGrass::fatalErrorEnv()) == 0)
    {
      level = Vect_open_old_head (&map, (char *) mapName.ascii(), 
                                (char *) mapset.ascii());
    }

    if ( QgsGrass::getError() == QgsGrass::FATAL ) {
	std::cerr << "Cannot open GRASS vector: " << QgsGrass::getErrorMessage().toLocal8Bit().data() << std::endl;
	return list;
    }

    if ( level < 2 ) {
        std::cerr << "Cannot open vector on level 2" << std::endl;
	QMessageBox::warning( 0, tr("Warning"), tr("Cannot open vector on level 2 (topology not available).") );
	return list;
    }

    #ifdef QGISDEBUG
    std::cerr << "GRASS vector successfully opened" << std::endl;
    #endif


    // Get layers
    int ncidx = Vect_cidx_get_num_fields ( &map );

    for ( int i = 0; i < ncidx; i++ ) {
	int field = Vect_cidx_get_field_number ( &map, i);
	QString fs;
	fs.sprintf("%d",field);

        #ifdef QGISDEBUG
        std::cerr << "i = " << i << " layer = " << field << std::endl;
        #endif

	/* Points */
	int npoints = Vect_cidx_get_type_count ( &map, field, GV_POINT);
	if ( npoints > 0 ) {
	    QString l = fs + "_point";
            list.append ( l );
	}

	/* Lines */
	/* Lines without category appears in layer 0, but not boundaries */
	int tp;
	if ( field == 0 ) 
	    tp = GV_LINE;
	else
	    tp = GV_LINE | GV_BOUNDARY;
	
	int nlines = Vect_cidx_get_type_count ( &map, field, tp);
	if ( nlines > 0 ) {
	    QString l = fs + "_line";
            list.append ( l );
	}

	/* Polygons */
	int nareas = Vect_cidx_get_type_count ( &map, field, GV_AREA);
	if ( nareas > 0 ) {
	    QString l = fs + "_polygon";
            list.append ( l );
	}
    }
    Vect_close ( &map );

    return list;
}

void QgsGrassSelect::on_GisdbaseBrowse_clicked()
{
    
    QString Gisdbase = QFileDialog::getExistingDirectory( this,
                                 tr("Choose existing GISDBASE"), egisdbase->text() );

    if ( !Gisdbase.isNull() ) 
    {
        egisdbase->setText ( Gisdbase );
    }
}

void QgsGrassSelect::on_ok_clicked()
{
    saveWindowLocation();

    gisdbase = egisdbase->text();
    lastGisdbase = QString( gisdbase );
    
    if ( elocation->count() == 0 ) {
        QString msg = tr("Wrong GISDBASE, no locations available.");
	QMessageBox::warning(this, tr("Wrong GISDBASE"), msg);
	return;
    }

    //write to qgsettings as gisdbase seems to be valid
    QSettings settings;
    settings.writeEntry("/GRASS/lastGisdbase",lastGisdbase );

    location = elocation->currentText();
    lastLocation = location;

    mapset = emapset->currentText();
    lastMapset = mapset;
    
    map = emap->currentText().stripWhiteSpace();

    if ( type != QgsGrassSelect::MAPSET && map.isEmpty() ) {
        QString msg = tr("Select a map.");
	QMessageBox::warning(0, tr("No map"), msg);
	return;
    }

    if ( type == QgsGrassSelect::VECTOR ) 
    {
        if ( elayer->count() == 0 ) 
        { 
	    QMessageBox::warning(0, tr("No layer"), 
                    tr("No layers available in this map"));
            return;
        }
        lastVectorMap = map;
	layer = elayer->currentText().stripWhiteSpace();
	lastLayer = layer;
    } else if ( type == QgsGrassSelect::RASTER) { 
        lastRasterMap = map;
	if ( map.find(" (GROUP)") != -1 ) {
	    map.remove ( " (GROUP)" );
	    selectedType = QgsGrassSelect::GROUP;
	} else {
	    selectedType = QgsGrassSelect::RASTER;
	}
    } else if ( type == QgsGrassSelect::MAPCALC ) {
	lastMapcalc = map;
    }	
    QDialog::accept();
}

void QgsGrassSelect::on_cancel_clicked()
{
    saveWindowLocation();
    QDialog::reject();
}
