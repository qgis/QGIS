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
 
QgsDlgVectorLayerProperties::QgsDlgVectorLayerProperties(QgsVectorLayer *lyr, QWidget *parent, const char *name)
  : QgsDlgVectorLayerPropertiesBase(parent, name), layer(lyr)
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
  legendtypecombobox->insertItem(tr("single symbol"));
	legendtypecombobox->insertItem(tr("graduated symbol"));
	legendtypecombobox->insertItem(tr("continuous color"));
}
QgsDlgVectorLayerProperties::~QgsDlgVectorLayerProperties()
{
}
