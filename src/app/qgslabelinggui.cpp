/***************************************************************************
  qgslabelinggui.cpp
  Smart labeling for vector layers
  -------------------
         begin                : June 2009
         copyright            : (C) Martin Dobias
         email                : wonder dot sk at gmail dot com

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslabelinggui.h"

#include "qgsmapcanvas.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"
#include "qgsmaplayerregistry.h"
#include "qgslabelengineconfigdialog.h"
#include "qgsexpressionbuilderdialog.h"
#include "qgsexpression.h"
#include "qgisapp.h"
#include "qgsmaprenderer.h"
#include "qgsproject.h"

#include <QApplication>

QgsLabelingGui::QgsLabelingGui( QgsVectorLayer* layer, QgsMapCanvas* mapCanvas, QWidget* parent )
    : QWidget( parent )
    , mLayer( layer )
    , mMapCanvas( mapCanvas )
{
  if ( !layer )
    return;

  setupUi( this );

  mSettingsWidget->setLayer( mLayer );

  // main layer label-enabling connections
  connect( chkEnableLabeling, SIGNAL( toggled( bool ) ), mFieldExpressionWidget, SLOT( setEnabled( bool ) ) );
  connect( chkEnableLabeling, SIGNAL( toggled( bool ) ), mLabelingFrame, SLOT( setEnabled( bool ) ) );

  // internal connections
  connect( btnEngineSettings, SIGNAL( clicked() ), this, SLOT( showEngineConfigDialog() ) );

  // field combo and expression button
  mFieldExpressionWidget->setLayer( mLayer );
  QgsDistanceArea myDa;
  myDa.setSourceCrs( mLayer->crs().srsid() );
  myDa.setEllipsoidalMode( QgisApp::instance()->mapCanvas()->mapSettings().hasCrsTransformEnabled() );
  myDa.setEllipsoid( QgsProject::instance()->readEntry( "Measure", "/Ellipsoid", GEO_NONE ) );
  mFieldExpressionWidget->setGeomCalculator( myDa );
}

void QgsLabelingGui::init()
{
  QgsPalLayerSettings lyr;
  lyr.readFromLayer( mLayer );

  // enable/disable main options based upon whether layer is being labeled
  chkEnableLabeling->setChecked( lyr.enabled );
  mFieldExpressionWidget->setEnabled( chkEnableLabeling->isChecked() );
  mLabelingFrame->setEnabled( chkEnableLabeling->isChecked() );

  // set the current field or add the current expression to the bottom of the list
  mFieldExpressionWidget->setField( lyr.fieldName );
}


QgsLabelingGui::~QgsLabelingGui()
{
}

void QgsLabelingGui::apply()
{
  writeSettingsToLayer();
  QgisApp::instance()->markDirty();
  // trigger refresh
  if ( mMapCanvas )
  {
    mMapCanvas->refresh();
  }
}

void QgsLabelingGui::writeSettingsToLayer()
{
  QgsPalLayerSettings settings = layerSettings();
  settings.writeToLayer( mLayer );
}

QgsPalLayerSettings QgsLabelingGui::layerSettings()
{
  QgsPalLayerSettings lyr;
  mSettingsWidget->saveToSettings( &lyr );

  lyr.enabled = chkEnableLabeling->isChecked();

  bool isExpression;
  lyr.fieldName = mFieldExpressionWidget->currentField( &isExpression );
  lyr.isExpression = isExpression;

  lyr.dist = 0;
  lyr.placementFlags = 0;

  return lyr;
}

void QgsLabelingGui::showEngineConfigDialog()
{
  QgsLabelEngineConfigDialog dlg( this );
  dlg.exec();
}
