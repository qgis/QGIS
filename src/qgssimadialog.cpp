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
#include <qfiledialog.h>
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
                QPicture pic;
                double scalefactor=sy->scaleFactor();
                mScaleSpin->setValue(scalefactor*100);
                QString svgfile=sy->picture();
                pic.load(svgfile,"svg");

                int width=(int)(pic.boundingRect().width()*scalefactor);
                int height=(int)(pic.boundingRect().height()*scalefactor);

                //prevent 0 width or height, which would cause a crash
                if(width==0)
                {
                    width=1;
                }
                if(height==0)
                {
                    height=1;
                }

                QPixmap pixmap(width,height);
                pixmap.fill();
                QPainter p(&pixmap);
                p.scale(scalefactor,scalefactor);
                p.drawPicture(0,0,pic);
                pmPreview->setPixmap(pixmap);

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
    QString string(pmPreview->name());
#ifdef QGISDEBUG

    qWarning(string);
#endif

    ms->setPicture(string);
    //set the scaled factor at the same time converting units from percentage
    ms->setScaleFactor(static_cast<int>(mScaleSpin->value()/100));

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
    QFont f("times", 12, QFont::Normal);
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

    QPicture pic;
    pic.load(string,"svg");

    QPixmap *pix = mVectorLayer->legendPixmap();

    int width = (int)(pic.boundingRect().width()*ms->scaleFactor()+fm.width(name));
    int height = (int)((pic.boundingRect().height()*ms->scaleFactor() > fm.height()) ? pic.boundingRect().height()*ms->scaleFactor() +10 : fm.height()+10);

    //prevent 0 width or height, which would cause a crash
    if(width==0)
    {
        width=1;
    }
    if(height==0)
    {
        height=1;
    }

    pix->resize(width, height);
    pix->fill();

    QPainter p(pix);
    p.scale(ms->scaleFactor(),ms->scaleFactor());
    p.drawPicture((int)(100/ms->scaleFactor()),(int)(100/ms->scaleFactor()),pic);
    p.resetXForm();

    p.setPen(Qt::black);
    p.setFont(f);
    p.drawText((int)(15+pic.boundingRect().width()*ms->scaleFactor()), (int)(pix->height() - 10), name);

    if (mVectorLayer->legendItem())
    {
        mVectorLayer->legendItem()->setPixmap(0, (*pix));
        //updateMarkerSize(0);
    }


    if (mVectorLayer->propertiesDialog())
    {
        mVectorLayer->propertiesDialog()->setRendererDirty(false);
    }
    //repaint the map canvas
    mVectorLayer->triggerRepaint();
}

void QgsSiMaDialog::mIconView_selectionChanged(QIconViewItem * theIconViewItem)
{
    QString svgfile=mCurrentDir+"/"+theIconViewItem->text();
    pmPreview->setName(svgfile);

    //draw the SVG-Image on the button
    QPicture pic;
    double scalefactor=mScaleSpin->value()/100;
    pic.load(svgfile,"svg");

    int width=(int)(pic.boundingRect().width()*scalefactor);
    int height=(int)(pic.boundingRect().height()*scalefactor);

    //prevent 0 width or height, which would cause a crash
    if(width==0)
    {
        width=1;
    }
    if(height==0)
    {
        height=1;
    }

    QPixmap pixmap(height,width);
    pixmap.fill();
    QPainter p(&pixmap);
    p.scale(scalefactor,scalefactor);
    p.drawPicture(0,0,pic);
    pmPreview->setPixmap(pixmap);

}
void QgsSiMaDialog::mScaleSpin_valueChanged( int theSize)
{
    std::cout << "mScaleSpin_valueChanged(" << theSize << ") " << std::endl;
    //draw the SVG-Image on the button
    QString svgfile(pmPreview->name());
    if(!svgfile.isEmpty())
    {
        QPicture pic;
        //user enters scaling factor as a percentage
        double scalefactor=mScaleSpin->value()/100;
        pic.load(svgfile,"svg");

        int width=(int)(pic.boundingRect().width()*scalefactor);
        int height=(int)(pic.boundingRect().height()*scalefactor);

        //prevent 0 width or height, which would cause a crash
        if(width==0)
        {
            width=1;
        }
        if(height==0)
        {
            height=1;
        }

        QPixmap pixmap(width,height);
        pixmap.fill();
        QPainter p(&pixmap);
        p.scale(scalefactor,scalefactor);
        p.drawPicture(0,0,pic);
        pmPreview->setPixmap(pixmap);
    }

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
        qWarning(*it);

        //use the QPixmap way, as the QPicture version does not seem to work properly
        QPicture pic;
        pic.load(mCurrentDir+"/"+(*it),"svg");
        QPixmap pix;
        pix.resize(pic.boundingRect().width(),pic.boundingRect().height());
        pix.fill();
        QPainter p(&pix);
        p.drawPicture(0,0,pic);
        QIconViewItem* ivi=new QIconViewItem(mIconView,*it,pix);

    }
}

QString QgsSiMaDialog::defaultDir()
{
    QString dir = QString(PKGDATAPATH)+"/svg";
    return dir;
}

