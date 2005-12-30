/***************************************************************************
                         qgsmarkerdialog.cpp  -  description
                             -------------------
    begin                : March 2004
    copyright            : (C) 2004 by Marco Hugentobler
    email                : mhugent@geo.unizh.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */

#include "qgsmarkerdialog.h"
#include "qgssvgcache.h"
#include <qdir.h>
#include <QFileDialog>
#include <q3iconview.h>
#include <qlineedit.h>
#include <q3picture.h>
#include <qpushbutton.h>
#include <qpainter.h>
#include <qapplication.h>
#include "qgsconfig.h"
//Added by qt3to4:
#include <QPixmap>

QgsMarkerDialog::QgsMarkerDialog(QString startdir): 
  //paramters removed by Tim during qt4 ui port - FIXME!!!
  //QgsMarkerDialogBase(0,0,true,Qt::WStyle_StaysOnTop), mCurrentDir(startdir)
  QgsMarkerDialogBase(), mCurrentDir(startdir)
{
    QObject::connect(mOkButton,SIGNAL(clicked()),this,SLOT(accept()));
    QObject::connect(mCancelButton,SIGNAL(clicked()),this,SLOT(reject()));
    QObject::connect(mBrowseDirectoriesButton,SIGNAL(clicked()),this,SLOT(changeDirectory()));
    QObject::connect(mIconView,SIGNAL(currentChanged(Q3IconViewItem*)),this,SLOT(updateSelectedMarker()));
    mDirectoryEdit->setText(startdir);
    visualizeMarkers(startdir);
}

QgsMarkerDialog::~QgsMarkerDialog()
{

}

void QgsMarkerDialog::updateSelectedMarker()
{
    Q3IconViewItem* current=mIconView->currentItem();
    if(current)
    {
	mSelectedMarker=current->text();
    }
    else
    {
	mSelectedMarker="";
    }
}

QString QgsMarkerDialog::selectedMarker()
{
    return mCurrentDir+"/"+mSelectedMarker;
}

void QgsMarkerDialog::changeDirectory()
{
    QString newdir = QFileDialog::getExistingDirectory(
        this, "Choose a directory", mCurrentDir);
    if (!newdir.isEmpty())
    {
	mCurrentDir=newdir;
	visualizeMarkers(mCurrentDir);
	mDirectoryEdit->setText(mCurrentDir);
    }
}

void QgsMarkerDialog::visualizeMarkers(QString directory)
{
    mIconView->clear();

    QDir dir(directory);
    QStringList files=dir.entryList("*.svg;*.SVG");
    
    for(QStringList::Iterator it = files.begin(); it != files.end(); ++it )
    {
	qWarning((*it).toLocal8Bit().data());
	
	//render the SVG file to a pixmap and put it into mIconView
	QPixmap pix = QgsSVGCache::instance().getPixmap(mCurrentDir + "/" + 
							(*it), 1);
	Q3IconViewItem* ivi=new Q3IconViewItem(mIconView,*it,pix);
	
    }
}

QString QgsMarkerDialog::defaultDir()
{
#ifdef WIN32
	//TODO fix this to use appropriate path on windows 
	//qgis install
	QString dir = "Foo";
#else
#ifdef Q_OS_MACX
    QString PKGDATAPATH = qApp->applicationDirPath() + "/share/qgis";
#endif
    QString dir = QString(PKGDATAPATH)+"/svg"; 
#endif
    return dir;
}

