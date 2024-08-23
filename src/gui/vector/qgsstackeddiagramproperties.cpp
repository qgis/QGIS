/***************************************************************************
  qgsstackeddiagramproperties.h
  Properties for stacked diagram layers
  -------------------
         begin                : August 2024
         copyright            : (C) Germán Carrillo
         email                : german at opengis dot ch

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsstackeddiagramproperties.h"
#include "qgsdiagramproperties.h"


QgsStackedDiagramProperties::QgsStackedDiagramProperties( QgsVectorLayer *layer, QWidget *parent, QgsMapCanvas *canvas )
  : QWidget{parent}
  , mMapCanvas( canvas )
{
  mLayer = layer;
  if ( !layer )
  {
    return;
  }

  setupUi( this );
  connect( mDiagramTypeComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsStackedDiagramProperties::mDiagramTypeComboBox_currentIndexChanged );

  // Initialize stacked diagram controls
  mDiagramTypeComboBox->addItem( tr( "Single diagram" ), QgsDiagramLayerSettings::Single );
  mDiagramTypeComboBox->addItem( tr( "Stacked diagrams" ), QgsDiagramLayerSettings::Stacked );
  mStackedDiagramModeComboBox->addItem( tr( "Horizontal" ), QgsDiagramSettings::Horizontal );
  mStackedDiagramModeComboBox->addItem( tr( "Vertical" ), QgsDiagramSettings::Vertical );

  mStackedDiagramSpacingSpinBox->setClearValue( 0 );
  mStackedDiagramSpacingUnitComboBox->setUnits( { Qgis::RenderUnit::Millimeters,
      Qgis::RenderUnit::MetersInMapUnits,
      Qgis::RenderUnit::MapUnits,
      Qgis::RenderUnit::Pixels,
      Qgis::RenderUnit::Points,
      Qgis::RenderUnit::Inches } );

  // Add default subdiagram tab
  gui = new QgsDiagramProperties( layer, this, mMapCanvas );
  gui->layout()->setContentsMargins( 0, 0, 0, 0 );
  QVBoxLayout *vLayout = new QVBoxLayout();
  vLayout->addWidget( gui );
  QWidget *w = new QWidget();
  w->setLayout( vLayout );

  connect( gui, &QgsDiagramProperties::auxiliaryFieldCreated, this, &QgsStackedDiagramProperties::auxiliaryFieldCreated );

  mSubDiagramsTabWidget->addTab( w, tr( "Diagram 1" ) );
}

void QgsStackedDiagramProperties::apply()
{
  if ( mDiagramTypeComboBox->currentData( Qt::UserRole ) == QgsDiagramLayerSettings::Single )
  {
    gui->apply();
  }
}

void QgsStackedDiagramProperties::syncToLayer()
{
  if ( mDiagramTypeComboBox->currentData( Qt::UserRole ) == QgsDiagramLayerSettings::Single )
  {
    gui->syncToLayer();
  }
}

void QgsStackedDiagramProperties::mDiagramTypeComboBox_currentIndexChanged( int index )
{
  if ( index == 0 )
  {
    mStackedDiagramSettingsFrame->hide();
  }
  else
  {
    mStackedDiagramSettingsFrame->show();
  }
}
