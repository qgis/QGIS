/***************************************************************************
  qgsdiagramwidget.h
  Container widget for diagram layers
  -------------------
         begin                : September 2024
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
#include "diagram/qgsstackeddiagram.h"

#include "qgsdiagramwidget.h"
#include "moc_qgsdiagramwidget.cpp"
#include "qgsvectorlayer.h"
#include "qgsapplication.h"
#include "qgsguiutils.h"
#include "qgslabelengineconfigdialog.h"
#include "qgsdiagramproperties.h"
#include "qgsstackeddiagramproperties.h"


QgsDiagramWidget::QgsDiagramWidget( QgsVectorLayer *layer, QgsMapCanvas *canvas, QWidget *parent )
  : QgsMapLayerConfigWidget( layer, canvas, parent )
  , mLayer( layer )
  , mCanvas( canvas )
{
  if ( !layer )
  {
    return;
  }

  setupUi( this );

  // Initialize stacked diagram controls
  mDiagramTypeComboBox->addItem( QgsApplication::getThemeIcon( QStringLiteral( "diagramNone.svg" ) ), tr( "No Diagrams" ), ModeNone );
  mDiagramTypeComboBox->addItem( QgsApplication::getThemeIcon( QStringLiteral( "pie-chart.svg" ) ), tr( "Pie Chart" ), ModePie );
  mDiagramTypeComboBox->addItem( QgsApplication::getThemeIcon( QStringLiteral( "text.svg" ) ), tr( "Text Diagram" ), ModeText );
  mDiagramTypeComboBox->addItem( QgsApplication::getThemeIcon( QStringLiteral( "histogram.svg" ) ), tr( "Histogram" ), ModeHistogram );
  mDiagramTypeComboBox->addItem( QgsApplication::getThemeIcon( QStringLiteral( "stacked-bar.svg" ) ), tr( "Stacked Bars" ), ModeStackedBar );
  mDiagramTypeComboBox->addItem( QgsApplication::getThemeIcon( QStringLiteral( "stacked-diagram.svg" ) ), tr( "Stacked Diagram" ), ModeStacked );

  connect( mEngineSettingsButton, &QAbstractButton::clicked, this, &QgsDiagramWidget::showEngineConfigDialog );

  connect( mDiagramTypeComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsDiagramWidget::mDiagramTypeComboBox_currentIndexChanged );

  const int iconSize16 = QgsGuiUtils::scaleIconSize( 16 );
  mEngineSettingsButton->setIconSize( QSize( iconSize16, iconSize16 ) );
}

void QgsDiagramWidget::apply()
{
  const Mode mode = static_cast<Mode>( mDiagramTypeComboBox->currentData().toInt() );

  switch ( mode )
  {
    case ModeStacked:
    {
      // Delegate to stacked diagram's apply
      static_cast<QgsStackedDiagramProperties *>( mWidget )->apply();
      break;
    }
    case ModePie:
    case ModeText:
    case ModeHistogram:
    case ModeStackedBar:
    {
      // Delegate to single diagram's apply
      static_cast<QgsDiagramProperties *>( mWidget )->apply();
      break;
    }
    case ModeNone:
    {
      mLayer->setDiagramRenderer( nullptr );

      QgsDiagramLayerSettings dls;
      mLayer->setDiagramLayerSettings( dls );

      // refresh
      QgsProject::instance()->setDirty( true );
      mLayer->triggerRepaint();
    }
  }
}

void QgsDiagramWidget::syncToOwnLayer()
{
  if ( !mLayer )
  {
    return;
  }

  whileBlocking( mDiagramTypeComboBox )->setCurrentIndex( -1 );

  const QgsDiagramRenderer *dr = mLayer->diagramRenderer();

  // pick the right mode from the layer
  if ( dr && dr->diagram() )
  {
    if ( dr->rendererName() == QgsStackedDiagramRenderer::DIAGRAM_RENDERER_NAME_STACKED )
    {
      mDiagramTypeComboBox->setCurrentIndex( ModeStacked );
    }
    else // Single diagram
    {
      const QString diagramName = dr->diagram()->diagramName();
      if ( diagramName == QgsPieDiagram::DIAGRAM_NAME_PIE )
      {
        mDiagramTypeComboBox->setCurrentIndex( ModePie );
      }
      else if ( diagramName == QgsTextDiagram::DIAGRAM_NAME_TEXT )
      {
        mDiagramTypeComboBox->setCurrentIndex( ModeText );
      }
      else if ( diagramName == QgsStackedBarDiagram::DIAGRAM_NAME_STACKED_BAR )
      {
        mDiagramTypeComboBox->setCurrentIndex( ModeStackedBar );
      }
      else // diagramName == QgsHistogramDiagram::DIAGRAM_NAME_HISTOGRAM
      {
        // Play safe and set to histogram by default if the diagram name is unknown
        mDiagramTypeComboBox->setCurrentIndex( ModeHistogram );
      }

      // Delegate to single diagram's syncToLayer
      static_cast<QgsDiagramProperties *>( mWidget )->syncToLayer();
    }
  }
  else // No Diagram
  {
    mDiagramTypeComboBox->setCurrentIndex( ModeNone );
  }
}

void QgsDiagramWidget::mDiagramTypeComboBox_currentIndexChanged( int index )
{
  if ( mWidget )
    mStackedWidget->removeWidget( mWidget );

  delete mWidget;
  mWidget = nullptr;

  if ( index < 0 )
    return;

  const Mode mode = static_cast<Mode>( mDiagramTypeComboBox->currentData().toInt() );

  switch ( mode )
  {
    case ModePie:
    case ModeText:
    case ModeHistogram:
    case ModeStackedBar:
    {
      QgsDiagramProperties *singleWidget = new QgsDiagramProperties( mLayer, this, mMapCanvas );
      singleWidget->layout()->setContentsMargins( 0, 0, 0, 0 );
      singleWidget->setDockMode( dockMode() );
      singleWidget->syncToLayer();

      if ( mode == ModePie )
        singleWidget->setDiagramType( QgsPieDiagram::DIAGRAM_NAME_PIE );
      else if ( mode == ModeText )
        singleWidget->setDiagramType( QgsTextDiagram::DIAGRAM_NAME_TEXT );
      else if ( mode == ModeHistogram )
        singleWidget->setDiagramType( QgsHistogramDiagram::DIAGRAM_NAME_HISTOGRAM );
      else if ( mode == ModeStackedBar )
        singleWidget->setDiagramType( QgsStackedBarDiagram::DIAGRAM_NAME_STACKED_BAR );

      connect( singleWidget, &QgsPanelWidget::showPanel, this, &QgsPanelWidget::openPanel );
      connect( singleWidget, &QgsDiagramProperties::widgetChanged, this, &QgsDiagramWidget::widgetChanged );
      connect( singleWidget, &QgsDiagramProperties::auxiliaryFieldCreated, this, &QgsDiagramWidget::auxiliaryFieldCreated );

      mWidget = singleWidget;
      mStackedWidget->addWidget( mWidget );
      mStackedWidget->setCurrentWidget( mWidget );
      break;
    }
    case ModeStacked:
    {
      QgsStackedDiagramProperties *stackedWidget = new QgsStackedDiagramProperties( mLayer, this, mCanvas );
      stackedWidget->setDockMode( dockMode() );
      connect( stackedWidget, &QgsPanelWidget::showPanel, this, &QgsPanelWidget::openPanel );
      connect( stackedWidget, &QgsStackedDiagramProperties::widgetChanged, this, &QgsDiagramWidget::widgetChanged );

      mWidget = stackedWidget;
      mStackedWidget->addWidget( mWidget );
      mStackedWidget->setCurrentWidget( mWidget );
      break;
    }
    case ModeNone:
      break;
  }
  emit widgetChanged();
}

void QgsDiagramWidget::showEngineConfigDialog()
{
  QgsPanelWidget *panel = QgsPanelWidget::findParentPanel( this );
  if ( panel && panel->dockMode() )
  {
    QgsLabelEngineConfigWidget *widget = new QgsLabelEngineConfigWidget( mCanvas );
    connect( widget, &QgsLabelEngineConfigWidget::widgetChanged, widget, &QgsLabelEngineConfigWidget::apply );
    panel->openPanel( widget );
  }
  else
  {
    QgsLabelEngineConfigDialog dialog( mCanvas, this );
    dialog.exec();
    // reactivate button's window
    activateWindow();
  }
}
