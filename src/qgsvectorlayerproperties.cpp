/***************************************************************************
                          qgsvectorlayerproperties.cpp  -  description
                             -------------------
    begin                : Sun Aug 11 2002
    copyright            : (C) 2002 by Gary E.Sherman
    email                : sherman at mrcc dot com
       Romans 3:23=>Romans 6:23=>Romans 5:8=>Romans 10:9,10=>Romans 12
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
#include <qframe.h>
#include <qcolordialog.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qspinbox.h>
#include <qstring.h>
#include <qlabel.h>
#include "qgsvectorlayer.h"
#include "qgssymbol.h"
#include "qgsvectorlayerproperties.h"
#include "qtabwidget.h"
#include <iostream>
#include <qcombobox.h>
//#include "qgssisydialog.h"
#include "qgssinglesymrenderer.h"
#include "qgsgraduatedsymrenderer.h"
#include <cfloat>
#include "qgslegenditem.h"
#include "qgscontinuouscolrenderer.h"
#include "qgssisydialog.h"
#include "qgsgrasydialog.h"
#include "qgscontcoldialog.h"

QgsVectorLayerProperties::QgsVectorLayerProperties(QgsVectorLayer* lyr):layer(lyr), rendererDirty(false), bufferDialog(layer->rendererDialog()), bufferRenderer(layer->renderer())
{
	// populate the property sheet based on the layer properties
	// general info
	QString source = lyr->source();
	source = source.left(source.find("password"));
	lblSource->setText(source);
	setCaption("Layer Properties - " + lyr->name());
	connect( settingsbutton, SIGNAL( clicked() ), this, SLOT( showSymbolSettings() ) );
	legendtypecombobox->insertItem(tr("single symbol"));
	legendtypecombobox->insertItem(tr("graduated symbol"));
	legendtypecombobox->insertItem(tr("continuous color"));
	QObject::connect(legendtypecombobox,SIGNAL(activated(const QString&)),this,SLOT(alterLayerDialog(const QString&)));
	QObject::connect(buttonOk,SIGNAL(clicked()),this,SLOT(apply()));
	QObject::connect(buttonCancel,SIGNAL(clicked()),this,SLOT(cancel()));
}

QgsVectorLayerProperties::~QgsVectorLayerProperties()
{
 
}

QgsSymbol* QgsVectorLayerProperties::getSymbol()
{

}

void QgsVectorLayerProperties::alterLayerDialog(const QString& string)
{
    if(rendererDirty)
    {
	delete bufferDialog;
	delete bufferRenderer;	
    }

    //create a new Dialog
    if(string==tr("single symbol"))
    {
        bufferRenderer=new QgsSingleSymRenderer();
	bufferRenderer->initializeSymbology(layer,this);
    }
    else if(string==tr("graduated symbol"))
    {
	bufferRenderer=new QgsGraduatedSymRenderer();
	bufferRenderer->initializeSymbology(layer,this);
    }
    else if(string==tr("continuous color"))
    {
	bufferRenderer=new QgsContinuousColRenderer();
	bufferRenderer->initializeSymbology(layer,this);
    }
    rendererDirty=true;
}

void QgsVectorLayerProperties::showSymbolSettings()
{
    bufferDialog->show();
    bufferDialog->raise();
}

void QgsVectorLayerProperties::setLegendType(QString type)
{
    legendtypecombobox->setCurrentText(type);  
}

void QgsVectorLayerProperties::apply()
{
    if(rendererDirty)
    {
	layer->setRenderer(bufferRenderer);
	layer->setRendererDialog(bufferDialog);
	rendererDirty=false;
	//copy the bufferPixmap to the vectorlayer and to its legend item
	*(layer->legendPixmap())=bufferPixmap;
	if(layer->legendItem())
	{
	    layer->legendItem()->setPixmap(0,bufferPixmap);
	}
    }
    accept();
    layer->triggerRepaint();
}

void QgsVectorLayerProperties::cancel()
{
    //todo: add code to free memory here
    if(rendererDirty)
    {
	delete bufferDialog;
	delete bufferRenderer;
    }
    reject();
}

