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


QgsSiMaDialog::QgsSiMaDialog(QgsVectorLayer* vectorlayer): QgsSiMaDialogBase(), mVectorLayer(vectorlayer)
{
    QObject::connect(mImageButton,SIGNAL(clicked()),this,SLOT(selectMarker()));
}

QgsSiMaDialog::QgsSiMaDialog(): QgsSiMaDialogBase(), mVectorLayer(0)
{
    QObject::connect(mImageButton,SIGNAL(clicked()),this,SLOT(selectMarker()));
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
    mImageButton->setName(QFileDialog::getOpenFileName());
}
