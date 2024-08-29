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

#include "diagram/qgshistogramdiagram.h"
#include "diagram/qgspiediagram.h"
#include "diagram/qgstextdiagram.h"
#include "diagram/qgsstackedbardiagram.h"

#include "qgsapplication.h"
#include "qgsdiagramproperties.h"
#include "qgslabelengineconfigdialog.h"
#include "qgsproject.h"
#include "qgsstackeddiagram.h"
#include "qgsstackeddiagramproperties.h"
#include "qgsvectorlayer.h"

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
  connect( mEngineSettingsButton, &QPushButton::clicked, this, &QgsStackedDiagramProperties::mEngineSettingsButton_clicked );
  connect( mAddSubDiagramButton, &QPushButton::clicked, this, &QgsStackedDiagramProperties::addSubDiagram );
  connect( mRemoveSubDiagramButton, &QPushButton::clicked, this, &QgsStackedDiagramProperties::removeSubDiagram );
  connect( mSubDiagramsTabWidget->tabBar(), &QTabBar::tabMoved, this, &QgsStackedDiagramProperties::mSubDiagramsTabWidget_tabMoved );

  // Initialize stacked diagram controls
  QIcon icon = QgsApplication::getThemeIcon( QStringLiteral( "diagramNone.svg" ) );
  mDiagramTypeComboBox->addItem( icon, tr( "No Diagrams" ), "None" );
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

  syncToLayer();
}

