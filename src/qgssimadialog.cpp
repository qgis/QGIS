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
#include "qgsmarkersymbol.h"
#include "qgsrenderitem.h"
#include "qgsdlgvectorlayerproperties.h"
#include "qgslegenditem.h"
#include <qfiledialog.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qpainter.h>


QgsSiMaDialog::QgsSiMaDialog(QgsVectorLayer* vectorlayer): QgsSiMaDialogBase(), mVectorLayer(vectorlayer), mMarkerSizeDirty(false)
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
		mScaleEdit->setText(QString::number(scalefactor,'f',2));
		QString svgfile=sy->picture();
		mImageButton->setName(svgfile);
		pic.load(svgfile,"svg");
		QPixmap pixmap(pic.boundingRect().width()*scalefactor,pic.boundingRect().height()*scalefactor);
		pixmap.fill();
		QPainter p(&pixmap);
		p.scale(scalefactor,scalefactor);
		p.drawPicture(0,0,pic);
		mImageButton->setPixmap(pixmap);
		
	    }else
	    {
		qWarning("Warning, typecast failed in qgssimadialog.cpp on line 51");
		mScaleEdit->setText("1.0");
	    }
	}else
	{
	    qWarning("Warning, typecast failed in qgssimadialog.cpp on line 42 or 46");
	}
	

	QObject::connect(mImageButton,SIGNAL(clicked()),this,SLOT(selectMarker()));  
	QObject::connect(mScaleEdit,SIGNAL(returnPressed()),this,SLOT(updateMarkerSize()));
	QObject::connect(mScaleEdit,SIGNAL(textChanged(const QString&)),this,SLOT(setMarkerSizeDirty()));
    }
}

QgsSiMaDialog::QgsSiMaDialog(): QgsSiMaDialogBase(), mVectorLayer(0), mMarkerSizeDirty(false)
{
    QObject::connect(mImageButton,SIGNAL(clicked()),this,SLOT(selectMarker()));
    QObject::connect(mScaleEdit,SIGNAL(returnPressed()),this,SLOT(updateMarkerSize()));
    QObject::connect(mScaleEdit,SIGNAL(textChanged(const QString&)),this,SLOT(setMarkerSizeDirty()));
    mScaleEdit->setText("1.0");
}

QgsSiMaDialog::~QgsSiMaDialog()
{

}

void QgsSiMaDialog::apply()
{   
    qWarning("in QgsSiMaDialog::apply()");
    QgsMarkerSymbol* ms= new QgsMarkerSymbol();
    QString string(mImageButton->name());
    qWarning(string);
    ms->setPicture(string);
    ms->setScaleFactor(mScaleEdit->text().toDouble());

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

    int width = 20+pic.boundingRect().width()*ms->scaleFactor()+fm.width(name);
    int height = (pic.boundingRect().height()*ms->scaleFactor() > fm.height()) ? pic.boundingRect().height()*ms->scaleFactor() +10 : fm.height()+10;
    pix->resize(width, height);
    pix->fill();

    QPainter p(pix);
    p.scale(ms->scaleFactor(),ms->scaleFactor());
    p.drawPicture(10/ms->scaleFactor(),5/ms->scaleFactor(),pic);
    p.resetXForm(); 

    p.setPen(Qt::black);
    p.setFont(f);
    p.drawText(15+pic.boundingRect().width()*ms->scaleFactor(), pix->height() - 10, name);

    if (mVectorLayer->legendItem())
    {
	mVectorLayer->legendItem()->setPixmap(0, (*pix));
	updateMarkerSize();
    }
    
    
    if (mVectorLayer->propertiesDialog())
    {
	mVectorLayer->propertiesDialog()->setRendererDirty(false);
    }
    //repaint the map canvas
    mVectorLayer->triggerRepaint();
}

void QgsSiMaDialog::selectMarker()
{
    QString svgfile=QFileDialog::getOpenFileName(QString::null,"Pictures (*.svg)",0,0,"Choose a marker picture");
    mImageButton->setName(svgfile);
    
    //draw the SVG-Image on the button
    QPicture pic;
    double scalefactor=mScaleEdit->text().toDouble();
    pic.load(svgfile,"svg");
    QPixmap pixmap(pic.boundingRect().width()*scalefactor,pic.boundingRect().height()*scalefactor);
    pixmap.fill();
    QPainter p(&pixmap);
    p.scale(scalefactor,scalefactor);
    p.drawPicture(0,0,pic);
    mImageButton->setPixmap(pixmap);
}

void QgsSiMaDialog::updateMarkerSize()
{
    if(mMarkerSizeDirty)
    {
	//draw the SVG-Image on the button
	QString svgfile(mImageButton->name());
	if(!svgfile.isEmpty())
	{
	    QPicture pic;
	    double scalefactor=mScaleEdit->text().toDouble();
	    pic.load(svgfile,"svg");
	    QPixmap pixmap(pic.boundingRect().width()*scalefactor,pic.boundingRect().height()*scalefactor);
	    pixmap.fill();
	    QPainter p(&pixmap);
	    p.scale(scalefactor,scalefactor);
	    p.drawPicture(0,0,pic);
	    mImageButton->setPixmap(pixmap);
	} 
	mMarkerSizeDirty=false;
    }
}

void QgsSiMaDialog::setMarkerSizeDirty()
{
    mMarkerSizeDirty=true;
}
