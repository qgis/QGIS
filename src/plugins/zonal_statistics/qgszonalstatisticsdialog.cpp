/***************************************************************************
                          qgszonalstatisticsdialog.h  -  description
                             -----------------------
    begin                : September 1st, 2009
    copyright            : (C) 2009 by Marco Hugentobler
    email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgszonalstatisticsdialog.h"
#include "qgsmaplayerregistry.h"
#include "qgsrasterlayer.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"

QgsZonalStatisticsDialog::QgsZonalStatisticsDialog(QgisInterface* iface): QDialog(), mIface(iface)
{
  setupUi(this);
  insertAvailableLayers();
  mColumnPrefixLineEdit->setText(proposeAttributePrefix());
}

QgsZonalStatisticsDialog::QgsZonalStatisticsDialog(): QDialog(0), mIface(0)
{
  setupUi(this);
}

QgsZonalStatisticsDialog::~QgsZonalStatisticsDialog()
{

}

void QgsZonalStatisticsDialog::insertAvailableLayers()
{
  //insert available raster layers
  //enter available layers into the combo box
  QMap<QString, QgsMapLayer*> mapLayers = QgsMapLayerRegistry::instance()->mapLayers();
  QMap<QString, QgsMapLayer*>::iterator layer_it = mapLayers.begin();

  for ( ; layer_it != mapLayers.end(); ++layer_it )
  {
    QgsRasterLayer* rl = dynamic_cast<QgsRasterLayer*>( layer_it.value() );
    if ( rl )
    {
      mRasterLayerComboBox->addItem( rl->name(), QVariant( rl->source() ) );
    }
    else
    {
      QgsVectorLayer* vl = dynamic_cast<QgsVectorLayer*>( layer_it.value() );
      if(vl && vl->geometryType() == QGis::Polygon)
      {
        mPolygonLayerComboBox->addItem( vl->name(), QVariant( vl->id() ) );
      }
    }
  }
}

QString QgsZonalStatisticsDialog::rasterFilePath() const
{
  int index = mRasterLayerComboBox->currentIndex();
  if ( index == -1 )
  {
    return "";
  }
  return mRasterLayerComboBox->itemData( index ).toString();
}

QgsVectorLayer* QgsZonalStatisticsDialog::polygonLayer() const
{
  int index = mPolygonLayerComboBox->currentIndex();
  if(index == -1)
  {
    return 0;
  }
  return dynamic_cast<QgsVectorLayer*>(QgsMapLayerRegistry::instance()->mapLayer(mPolygonLayerComboBox->itemData( index ).toString()));
}

QString QgsZonalStatisticsDialog::attributePrefix() const
{
  return mColumnPrefixLineEdit->text();
}

QString QgsZonalStatisticsDialog::proposeAttributePrefix() const
{
  if(!polygonLayer())
  {
    return "";
  }

  QString proposedPrefix = "";
  while(!prefixIsValid(proposedPrefix))
  {
    proposedPrefix.prepend("_");
  }
  return proposedPrefix;
}

bool QgsZonalStatisticsDialog::prefixIsValid(const QString& prefix) const
{
  QgsVectorLayer* vl = polygonLayer();
  if(!vl)
  {
    return false;
  }
  QgsVectorDataProvider* dp = vl->dataProvider();
  if(!dp)
  {
    return false;
  }

  QgsFieldMap providerFieldMap = dp->fields();
  QgsFieldMap::const_iterator it = providerFieldMap.constBegin();
  QString currentFieldName;

  for(; it != providerFieldMap.constEnd(); ++it)
    {
      currentFieldName = it.value().name();
      if(currentFieldName == (prefix + "mean") || currentFieldName == (prefix + "sum") || currentFieldName == (prefix + "count") )
      {
        return false;
      }
    }
  return true;
}
