/***************************************************************************
  labelinggui.cpp
  Smart labeling for vector layers
  -------------------
         begin                : June 2009
         copyright            : (C) Martin Dobias
         email                : wonder.sk at gmail.com

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "labelinggui.h"

#include <qgsvectorlayer.h>
#include <qgsvectordataprovider.h>
#include <qgsmaplayerregistry.h>

#include "pallabeling.h"
#include "engineconfigdialog.h"

#include <QColorDialog>
#include <QFontDialog>

#include <iostream>
#include <QApplication>

LabelingGui::LabelingGui( PalLabeling* lbl, QString layerId, QWidget* parent )
    : QDialog( parent ), mLBL( lbl ), mLayerId( layerId )
{
  setupUi( this );

  connect(btnTextColor, SIGNAL(clicked()), this, SLOT(changeTextColor()) );
  connect(btnChangeFont, SIGNAL(clicked()), this, SLOT(changeTextFont()) );
  connect(btnEngineSettings, SIGNAL(clicked()), this, SLOT(showEngineConfigDialog()) );

  populatePlacementMethods();
  populateFieldNames();

  LayerSettings lyr = lbl->layer(layerId);
  if (!lyr.layerId.isEmpty())
  {
    // load the labeling settings
    cboPlacement->setCurrentIndex( cboPlacement->findData( QVariant( (int)lyr.placement ) ) );
    cboFieldName->setCurrentIndex( cboFieldName->findText(lyr.fieldName) );
    chkEnableLabeling->setChecked( lyr.enabled );
    sliderPriority->setValue( lyr.priority );
    chkNoObstacle->setChecked( !lyr.obstacle );
  }
  else
  {
    // set enabled by default
    chkEnableLabeling->setChecked( true );
  }

  btnTextColor->setColor( lyr.textColor );
  updateFontPreview( lyr.textFont );
}

LabelingGui::~LabelingGui()
{
}

QgsVectorLayer* LabelingGui::layer()
{
  QgsMapLayer* layer = QgsMapLayerRegistry::instance()->mapLayer(mLayerId);
  if (layer == NULL || layer->type() != QgsMapLayer::VectorLayer)
    return NULL;
  return static_cast<QgsVectorLayer*>(layer);
}

LayerSettings LabelingGui::layerSettings()
{
  LayerSettings lyr;
  lyr.layerId = mLayerId;
  lyr.fieldName = cboFieldName->currentText();
  lyr.placement = (LayerSettings::Placement) cboPlacement->itemData(cboPlacement->currentIndex()).toInt();
  lyr.textColor = btnTextColor->color();
  lyr.textFont = lblFontPreview->font();
  lyr.enabled = chkEnableLabeling->isChecked();
  lyr.priority = sliderPriority->value();
  lyr.obstacle = !chkNoObstacle->isChecked();

  return lyr;
}

void LabelingGui::populatePlacementMethods()
{
  switch (layer()->geometryType())
  {
    case QGis::Point:
      cboPlacement->addItem(tr("Around the point"), QVariant(LayerSettings::AroundPoint));
      break;
    case QGis::Line:
      cboPlacement->addItem(tr("On the line"), QVariant(LayerSettings::OnLine));
      cboPlacement->addItem(tr("Around the line"), QVariant(LayerSettings::AroundLine));
      break;
    case QGis::Polygon:
      cboPlacement->addItem(tr("Horizontal"), QVariant(LayerSettings::Horizontal));
      cboPlacement->addItem(tr("Free"), QVariant(LayerSettings::Free));
      cboPlacement->addItem(tr("Around the centroid"), QVariant(LayerSettings::AroundPoint));
      cboPlacement->addItem(tr("On the perimeter"), QVariant(LayerSettings::OnLine));
      cboPlacement->addItem(tr("Around the perimeter"), QVariant(LayerSettings::AroundLine));
      break;
  }
}

void LabelingGui::populateFieldNames()
{
  QgsFieldMap fields = layer()->dataProvider()->fields();
  for (QgsFieldMap::iterator it = fields.begin(); it != fields.end(); it++)
  {
    cboFieldName->addItem(it->name());
  }
}

void LabelingGui::changeTextColor()
{
  QColor color = QColorDialog::getColor( btnTextColor->color(), this);
  if (!color.isValid())
    return;

  btnTextColor->setColor(color);
  updateFontPreview( lblFontPreview->font() );
}

void LabelingGui::changeTextFont()
{
  bool ok;
  QFont font = QFontDialog::getFont(&ok, lblFontPreview->font(), this);
  if (ok)
    updateFontPreview( font );
}

void LabelingGui::updateFontPreview(QFont font)
{
  lblFontName->setText( QString("%1, %2").arg(font.family()).arg(font.pointSize()) );
  lblFontPreview->setFont(font);

  QPalette palette = lblFontPreview->palette();
  QBrush brush(btnTextColor->color());
  palette.setBrush(QPalette::Active, QPalette::WindowText, brush);
  palette.setBrush(QPalette::Inactive, QPalette::WindowText, brush);
  lblFontPreview->setPalette(palette);
}

void LabelingGui::showEngineConfigDialog()
{
  EngineConfigDialog dlg(mLBL, this);
  dlg.exec();
}
