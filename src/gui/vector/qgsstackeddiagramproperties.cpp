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
  connect( mAddSubDiagramButton, &QPushButton::clicked, this, &QgsStackedDiagramProperties::addSubDiagram );
  connect( mRemoveSubDiagramButton, &QPushButton::clicked, this, &QgsStackedDiagramProperties::removeSubDiagram );

  // Initialize stacked diagram controls
  mDiagramTypeComboBox->addItem( tr( "Single diagram" ), QgsDiagramLayerSettings::Single );
  mDiagramTypeComboBox->addItem( tr( "Stacked diagrams" ), QgsDiagramLayerSettings::Stacked );
  mStackedDiagramModeComboBox->addItem( tr( "Horizontal" ), QgsDiagramSettings::Horizontal );
  mStackedDiagramModeComboBox->addItem( tr( "Vertical" ), QgsDiagramSettings::Vertical );
  mRemoveSubDiagramButton->setEnabled( false );

  mStackedDiagramSpacingSpinBox->setClearValue( 0 );
  mStackedDiagramSpacingUnitComboBox->setUnits( { Qgis::RenderUnit::Millimeters,
      Qgis::RenderUnit::MetersInMapUnits,
      Qgis::RenderUnit::MapUnits,
      Qgis::RenderUnit::Pixels,
      Qgis::RenderUnit::Points,
      Qgis::RenderUnit::Inches } );

  // Add default subdiagram tab
  addSubDiagram();
}

void QgsStackedDiagramProperties::addSubDiagram()
{
  if ( mSubDiagramsTabWidget->count() == 0 )
  {
    defaultDiagram = new QgsDiagramProperties( mLayer, this, mMapCanvas );
    defaultDiagram->layout()->setContentsMargins( 6, 6, 6, 6 );
    connect( defaultDiagram, &QgsDiagramProperties::auxiliaryFieldCreated, this, &QgsStackedDiagramProperties::auxiliaryFieldCreated );

    mSubDiagramsTabWidget->addTab( defaultDiagram, tr( "Diagram 1" ) );
  }
  else
  {
    QgsDiagramProperties *gui = new QgsDiagramProperties( mLayer, this, mMapCanvas );
    gui->layout()->setContentsMargins( 6, 6, 6, 6 );
    connect( gui, &QgsDiagramProperties::auxiliaryFieldCreated, this, &QgsStackedDiagramProperties::auxiliaryFieldCreated );

    mSubDiagramsTabWidget->addTab( gui, tr( "Diagram %1" ).arg( mSubDiagramsTabWidget->count() + 1 ) );
  }

  mRemoveSubDiagramButton->setEnabled( mSubDiagramsTabWidget->count() > 2 );
}

void QgsStackedDiagramProperties::removeSubDiagram()
{
  if ( mSubDiagramsTabWidget->count() > 2 )
  {
    const int index = mSubDiagramsTabWidget->currentIndex();
    delete mSubDiagramsTabWidget->widget( index );

    mRemoveSubDiagramButton->setEnabled( mSubDiagramsTabWidget->count() > 2 );

    for ( int i = index; i < mSubDiagramsTabWidget->count(); i++ )
    {
      mSubDiagramsTabWidget->setTabText( i, tr( "Diagram %1" ).arg( i + 1 ) );
    }
  }
}

void QgsStackedDiagramProperties::apply()
{
  if ( mDiagramTypeComboBox->currentData( Qt::UserRole ) == QgsDiagramLayerSettings::Single )
  {
    defaultDiagram->apply();
  }
  else
  {
    // Get DiagramSettings from each subdiagram
    // Create DiagramSetings for the StackedDiagram
    // Add subdiagrams with their DiagramSettings to StackedDiagram
    // Create DiagramRenderer using info from first diagram and setting Stacked Diagram and the stacked diagram's DiagramSettings
    // Create DiagramLayerSettings from first diagram
  }
}

void QgsStackedDiagramProperties::syncToLayer()
{
  if ( mDiagramTypeComboBox->currentData( Qt::UserRole ) == QgsDiagramLayerSettings::Single )
  {
    defaultDiagram->syncToLayer();
  }
}

void QgsStackedDiagramProperties::mDiagramTypeComboBox_currentIndexChanged( int index )
{
  if ( index == 0 )
  {
    mStackedDiagramSettingsFrame->hide();

    // Hide tabs other than the first one
    for ( int i = 0; i < mSubDiagramsTabWidget->count(); i++ )
    {
      if ( i < 1 )
        continue;

      mSubDiagramsTabWidget->setTabVisible( i, false );
    }
  }
  else
  {
    mStackedDiagramSettingsFrame->show();

    // TODO: Create the second tab or show all hidden tabs
    if ( mSubDiagramsTabWidget->count() == 1 )
    {
      // Add second subdiagram tab
      addSubDiagram();
    }
    else
    {
      for ( int i = 0; i < mSubDiagramsTabWidget->count(); i++ )
      {
        if ( i < 1 )
          continue;

        mSubDiagramsTabWidget->setTabVisible( i, true );
      }
    }
  }
}
