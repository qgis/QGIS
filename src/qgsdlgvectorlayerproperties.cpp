/***************************************************************************
                          qgsdlgvectorlayerproperties.cpp
                   Unified property dialog for vector layers
                             -------------------
    begin                : 2004-01-28
    copyright            : (C) 2004 by Gary E.Sherman
    email                : sherman at mrcc.com
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
#include <qstring.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qlistview.h>
#include <qcombobox.h>
#include <qtextstream.h>
#include "qgis.h"
#include "qgsrect.h"
#include "qgsfield.h"
#include "qgsdlgvectorlayerproperties.h"
#include "qgsdataprovider.h"
#include "qgsvectorlayer.h"
#include "qgssinglesymrenderer.h"
#include "qgsgraduatedsymrenderer.h"
#include "qgscontinuouscolrenderer.h"
#include "qgslegenditem.h"
#include <qwidgetstack.h>
#include <qpushbutton.h>
#include "qgssisydialog.h"
#include "qgsgrasydialog.h"
#include "qgscontcoldialog.h"
#include "qobjectlist.h"
 
QgsDlgVectorLayerProperties::QgsDlgVectorLayerProperties(QgsVectorLayer *lyr, QWidget *parent, const char *name): QgsDlgVectorLayerPropertiesBase(parent, name), layer(lyr), rendererDirty(false), bufferDialog(layer->rendererDialog()), bufferRenderer(layer->renderer())
{
  // populate the general information
  QString source = layer->source();
	source = source.left(source.find("password"));
	lblSource->setText(source);
	txtDisplayName->setText(layer->name());
  // display type and feature count
  lblGeometryType->setText(QGis::qgisVectorGeometryType[layer->vectorType()]);
  QgsDataProvider *dp = layer->getDataProvider();
  QString numFeatures;
  numFeatures = numFeatures.setNum(dp->featureCount());
  lblFeatureCount->setText(numFeatures);
  QgsRect *extent = dp->extent();
  QString ll;
  //QTextOStream (&ll) << extent->xMin() << ", " << extent->yMin();
  lblLowerLeft->setText(ll.sprintf("%16f, %16f", extent->xMin(), extent->yMin()));
  QString ur;
//  QTextOStream (&ur) << extent->xMax() << ", " << extent->yMax(); 
  lblUpperRight->setText(ur.sprintf("%16f, %16f", extent->xMax(), extent->yMax()));
  std::vector<QgsField> fields = dp->fields();  	
  // populate the table with the field information
  for(int i = 0; i < fields.size(); i++){
    QgsField fld = fields[i];
    QListViewItem *lvi = new QListViewItem(listViewFields,fld.getName(), 
      fld.getType(), QString("%1").arg(fld.getLength()),
      QString("%1").arg(fld.getPrecision())); 
  }
  // symbology initialization
  legendtypecombobox->insertItem(tr("Single Symbol"));
  legendtypecombobox->insertItem(tr("Graduated Symbol"));
  legendtypecombobox->insertItem(tr("Continuous Color"));

  QObject::connect(legendtypecombobox,SIGNAL(activated(const QString&)),this,SLOT(alterLayerDialog(const QString&)));
  QObject::connect(btnApply,SIGNAL(clicked()),this,SLOT(apply()));
  QObject::connect(btnClose,SIGNAL(clicked()),this,SLOT(close()));

  //insert the renderer dialog of the vector layer into the widget stack
  widgetStackRenderers->addWidget(bufferDialog);
  widgetStackRenderers->raiseWidget(bufferDialog);
  
}

QgsDlgVectorLayerProperties::~QgsDlgVectorLayerProperties()
{
    widgetStackRenderers->removeWidget(bufferDialog);
    if(rendererDirty)
    {
	delete bufferDialog;
	delete bufferRenderer;	
    }
}

void QgsDlgVectorLayerProperties::alterLayerDialog(const QString& string)
{
    if(rendererDirty)
    {
	widgetStackRenderers->removeWidget(bufferDialog);
	delete bufferDialog;
	delete bufferRenderer;	
    }

    //create a new Dialog
    if(string==tr("Single Symbol"))
    {
	qWarning("im single symbol part");
        bufferRenderer=new QgsSingleSymRenderer();
	bufferRenderer->initializeSymbology(layer,this);
    }
    else if(string==tr("Graduated Symbol"))
    {
	qWarning("im graduated symbol part");
	bufferRenderer=new QgsGraduatedSymRenderer();
	bufferRenderer->initializeSymbology(layer,this);
    }
    else if(string==tr("Continuous Color"))
    {
	qWarning("im continuous symbol part");
	bufferRenderer=new QgsContinuousColRenderer();
	bufferRenderer->initializeSymbology(layer,this);
    }
    widgetStackRenderers->addWidget(bufferDialog);
    widgetStackRenderers->raiseWidget(bufferDialog);
    rendererDirty=true;
}

void QgsDlgVectorLayerProperties::setRendererDirty(bool enabled)
{
    rendererDirty=enabled;
}

void QgsDlgVectorLayerProperties::apply()
{
    if(rendererDirty)
    {
	layer->setRenderer(bufferRenderer);
	layer->setRendererDialog(bufferDialog);
    }

    QgsSiSyDialog* sdialog=dynamic_cast<QgsSiSyDialog*>(layer->rendererDialog());
    QgsGraSyDialog* gdialog=dynamic_cast<QgsGraSyDialog*>(layer->rendererDialog());
    QgsContColDialog* cdialog=dynamic_cast<QgsContColDialog*>(layer->rendererDialog());
    if(sdialog)
    {
	sdialog->apply();
    }
    else if(gdialog)
    {
	gdialog->apply();
    }
    else if(cdialog)
    {
	cdialog->apply();
    }
    
    rendererDirty=false;
}


void QgsDlgVectorLayerProperties::close()
{
    if(rendererDirty)
    {
	widgetStackRenderers->removeWidget(bufferDialog);
	delete bufferDialog;
	delete bufferRenderer;
	bufferDialog=layer->rendererDialog();
	bufferRenderer=layer->renderer();
	widgetStackRenderers->addWidget(bufferDialog);
	widgetStackRenderers->raiseWidget(bufferDialog);
	rendererDirty=false;
    }
    reject();
}

QDialog* QgsDlgVectorLayerProperties::getBufferDialog()
{
    return bufferDialog;
}

QgsRenderer* QgsDlgVectorLayerProperties::getBufferRenderer()
{
    return bufferRenderer;
}

void QgsDlgVectorLayerProperties::setLegendType(QString type)
{
    legendtypecombobox->setCurrentText(type);  
}