void QgsStackedDiagramProperties::addSubDiagram()
{
  QgsDiagramProperties *gui = new QgsDiagramProperties( mLayer, this, mMapCanvas );
  gui->layout()->setContentsMargins( 6, 6, 6, 6 );
  connect( gui, &QgsDiagramProperties::auxiliaryFieldCreated, this, &QgsStackedDiagramProperties::auxiliaryFieldCreated );

  mSubDiagramsTabWidget->addTab( gui, tr( "Diagram %1" ).arg( mSubDiagramsTabWidget->count() + 1 ) );

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

void QgsStackedDiagramProperties::syncToLayer()
{
  const QgsDiagramRenderer *dr = mLayer->diagramRenderer();

  if ( dr && dr->diagram() )
  {
    if ( dr->diagram()->diagramName() == DIAGRAM_NAME_STACKED )
    {
      mDiagramTypeComboBox->blockSignals( true );
      mDiagramTypeComboBox->setCurrentIndex( mDiagramTypeComboBox->findData( QgsDiagramLayerSettings::Stacked ) );
      mDiagramTypeComboBox->blockSignals( false );
      //force a refresh of widget status to match diagram type
      mDiagramTypeComboBox_currentIndexChanged( mDiagramTypeComboBox->currentIndex() );

      const QList<QgsDiagramSettings> settingList = dr->diagramSettings();
      mStackedDiagramModeComboBox->setCurrentIndex( settingList.at( 0 ).stackedDiagramMode );
      mStackedDiagramSpacingSpinBox->setValue( settingList.at( 0 ).stackedDiagramSpacing() );
      mStackedDiagramSpacingUnitComboBox->setUnit( settingList.at( 0 ).stackedDiagramSpacingUnit() );

      // Create as many tabs as necessary
      const QgsStackedDiagram *stackedDiagram = dynamic_cast< const QgsStackedDiagram *>( dr->diagram() );
      const int subDiagramCount = stackedDiagram->subDiagramCount();
      while ( mSubDiagramsTabWidget->count() < subDiagramCount )
      {
        addSubDiagram();
      }

      // Call subdiagrams' syncToLayer with the corresponding subdiagram index
      for ( int i = 0; i < mSubDiagramsTabWidget->count(); i++ )
      {
        QgsDiagramProperties *diagramProperties = static_cast<QgsDiagramProperties *>( mSubDiagramsTabWidget->widget( i ) );
        diagramProperties->mSubDiagramIndex = i;
        diagramProperties->syncToLayer();
      }
    }
    else
    {
      mDiagramTypeComboBox->blockSignals( true );
      mDiagramTypeComboBox->setCurrentIndex( mDiagramTypeComboBox->findData( QgsDiagramLayerSettings::Single ) );
      mDiagramTypeComboBox->blockSignals( false );
      //force a refresh of widget status to match diagram type
      mDiagramTypeComboBox_currentIndexChanged( mDiagramTypeComboBox->currentIndex() );

      // Delegate to single diagram's syncToLayer
      // If the diagram name is unknown, the single diagram will choose a default one (pie)
      static_cast<QgsDiagramProperties *>( mSubDiagramsTabWidget->widget( 0 ) )->syncToLayer();
    }
  }
  else
  {
    mDiagramTypeComboBox->blockSignals( true );
    mDiagramTypeComboBox->setCurrentIndex( 0 ); // No Diagram
    mDiagramTypeComboBox->blockSignals( false );
    //force a refresh of widget status to match diagram type
    mDiagramTypeComboBox_currentIndexChanged( mDiagramTypeComboBox->currentIndex() );

    // Delegate to first diagram's syncToLayer
    // It will add required reasonable defaults
    static_cast<QgsDiagramProperties *>( mSubDiagramsTabWidget->widget( 0 ) )->syncToLayer();
  }
}

void QgsStackedDiagramProperties::mSubDiagramsTabWidget_tabMoved( int from, int to )
{
  Q_UNUSED( from )
  Q_UNUSED( to )
  for ( int i = 0; i < mSubDiagramsTabWidget->count(); i++ )
  {
    mSubDiagramsTabWidget->setTabText( i, tr( "Diagram %1" ).arg( i + 1 ) );
  }
}

void QgsStackedDiagramProperties::apply()
{
  if ( mDiagramTypeComboBox->currentData( Qt::UserRole ) == "None" )
  {
    std::unique_ptr< QgsDiagramRenderer > renderer;
    mLayer->setDiagramRenderer( renderer.release() );

    QgsDiagramLayerSettings dls;
    mLayer->setDiagramLayerSettings( dls );

    // refresh
    QgsProject::instance()->setDirty( true );
    mLayer->triggerRepaint();
  }
  else if ( mDiagramTypeComboBox->currentData( Qt::UserRole ) == QgsDiagramLayerSettings::Single )
  {
    static_cast<QgsDiagramProperties *>( mSubDiagramsTabWidget->widget( 0 ) )->apply();
  }
  else // Stacked diagram
  {
    // TODO: Validate that we have at least 2 diagrams

    // Create DiagramSetings for the StackedDiagram
    std::unique_ptr< QgsDiagramSettings> ds = std::make_unique<QgsDiagramSettings>();
    ds->stackedDiagramMode = static_cast<QgsDiagramSettings::StackedDiagramMode>( mStackedDiagramModeComboBox->currentData().toInt() );
    ds->setStackedDiagramSpacingUnit( mStackedDiagramSpacingUnitComboBox->unit() );
    ds->setStackedDiagramSpacing( mStackedDiagramSpacingSpinBox->value() );

    // Add subdiagrams with their DiagramSettings to StackedDiagram
    std::unique_ptr< QgsStackedDiagram > stackedDiagram = std::make_unique< QgsStackedDiagram >();

    // Get DiagramSettings from each subdiagram
    for ( int i = 0; i < mSubDiagramsTabWidget->count(); i++ )
    {
      QgsDiagramProperties *diagramProperties = static_cast<QgsDiagramProperties *>( mSubDiagramsTabWidget->widget( i ) );
      std::unique_ptr< QgsDiagramSettings > ds1 = diagramProperties->createDiagramSettings();
      ds->categoryAttributes += ds1->categoryAttributes;
      ds->categoryLabels += ds1->categoryLabels;
      ds->categoryColors += ds1->categoryColors;

      std::unique_ptr< QgsDiagram > diagram;

      if ( diagramProperties->mDiagramType == DIAGRAM_NAME_TEXT )
      {
        diagram = std::make_unique< QgsTextDiagram >();
      }
      else if ( diagramProperties->mDiagramType == DIAGRAM_NAME_PIE )
      {
        diagram = std::make_unique< QgsPieDiagram >();
      }
      else if ( diagramProperties->mDiagramType == DIAGRAM_NAME_STACKED_BAR )
      {
        diagram = std::make_unique< QgsStackedBarDiagram >();
      }
      else // DIAGRAM_NAME_HISTOGRAM
      {
        diagram = std::make_unique< QgsHistogramDiagram >();
      }
      stackedDiagram->addSubDiagram( diagram.release(), ds1.release() );
    }

    // Get first diagram to configure some stacked diagram settings from it
    QgsDiagramProperties *firstDiagramProperties = static_cast<QgsDiagramProperties *>( mSubDiagramsTabWidget->widget( 0 ) );

    // Create DiagramRenderer using info from first diagram and setting Stacked Diagram and the stacked diagram's DiagramSettings
    std::unique_ptr< QgsDiagramRenderer > renderer = firstDiagramProperties->createRendererBaseInfo( *ds );
    renderer->setDiagram( stackedDiagram.release() );
    mLayer->setDiagramRenderer( renderer.release() );

    // Create DiagramLayerSettings from first diagram
    QgsDiagramLayerSettings dls = firstDiagramProperties->createDiagramLayerSettings();
    mLayer->setDiagramLayerSettings( dls );

    // refresh
    QgsProject::instance()->setDirty( true );
    mLayer->triggerRepaint();
  }
}

void QgsStackedDiagramProperties::mDiagramTypeComboBox_currentIndexChanged( int index )
{
  if ( index == 0 ) // No diagram
  {
    mDiagramsFrame->setEnabled( false );
    mStackedDiagramSettingsFrame->hide();

    // Hide tabs other than the first one
    for ( int i = 0; i < mSubDiagramsTabWidget->count(); i++ )
    {
      if ( i < 1 )
        continue;

      mSubDiagramsTabWidget->setTabVisible( i, false );
    }
  }
  else if ( index == 1 ) // Single
  {
    mDiagramsFrame->setEnabled( true );
    mStackedDiagramSettingsFrame->hide();

    // Hide tabs other than the first one
    for ( int i = 0; i < mSubDiagramsTabWidget->count(); i++ )
    {
      if ( i < 1 )
        continue;

      mSubDiagramsTabWidget->setTabVisible( i, false );
    }
  }
  else // Stacked
  {
    mDiagramsFrame->setEnabled( true );
    mStackedDiagramSettingsFrame->show();

    // Create the second tab or show all hidden tabs
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

void QgsStackedDiagramProperties::mEngineSettingsButton_clicked()
{
  QgsPanelWidget *panel = QgsPanelWidget::findParentPanel( this );
  if ( panel && panel->dockMode() )
  {
    QgsLabelEngineConfigWidget *widget = new QgsLabelEngineConfigWidget( mMapCanvas );
    connect( widget, &QgsLabelEngineConfigWidget::widgetChanged, widget, &QgsLabelEngineConfigWidget::apply );
    panel->openPanel( widget );
  }
  else
  {
    QgsLabelEngineConfigDialog dialog( mMapCanvas, this );
    dialog.exec();
    // reactivate button's window
    activateWindow();
  }
}
