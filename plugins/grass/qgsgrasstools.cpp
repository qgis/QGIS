/***************************************************************************
                              qgsgrasstools.cpp
                             -------------------
    begin                : March, 2005
    copyright            : (C) 2005 by Radim Blazek
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

#include <qapplication.h>
#include <qdir.h>
#include <qfile.h>
#include <qfiledialog.h> 
#include <qsettings.h>
#include <qpixmap.h>
#include <qlistbox.h>
#include <qstringlist.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qspinbox.h>
#include <qmessagebox.h>
#include <qinputdialog.h>
#include <qsettings.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qpen.h>
#include <qpointarray.h>
#include <qcursor.h>
#include <qnamespace.h>
#include <qlistview.h>
#include <qcolordialog.h>
#include <qtable.h>
#include <qstatusbar.h>
#include <qevent.h>
#include <qpoint.h>
#include <qsize.h>
#include <qdom.h>
#include <qtabwidget.h>
#include <qlayout.h>
#include <qcheckbox.h>
#include <qprocess.h>
#include <qiconset.h>

#include "../../src/qgis.h"
#include "../../src/qgsmapcanvas.h"
#include "../../src/qgsmaplayer.h"
#include "../../src/qgsvectorlayer.h"
#include "../../src/qgsdataprovider.h"
#include "../../src/qgsfield.h"
#include "../../src/qgsfeatureattribute.h"

extern "C" {
#include <gis.h>
#include <Vect.h>
}

#include "../../providers/grass/qgsgrass.h"
#include "../../providers/grass/qgsgrassprovider.h"
#include "qgsgrassattributes.h"
#include "qgsgrasstools.h"
#include "qgsgrassmodule.h"

QgsGrassTools::QgsGrassTools ( QgisApp *qgisApp, QgisIface *iface, 
	                     QWidget * parent, const char * name, WFlags f )
             :QgsGrassToolsBase ( parent, name, f )
{
    #ifdef QGISDEBUG
    std::cerr << "QgsGrassTools()" << std::endl;
    #endif

    mQgisApp = qgisApp;
    mIface = iface;
    mCanvas = mIface->getMapCanvas();

    // Set list view
    mModulesListView->setColumnText(0,"Modules");
    mModulesListView->clear();
    mModulesListView->setSorting(-1);
    mModulesListView->setRootIsDecorated(true);
    mModulesListView->setResizeMode(QListView::AllColumns);
    mModulesListView->header()->hide();

    connect( mModulesListView, SIGNAL(clicked(QListViewItem *)), 
		         this, SLOT(moduleClicked( QListViewItem *)) );

#if defined(WIN32) || defined(Q_OS_MACX)
    mAppDir = qApp->applicationDirPath();
#else
    mAppDir = PREFIX;
#endif

    QString conf = mAppDir + "/share/qgis/grass/config/default.qgc";
    loadConfig ( conf );
    statusBar()->hide();
    restorePosition();
}

void QgsGrassTools::moduleClicked( QListViewItem * item )
{
    if ( !item ) return;

    QString name = item->text(1);
    //std::cerr << "name = " << name << std::endl;
    
    if ( name.length() == 0 ) return;  // Section
    
    QString path = mAppDir + "/share/qgis/grass/modules/" + name;
    QgsGrassModule *m = new QgsGrassModule ( this, mQgisApp, mIface, path, mTabWidget );
    //mTabWidget->addTab ( m, item->text(0) );
    
    QPixmap pixmap = QgsGrassModule::pixmap ( path, 25 ); 
    QIconSet is;
    is.setPixmap ( pixmap, QIconSet::Small, QIconSet::Normal );
    mTabWidget->addTab ( (QWidget*)m, is, "" );
		
    mTabWidget->setCurrentPage ( mTabWidget->count()-1 );
}

bool QgsGrassTools::loadConfig(QString filePath)
{
    #ifdef QGISDEBUG
    std::cerr << "QgsGrassTools::loadConfig(): " << filePath.local8Bit() << std::endl;
    #endif
    mModulesListView->clear();

    QFile file ( filePath );

    if ( !file.exists() ) {
	QMessageBox::warning( 0, "Warning", "The config file (" + filePath + ") not found." );
	return false;
    }
    if ( ! file.open( IO_ReadOnly ) ) {
	QMessageBox::warning( 0, "Warning", "Cannot open config file (" + filePath + ")" );
	return false;
    }
    
    QDomDocument doc ( "qgisgrass" );
    QString err;
    int line, column;
    if ( !doc.setContent( &file,  &err, &line, &column ) ) {
	QString errmsg = "Cannot read config file (" + filePath + "):\n" + err + "\nat line "  
	                 + QString::number(line) + " column " + QString::number(column);
	std::cerr << errmsg.local8Bit() << std::endl;
	QMessageBox::warning( 0, "Warning", errmsg );
	file.close();
	return false;
    }

    QDomElement docElem = doc.documentElement();
    QDomNodeList modulesNodes = docElem.elementsByTagName ( "modules" );

    if ( modulesNodes.count() == 0 ) {
	 file.close();
	 return false;
    }

    QDomNode modulesNode = modulesNodes.item(0);
    QDomElement modulesElem = modulesNode.toElement();
    
    // Go through the sections and modules and add them to the list view
    addModules ( 0, modulesElem );
    
    file.close();
}

void QgsGrassTools::addModules (  QListViewItem *parent, QDomElement &element )
{
    QDomNode n = element.firstChild();

    QListViewItem *item;
    QListViewItem *lastItem = 0;
    while( !n.isNull() ) {
	QDomElement e = n.toElement();
	if( !e.isNull() ) {
	    //std::cout << "tag = " << e.tagName() << std::endl;

	    if ( e.tagName() == "section" && e.tagName() == "grass" ) {
		std::cout << "Unknown tag: " << e.tagName() << std::endl;
		continue;
	    }
	    
	    if ( parent ) {
		item = new QListViewItem( parent, lastItem );
	    } else {
		item = new QListViewItem( mModulesListView, lastItem );
	    }

	    if ( e.tagName() == "section" ) {
		QString label = e.attribute("label");
	        std::cout << "label = " << label.local8Bit() << std::endl;
		item->setText( 0, label );
		item->setOpen(true); // for debuging to spare one click

		addModules ( item, e );
		
		lastItem = item;
	    } else if ( e.tagName() == "grass" ) { // GRASS module
		QString name = e.attribute("name");
	        std::cout << "name = " << name.local8Bit() << std::endl;

                QString path = mAppDir + "/share/qgis/grass/modules/" + name;
                QString label = QgsGrassModule::label ( path );
		QPixmap pixmap = QgsGrassModule::pixmap ( path, 25 ); 

		item->setText( 0, label );
		item->setPixmap( 0, pixmap );
		item->setText( 1, name );
		lastItem = item;
	    }
	}
	n = n.nextSibling();
    }
}

QgsGrassTools::~QgsGrassTools()
{
    #ifdef QGISDEBUG
    std::cerr << "QgsGrassTools::~QgsGrassTools()" << std::endl;
    #endif
    saveWindowLocation();
}

QString QgsGrassTools::appDir(void)
{
    return mAppDir;
}

void QgsGrassTools::close(void)
{
    saveWindowLocation();
    hide();
}

void QgsGrassTools::closeEvent(QCloseEvent *e)
{
    saveWindowLocation();
    e->accept();
}

void QgsGrassTools::restorePosition()
{
    QSettings settings;
    int ww = settings.readNumEntry("/qgis/grass/windows/tools/w", 250);
    int wh = settings.readNumEntry("/qgis/grass/windows/tools/h", 300);
    int wx = settings.readNumEntry("/qgis/grass/windows/tools/x", 100);
    int wy = settings.readNumEntry("/qgis/grass/windows/tools/y", 100);
    resize(ww,wh);
    move(wx,wy);
    QgsGrassToolsBase::show();
}

void QgsGrassTools::saveWindowLocation()
{
    QSettings settings;
    QPoint p = this->pos();
    QSize s = this->size();
    settings.writeEntry("/qgis/grass/windows/tools/x", p.x());
    settings.writeEntry("/qgis/grass/windows/tools/y", p.y());
    settings.writeEntry("/qgis/grass/windows/tools/w", s.width());
    settings.writeEntry("/qgis/grass/windows/tools/h", s.height());
}



