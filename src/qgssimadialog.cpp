/***************************************************************************
                          qgssimadialog.cpp
                         Single marker renderer dialog
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

#include "qgssimadialog.h"
#include "qgssimarenderer.h"
#include "qgsvectorlayer.h"
#include "qgsmarkerdialog.h"
#include "qgsmarkersymbol.h"
#include "qgsrenderitem.h"
#include "qgsdlgvectorlayerproperties.h"
#include "qgslegenditem.h"
#include "qgssvgcache.h"
#include <qapplication.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qfiledialog.h>
#include <qimage.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qpainter.h>
#include <qspinbox.h>
#include <qiconview.h>
#include <qlabel.h>

QgsSiMaDialog::QgsSiMaDialog(QgsVectorLayer* vectorlayer): QgsSiMaDialogBase(), mVectorLayer(vectorlayer)
{
    if(mVectorLayer)
    {
        QgsSiMaRenderer *renderer;

        //initial settings, use the buffer of the propertiesDialog if possible. If this is not possible, use the renderer of the vectorlayer directly
        if (mVectorLayer->propertiesDialog())
        {
             renderer = dynamic_cast < QgsSiMaRenderer * >(mVectorLayer->propertiesDialog()->getBufferRenderer());
        }
        else
        {
            renderer = dynamic_cast < QgsSiMaRenderer * >(mVectorLayer->renderer());
        }

        if(renderer)
        {
            QgsMarkerSymbol* sy=dynamic_cast < QgsMarkerSymbol* >(renderer->item()->getSymbol());
            if(sy)
            {
                double scalefactor=sy->scaleFactor();
                mScaleSpin->setValue((int)(scalefactor*100.0));
                mSelectedMarker=sy->picture();
                pmPreview->setPixmap(QgsSVGCache::instance().
				     getPixmap(mSelectedMarker, scalefactor));
            }
            else
            {
                qWarning("Warning, typecast failed in qgssimadialog.cpp on line 51");
                mScaleSpin->setValue(100);
            }
        }
        else
        {
            qWarning("Warning, typecast failed in qgssimadialog.cpp on line 42 or 46");
        }
  
  //set the dir to the default svg dir
#if defined(WIN32) || defined(Q_OS_MACX)
        QString PKGDATAPATH = qApp->applicationDirPath() + "/share/qgis";
#endif
        mCurrentDir=QString(PKGDATAPATH)+"/svg/";
#ifdef QGISDEBUG
  qWarning("mCurrentDir in constructor: "+mCurrentDir);
#endif
        visualizeMarkers(mCurrentDir);
        mDirectoryEdit->setText(mCurrentDir);
        //QString(PKGDATAPATH);
    }
}



QgsSiMaDialog::~QgsSiMaDialog()
{
    //do nothing
}

void QgsSiMaDialog::apply()
{
#ifdef QGISDEBUG
    qWarning("in QgsSiMaDialog::apply()");
#endif

    QgsMarkerSymbol* ms= new QgsMarkerSymbol();
    QString string(mSelectedMarker);
#ifdef QGISDEBUG
    qWarning(string);
#endif

    ms->setPicture(string);
    //set the scaled factor at the same time converting units from percentage
    ms->setScaleFactor(mScaleSpin->value()/100.0);

    QgsRenderItem* ri = new QgsRenderItem();
    ri->setSymbol(ms);

    QgsSiMaRenderer *renderer = dynamic_cast < QgsSiMaRenderer * >(mVectorLayer->renderer());

    if( renderer )
    {
        renderer->addItem(ri);
    }
    else
    {
        qWarning("typecast failed in QgsSiMaDialog::apply()");
        return;
    }

    //add a pixmap to the legend item

    //font tor the legend text
    QFont f("arial", 10, QFont::Normal);
    QFontMetrics fm(f);

    QString name;
    if (mVectorLayer->propertiesDialog())
    {
        name = mVectorLayer->propertiesDialog()->displayName();
    }
    else
    {
        name = "";
    }


    QPixmap *pix = mVectorLayer->legendPixmap();

    //spaces between legend pixmap elements
    int leftspace=5;
    int topspace=5;
    int bottomspace=5;
    int betweenspace=5;
    int rightspace=5;
    
    // get the marker used, calculate width and height of the legend pixmap
    QPixmap symbolPix = 
      QgsSVGCache::instance().getPixmap(string, ms->scaleFactor());
    int width = symbolPix.width() + fm.width(name) + leftspace + 
      betweenspace + rightspace;
    int height = (symbolPix.height() > fm.height() ? 
		  symbolPix.height() + topspace+bottomspace : 
		  fm.height() + topspace+bottomspace);
    pix->resize(width, height);
    pix->fill();
    
    // paint symbol and layer name
    QPainter p(pix);
    p.drawPixmap(leftspace, topspace, symbolPix);
    p.setPen(Qt::black);
    p.setFont(f);
    p.drawText(leftspace + betweenspace + symbolPix.width(), 
	       pix->height() - bottomspace,name);

    mVectorLayer->updateItemPixmap();

    if (mVectorLayer->propertiesDialog())
    {
        mVectorLayer->propertiesDialog()->setRendererDirty(false);
    }
    //repaint the map canvas
    mVectorLayer->triggerRepaint();
}

void QgsSiMaDialog::setMarker(const QString& file, double scaleFactor) {
  QFile f(file);
  QFileInfo fi(f);

  if (fi.exists()) {
    // set the directory
    mCurrentDir = fi.dir().path() + "/";
    visualizeMarkers(mCurrentDir);
    mDirectoryEdit->setText(mCurrentDir);
  
    QIconViewItem* item = mIconView->findItem(fi.fileName(), Qt::ExactMatch);
    if (item) {
      // set the picture
      mIconView->setSelected(item, true);
      
      // set the scale factor
      mScaleSpin->setValue(scaleFactor * 100.0);
    }
  }
  emit settingsChanged();
}

const QString& QgsSiMaDialog::getPicture() const {
  return mSelectedMarker;
}

double QgsSiMaDialog::getScaleFactor() const {
  return mScaleSpin->value() / 100.0;
}

void QgsSiMaDialog::mIconView_selectionChanged(QIconViewItem * theIconViewItem)
{
#ifdef Q_OS_MACX
    mSelectedMarker=mCurrentDir+"/"+theIconViewItem->text();
#else
    mSelectedMarker=mCurrentDir+theIconViewItem->text();
#endif

    //draw the SVG-Image on the button
    double scalefactor=mScaleSpin->value()/100.0;
    pmPreview->setPixmap(QgsSVGCache::instance().
			 getPixmap(mSelectedMarker, scalefactor));    
    
    emit settingsChanged();
}

void QgsSiMaDialog::mScaleSpin_valueChanged( int theSize)
{
#ifdef QGISDEBUG
    std::cout << "mScaleSpin_valueChanged(" << theSize << ") " << std::endl;
#endif
    //draw the SVG-Image on the button
    if(!mSelectedMarker.isEmpty())
    {
        //user enters scaling factor as a percentage
        double scalefactor=mScaleSpin->value()/100.0;
	pmPreview->setPixmap(QgsSVGCache::instance().
			     getPixmap(mSelectedMarker, scalefactor));
    }
    
    emit settingsChanged();
}

void QgsSiMaDialog::mBrowseDirectoriesButton_clicked()
{
    QString newdir=QFileDialog::getExistingDirectory(mCurrentDir,this,"get existing directory","Choose a directory",TRUE);
    if (!newdir.isEmpty())
    {
        mCurrentDir=newdir;
        visualizeMarkers(mCurrentDir);
        mDirectoryEdit->setText(mCurrentDir);
    }
}

void QgsSiMaDialog::visualizeMarkers(QString directory)
{
    mIconView->clear();

    QDir dir(directory);
    QStringList files=dir.entryList("*.svg;*.SVG");

    for(QStringList::Iterator it = files.begin(); it != files.end(); ++it )
    {
#ifdef QGISDEBUG
        qWarning(*it);
#endif
	QPixmap pix = QgsSVGCache::instance().getPixmap(mCurrentDir + 
							"/" + (*it), 1);
        QIconViewItem* ivi=new QIconViewItem(mIconView,*it,pix);
    }
}

QString QgsSiMaDialog::defaultDir()
{
#if defined(WIN32) || defined(Q_OS_MACX)
    QString PKGDATAPATH = qApp->applicationDirPath() + "/share/qgis";
#endif
    QString dir = QString(PKGDATAPATH)+"/svg";
    return dir;
}

