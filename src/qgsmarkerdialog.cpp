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
#include <qfiledialog.h>
#include <qiconview.h>
#include <qlineedit.h>
#include <qpicture.h>
#include <qpushbutton.h>
#include <qpainter.h>
#include <qapplication.h>
#include "qgsconfig.h"

QgsMarkerDialog::QgsMarkerDialog(QString startdir): QgsMarkerDialogBase(0,0,true,Qt::WStyle_StaysOnTop), mCurrentDir(startdir)
{
    QObject::connect(mOkButton,SIGNAL(clicked()),this,SLOT(accept()));
    QObject::connect(mCancelButton,SIGNAL(clicked()),this,SLOT(reject()));
    QObject::connect(mBrowseDirectoriesButton,SIGNAL(clicked()),this,SLOT(changeDirectory()));
    QObject::connect(mIconView,SIGNAL(currentChanged(QIconViewItem*)),this,SLOT(updateSelectedMarker()));
    mDirectoryEdit->setText(startdir);
    visualizeMarkers(startdir);
}

QgsMarkerDialog::~QgsMarkerDialog()
{

}

void QgsMarkerDialog::updateSelectedMarker()
{
    QIconViewItem* current=mIconView->currentItem();
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
    QString newdir=QFileDialog::getExistingDirectory(mCurrentDir,this,"get existing directory","Choose a directory",TRUE);
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
	qWarning(*it);
	
	//render the SVG file to a pixmap and put it into mIconView
	QPixmap pix = QgsSVGCache::instance().getPixmap(mCurrentDir + "/" + 
							(*it), 1);
	QIconViewItem* ivi=new QIconViewItem(mIconView,*it,pix);
	
    }
}

QString QgsMarkerDialog::defaultDir()
{
#ifdef WIN32
	//TODO fix this to use appropriate path on windows 
	//qgis install
	QString dir = "Foo";
#else
    QString dir = QString(PKGDATAPATH)+"/svg"; 
#endif
    return dir;
}

