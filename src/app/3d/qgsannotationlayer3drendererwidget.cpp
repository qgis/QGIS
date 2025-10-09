/***************************************************************************
  qgsannotationlayer3drendererwidget.cpp
  ------------------------------
  Date                 : September 2025
  Copyright            : (C) 2025 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsannotationlayer3drendererwidget.h"
#include "moc_qgsannotationlayer3drendererwidget.cpp"

#include "qgsapplication.h"
#include "qgsannotationlayer.h"
#include "qgsannotationlayer3drenderer.h"

#include <QBoxLayout>
#include <QCheckBox>

QgsAnnotationLayer3DRendererWidget::QgsAnnotationLayer3DRendererWidget( QgsAnnotationLayer *layer, QgsMapCanvas *canvas, QWidget *parent )
  : QgsMapLayerConfigWidget( layer, canvas, parent )
{
  setPanelTitle( tr( "3D View" ) );
  setObjectName( QStringLiteral( "mOptsPage_3DView" ) );

  setupUi( this );

  mFontButton->setMode( QgsFontButton::ModeTextRenderer );

  mComboRendererType->addItem( QgsApplication::getThemeIcon( QStringLiteral( "rendererNullSymbol.svg" ) ), tr( "No Symbols" ), QVariant::fromValue( RendererType::None ) );
  mComboRendererType->addItem( QgsApplication::getThemeIcon( QStringLiteral( "rendererSingleSymbol.svg" ) ), tr( "3D Billboards" ), QVariant::fromValue( RendererType::Billboards ) );
  mComboRendererType->setCurrentIndex( mComboRendererType->findData( QVariant::fromValue( RendererType::None ) ) );
  mStackedWidget->setCurrentWidget( mPageNoRenderer );

  connect( mComboRendererType, qOverload< int >( &QComboBox::currentIndexChanged ), this, &QgsAnnotationLayer3DRendererWidget::rendererTypeChanged );

  mComboClamping->addItem( tr( "Relative to Terrain" ), QVariant::fromValue( Qgis::AltitudeClamping::Relative ) );
  mComboClamping->addItem( tr( "Absolute" ), QVariant::fromValue( Qgis::AltitudeClamping::Absolute ) );

  connect( mComboClamping, qOverload<int>( &QComboBox::currentIndexChanged ), this, &QgsAnnotationLayer3DRendererWidget::clampingChanged );
  mComboClamping->setCurrentIndex( mComboClamping->findData( QVariant::fromValue( Qgis::AltitudeClamping::Relative ) ) );
  clampingChanged();

  connect( mOffsetZSpinBox, qOverload< double >( &QDoubleSpinBox::valueChanged ), this, [this]( double ) {
    if ( !mBlockChanges )
      emit widgetChanged();
  } );

  connect( mCheckShowCallouts, &QCheckBox::toggled, this, [this] {
    if ( !mBlockChanges )
      emit widgetChanged();
  } );

  connect( mFontButton, &QgsFontButton::changed, this, [this] {
    if ( !mBlockChanges )
      emit widgetChanged();
  } );

  syncToLayer( layer );
}

void QgsAnnotationLayer3DRendererWidget::setRenderer( const QgsAnnotationLayer3DRenderer *renderer )
{
  mBlockChanges++;
  mRenderer.reset( renderer ? renderer->clone() : nullptr );
  if ( renderer )
  {
    mComboRendererType->setCurrentIndex( mComboRendererType->findData( QVariant::fromValue( RendererType::Billboards ) ) );
    mComboClamping->setCurrentIndex( mComboClamping->findData( QVariant::fromValue( renderer->altitudeClamping() ) ) );
    mOffsetZSpinBox->setValue( renderer->zOffset() );
    mCheckShowCallouts->setChecked( renderer->showCalloutLines() );
    mFontButton->setTextFormat( renderer->textFormat() );
  }
  else
  {
    mComboRendererType->setCurrentIndex( mComboRendererType->findData( QVariant::fromValue( RendererType::None ) ) );
  }
  mBlockChanges--;
}

std::unique_ptr< QgsAnnotationLayer3DRenderer > QgsAnnotationLayer3DRendererWidget::renderer()
{
  const RendererType type = mComboRendererType->currentData().value< RendererType >();
  switch ( type )
  {
    case RendererType::None:
      return nullptr;
      break;
    case RendererType::Billboards:
    {
      auto renderer = std::make_unique< QgsAnnotationLayer3DRenderer >();
      renderer->setAltitudeClamping( mComboClamping->currentData().value< Qgis::AltitudeClamping >() );
      renderer->setZOffset( mOffsetZSpinBox->value() );
      renderer->setShowCalloutLines( mCheckShowCallouts->isChecked() );
      renderer->setTextFormat( mFontButton->textFormat() );
      return renderer;
    }
  }
  BUILTIN_UNREACHABLE
}

void QgsAnnotationLayer3DRendererWidget::apply()
{
  std::unique_ptr< QgsAnnotationLayer3DRenderer > r = renderer();
  mLayer->setRenderer3D( r.release() );
}

void QgsAnnotationLayer3DRendererWidget::rendererTypeChanged()
{
  const RendererType type = mComboRendererType->currentData().value< RendererType >();
  switch ( type )
  {
    case RendererType::None:
      mStackedWidget->setCurrentWidget( mPageNoRenderer );
      break;
    case RendererType::Billboards:
      mStackedWidget->setCurrentWidget( mPageBillboards );
      break;
  }
  if ( !mBlockChanges )
    emit widgetChanged();
}

void QgsAnnotationLayer3DRendererWidget::clampingChanged()
{
  switch ( mComboClamping->currentData().value< Qgis::AltitudeClamping >() )
  {
    case Qgis::AltitudeClamping::Absolute:
      mLabelClampingExplanation->setText(
        QStringLiteral( "<p><b>%1</b></p><p>%2</p>" ).arg( tr( "All billboards will be placed at the same elevation." ), tr( "The terrain height will be ignored." ) )
      );

      break;
    case Qgis::AltitudeClamping::Relative:
      mLabelClampingExplanation->setText(
        QStringLiteral( "<p><b>%1</b></p>" ).arg( tr( "Billboard elevation is relative to terrain height." ) )
      );
      break;
    case Qgis::AltitudeClamping::Terrain:
      mLabelClampingExplanation->setText(
        QStringLiteral( "<p><b>%1</b></p><p>%2</p>" ).arg( tr( "Billboard elevation will be taken directly from the terrain height." ), tr( "Billboards will be placed directly on the terrain." ) )
      );
      break;
  }
  if ( !mBlockChanges )
    emit widgetChanged();
}

void QgsAnnotationLayer3DRendererWidget::syncToLayer( QgsMapLayer *layer )
{
  mLayer = layer;
  QgsAbstract3DRenderer *r = layer->renderer3D();
  if ( r && r->type() == QLatin1String( "annotation" ) )
  {
    QgsAnnotationLayer3DRenderer *annotationRenderer = qgis::down_cast<QgsAnnotationLayer3DRenderer *>( r );
    setRenderer( annotationRenderer );
  }
  else
  {
    setRenderer( nullptr );
  }
}

//
// QgsAnnotationLayer3DRendererWidgetFactory
//

QgsAnnotationLayer3DRendererWidgetFactory::QgsAnnotationLayer3DRendererWidgetFactory( QObject *parent )
  : QObject( parent )
{
  setIcon( QIcon( ":/images/themes/default/3d.svg" ) );
  setTitle( tr( "3D View" ) );
}

QgsMapLayerConfigWidget *QgsAnnotationLayer3DRendererWidgetFactory::createWidget( QgsMapLayer *layer, QgsMapCanvas *canvas, bool dockWidget, QWidget *parent ) const
{
  Q_UNUSED( dockWidget )
  QgsAnnotationLayer *annotationLayer = qobject_cast<QgsAnnotationLayer *>( layer );
  if ( !annotationLayer )
    return nullptr;
  return new QgsAnnotationLayer3DRendererWidget( annotationLayer, canvas, parent );
}

bool QgsAnnotationLayer3DRendererWidgetFactory::supportLayerPropertiesDialog() const
{
  return true;
}

bool QgsAnnotationLayer3DRendererWidgetFactory::supportsStyleDock() const
{
  return true;
}

bool QgsAnnotationLayer3DRendererWidgetFactory::supportsLayer( QgsMapLayer *layer ) const
{
  return layer->type() == Qgis::LayerType::Annotation;
}

QString QgsAnnotationLayer3DRendererWidgetFactory::layerPropertiesPagePositionHint() const
{
  return QStringLiteral( "mOptsPage_Rendering" );
}
