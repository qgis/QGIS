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
#include <qfiledialog.h> 
#include <qsettings.h>
#include <qpixmap.h>
#include <qlistbox.h>
#include <qstringlist.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qmessagebox.h>
#include <qinputdialog.h>

extern "C" {
#include <gis.h>
#include <Vect.h>
}

#include "../../providers/grass/qgsgrass.h"
#include "qgsgrassselect.h"

QgsGrassSelect::QgsGrassSelect(int type):QgsGrassSelectBase()
{
    if ( first ) {
	if ( QgsGrass::activeMode() ) {
	    lastGisdbase = QgsGrass::getDefaultGisdbase();
	    lastLocation = QgsGrass::getDefaultLocation();
	    lastMapset = QgsGrass::getDefaultMapset();
	} else {
	    QDir home = QDir::home();
	    lastGisdbase = QString( home.path() );
	}
	first = false;
    }
    QgsGrassSelect::type = type;

    if ( type == QgsGrassSelect::RASTER ) {
	/* Remove layer combo box */
	Layer->hide();
	elayer->hide();
	resize ( width(), height() - 40 );
	setCaption ( "Add GRASS Raster Layer" );
    } else { // vector
	setCaption ( "Add GRASS Vector Layer" );
    }
	    
    egisdbase->setText(lastGisdbase);

    setLocations();
}

QgsGrassSelect::~QgsGrassSelect()
{
}

bool QgsGrassSelect::first = true;
QString QgsGrassSelect::lastGisdbase;
QString QgsGrassSelect::lastLocation;
QString QgsGrassSelect::lastMapset;
QString QgsGrassSelect::lastVectorMap;
QString QgsGrassSelect::lastRasterMap;
QString QgsGrassSelect::lastLayer;

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

	QString chf = egisdbase->text() + "/" + d[i] + "/PERMANENT/DEFAULT_WIND";
	if ( QFile::exists ( chf ) ) {
            elocation->insertItem ( QString ( d[i] ), -1 );
	    if ( QString ( d[i] ) == lastLocation ) {
		sel = idx;
	    }
	    idx++;
	}
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
    for ( unsigned int i = 0; i < ld.count(); i++ ) {
	QString windf = ldpath + "/" + ld[i] + "/WIND";
	
	if ( QFile::exists ( windf ) ) {
	    emapset->insertItem ( ld[i], -1 );
	    if ( ld[i] == lastMapset ) {
		sel = idx;
	    }
	    idx++;
	}
    }
    if ( idx >= 0 ) {
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

    if (type == VECTOR ) { // vector
	ldpath.append ( "/vector/" );
	QDir md = QDir( ldpath );
	
	for ( unsigned int j = 0; j < md.count(); j++ ) {
	    QString chf = ldpath + md[j] + "/head";
	    
	    if ( QFile::exists ( chf ) ) {
		QString m = QString( md[j] );
		emap->insertItem ( m, -1 );
		if ( m == lastVectorMap ) {
		    sel = idx;
		}
		idx++;
	    }
	}
    } else { // raster
	/* add cells */
	QDir md = QDir( ldpath + "/cell/" );
	md.setFilter (QDir::Files);
	
	for ( unsigned int j = 0; j < md.count(); j++ ) {
	    QString m = QString( md[j] );
	    emap->insertItem ( m, -1 );
	    if ( m == lastRasterMap ) {
		sel = idx;
	    }
	    idx++;
	}

	/* add groups */
	md = QDir( ldpath + "/group/" );
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
    if ( idx >= 0 ) {
	emap->setCurrentItem(sel);
    } else {
	emap->clearEdit(); // set box line empty
    }

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

    // Set location
    QgsGrass::setLocation ( egisdbase->text(), elocation->currentText() );

    /* Open vector */
    QgsGrass::resetError();
    Vect_set_open_level (2);
    struct Map_info map;
    int level = Vect_open_old_head (&map, (char *) emap->currentText().ascii(), 
	                            (char *) emapset->currentText().ascii());

    if ( QgsGrass::getError() == QgsGrass::FATAL ) {
	std::cerr << "Cannot open GRASS vector: " << QgsGrass::getErrorMessage() << std::endl;
	return;
    }

    if ( level < 2 ) {
        std::cerr << "Cannot open vector on level 2" << std::endl;
	QMessageBox::warning( 0, "Warning", "Cannot open vector on level 2 (topology not available)." );
	return;
    }

    #ifdef QGISDEBUG
    std::cerr << "GRASS vector successfully opened" << std::endl;
    #endif

    int idx = 0;
    int sel = -1;

    // Get layers
    int ncidx = Vect_cidx_get_num_fields ( &map );

    for ( int i = 0; i < ncidx; i++ ) {
	int field = Vect_cidx_get_field_number ( &map, i);
	QString fs;
	fs.sprintf("%d",field);

	/* Points */
	int npoints = Vect_cidx_get_type_count ( &map, field, GV_POINT);
	if ( npoints > 0 ) {
	    QString l = fs + "_point";
	    elayer->insertItem ( l, -1 );
	    if ( l == lastLayer ) sel = idx;
	    idx++;
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
	    elayer->insertItem ( l, -1 );
	    if ( l == lastLayer ) sel = idx;
	    idx++;
	}

	/* Polygons */
	int nareas = Vect_cidx_get_type_count ( &map, field, GV_AREA);
	if ( nareas > 0 ) {
	    QString l = fs + "_polygon";
	    elayer->insertItem ( l, -1 );
	    if ( l == lastLayer ) sel = idx;
	    idx++;
	}
    }
    Vect_close ( &map );
	
    if ( idx >= 0 ) {
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
void QgsGrassSelect::getGisdbase()
{
    
    QString Gisdbase = QFileDialog::getExistingDirectory( egisdbase->text(), this, 
	                            "get existing GISDBASE" , "Choose existing GISDBASE", TRUE );
    egisdbase->setText ( Gisdbase );
}

void QgsGrassSelect::accept()
{
    gisdbase = egisdbase->text();
    lastGisdbase = QString( gisdbase );
    
    if ( elocation->count() == 0 ) {
        QString msg = "Wrong GISDBASE, no locations available.";
	QMessageBox::warning(this, "Wrong GISDBASE", msg);
	return;
    }

    location = elocation->currentText();
    lastLocation = location;

    mapset = emapset->currentText();
    lastMapset = mapset;
    
    map = emap->currentText().stripWhiteSpace();

    if ( map.isEmpty() ) {
        QString msg = "Select a map.";
	QMessageBox::warning(0, "No map", msg);
	return;
    }
    

    if ( type == QgsGrassSelect::VECTOR ) {
        lastVectorMap = map;
	layer = elayer->currentText().stripWhiteSpace();
	lastLayer = layer;
    } else { // RASTER
        lastRasterMap = map;
	if ( map.find(" (GROUP)") != -1 ) {
	    map.remove ( " (GROUP)" );
	    selectedType = QgsGrassSelect::GROUP;
	} else {
	    selectedType = QgsGrassSelect::RASTER;
	}
    }
    
    QDialog::accept();
}

void QgsGrassSelect::reject()
{
    QDialog::reject();
}
