/***************************************************************************
                              qgspointdisplacementrendererwidget.cpp
                              --------------------------------------
  begin                : January 26, 2010
  copyright            : (C) 2010 by Marco Hugentobler
  email                : marco at hugis dot net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgspointdisplacementrendererwidget.h"
#include "qgspointdisplacementrenderer.h"
#include "qgsrendererregistry.h"
#include "qgsfield.h"
#include "qgsstyle.h"
#include "qgssymbolselectordialog.h"
#include "qgssymbollayerutils.h"
#include "qgsvectorlayer.h"
#include "qgisgui.h"

QgsRendererWidget* QgsPointDisplacementRendererWidget::create( QgsVectorLayer* layer, QgsStyle* style, QgsFeatureRenderer* renderer )
{
  return new QgsPointDisplacementRendererWidget( layer, style, renderer );
}

QgsPointDisplacementRendererWidget::QgsPointDisplacementRendererWidget( QgsVectorLayer* layer, QgsStyle* style, QgsFeatureRenderer* renderer )
    : QgsRendererWidget( layer, style )
    , mRenderer( nullptr )
{
  if ( !layer )
  {
    return;
  }

  //the renderer only applies to point vector layers
  if ( layer->wkbType() != QgsWkbTypes::Point && layer->wkbType()  != QgsWkbTypes::Point25D )
  {
    //setup blank dialog
    mRenderer = nullptr;
    setupBlankUi( layer->name() );
    return;
  }
  setupUi( this );

  mDistanceUnitWidget->setUnits( QgsUnitTypes::RenderUnitList() << QgsUnitTypes::RenderMillimeters << QgsUnitTypes::RenderMapUnits << QgsUnitTypes::RenderPixels );

  if ( renderer )
  {
    mRenderer = QgsPointDisplacementRenderer::convertFromRenderer( renderer );
  }
  if ( !mRenderer )
  {
    mRenderer = new QgsPointDisplacementRenderer();
  }

  blockAllSignals( true );

  mPlacementComboBox->addItem( tr( "Ring" ), QgsPointDisplacementRenderer::Ring );
  mPlacementComboBox->addItem( tr( "Concentric rings" ), QgsPointDisplacementRenderer::ConcentricRings );

  //insert attributes into combo box
  if ( layer )
  {
    Q_FOREACH ( const QgsField& f, layer->fields() )
    {
      mLabelFieldComboBox->addItem( f.name() );
    }
    mLabelFieldComboBox->addItem( tr( "None" ) );

    QString currentLabelAttribute = mRenderer->labelAttributeName();
    if ( !currentLabelAttribute.isEmpty() )
    {
      mLabelFieldComboBox->setCurrentIndex( mLabelFieldComboBox->findText( currentLabelAttribute ) );
    }
    else
    {
      mLabelFieldComboBox->setCurrentIndex( mLabelFieldComboBox->findText( tr( "None" ) ) );
    }
  }

  //insert possible renderer types
  QStringList rendererList = QgsRendererRegistry::instance()->renderersList( QgsRendererAbstractMetadata::PointLayer );
  QStringList::const_iterator it = rendererList.constBegin();
  for ( ; it != rendererList.constEnd(); ++it )
  {
    if ( *it != "pointDisplacement" )
    {
      QgsRendererAbstractMetadata* m = QgsRendererRegistry::instance()->rendererMetadata( *it );
      mRendererComboBox->addItem( m->icon(), m->visibleName(), *it );
    }
  }

  mCircleColorButton->setColorDialogTitle( tr( "Select color" ) );
  mCircleColorButton->setContext( "symbology" );
  mCircleColorButton->setAllowAlpha( true );
  mCircleColorButton->setShowNoColor( true );
  mCircleColorButton->setNoColorString( tr( "No outline" ) );
  mLabelColorButton->setContext( "symbology" );
  mLabelColorButton->setColorDialogTitle( tr( "Select color" ) );
  mLabelColorButton->setAllowAlpha( true );

  mCircleWidthSpinBox->setValue( mRenderer->circleWidth() );
  mCircleColorButton->setColor( mRenderer->circleColor() );
  mLabelColorButton->setColor( mRenderer->labelColor() );
  mCircleModificationSpinBox->setClearValue( 0.0 );
  mCircleModificationSpinBox->setValue( mRenderer->circleRadiusAddition() );
  mDistanceSpinBox->setValue( mRenderer->tolerance() );
  mDistanceUnitWidget->setUnit( mRenderer->toleranceUnit() );
  mDistanceUnitWidget->setMapUnitScale( mRenderer->toleranceMapUnitScale() );

  mPlacementComboBox->setCurrentIndex( mPlacementComboBox->findData( mRenderer->placement() ) );

  //scale dependent labelling
  mMaxScaleDenominatorEdit->setText( QString::number( mRenderer->maxLabelScaleDenominator() ) );
  mMaxScaleDenominatorEdit->setValidator( new QDoubleValidator( mMaxScaleDenominatorEdit ) );
  if ( mRenderer->maxLabelScaleDenominator() > 0 )
  {
    mScaleDependentLabelsCheckBox->setCheckState( Qt::Checked );
  }
  else
  {
    mScaleDependentLabelsCheckBox->setCheckState( Qt::Unchecked );
    mMaxScaleDenominatorEdit->setEnabled( false );
  }


  blockAllSignals( false );

  //set the appropriate renderer dialog
  if ( mRenderer->embeddedRenderer() )
  {
    QString rendererName = mRenderer->embeddedRenderer()->type();
    int rendererIndex = mRendererComboBox->findData( rendererName );
    if ( rendererIndex != -1 )
    {
      mRendererComboBox->setCurrentIndex( rendererIndex );
      on_mRendererComboBox_currentIndexChanged( rendererIndex );
    }
  }

  updateCenterIcon();
}

QgsPointDisplacementRendererWidget::~QgsPointDisplacementRendererWidget()
{
  delete mRenderer;
}

QgsFeatureRenderer* QgsPointDisplacementRendererWidget::renderer()
{
  return mRenderer;
}

void QgsPointDisplacementRendererWidget::setMapCanvas( QgsMapCanvas* canvas )
{
  QgsRendererWidget::setMapCanvas( canvas );
  if ( mDistanceUnitWidget )
    mDistanceUnitWidget->setMapCanvas( canvas );
}

void QgsPointDisplacementRendererWidget::on_mLabelFieldComboBox_currentIndexChanged( const QString& text )
{
  if ( mRenderer )
  {
    if ( text == tr( "None" ) )
    {
      mRenderer->setLabelAttributeName( "" );
    }
    else
    {
      mRenderer->setLabelAttributeName( text );
    }
    emit widgetChanged();
  }
}

void QgsPointDisplacementRendererWidget::on_mRendererComboBox_currentIndexChanged( int index )
{
  QString rendererId = mRendererComboBox->itemData( index ).toString();
  QgsRendererAbstractMetadata* m = QgsRendererRegistry::instance()->rendererMetadata( rendererId );
  if ( m )
  {
    // unfortunately renderer conversion is only available through the creation of a widget...
    QgsRendererWidget* tempRenderWidget = m->createRendererWidget( mLayer, mStyle, mRenderer->embeddedRenderer()->clone() );
    mRenderer->setEmbeddedRenderer( tempRenderWidget->renderer()->clone() );
    delete tempRenderWidget;
    emit widgetChanged();
  }
}

void QgsPointDisplacementRendererWidget::on_mPlacementComboBox_currentIndexChanged( int index )
{
  if ( !mRenderer )
    return;

  mRenderer->setPlacement(( QgsPointDisplacementRenderer::Placement )mPlacementComboBox->itemData( index ).toInt() );
  emit widgetChanged();
}

void QgsPointDisplacementRendererWidget::on_mRendererSettingsButton_clicked()
{
  if ( !mRenderer )
    return;

  QgsRendererAbstractMetadata* m = QgsRendererRegistry::instance()->rendererMetadata( mRenderer->embeddedRenderer()->type() );
  if ( m )
  {
    QgsRendererWidget* w = m->createRendererWidget( mLayer, mStyle, mRenderer->embeddedRenderer()->clone() );
    w->setMapCanvas( mMapCanvas );
    connect( w, SIGNAL( widgetChanged() ), this, SLOT( updateRendererFromWidget() ) );
    w->setDockMode( this->dockMode() );
    openPanel( w );
  }
}

void QgsPointDisplacementRendererWidget::on_mLabelFontButton_clicked()
{
  if ( !mRenderer )
  {
    return;
  }

  bool ok;
  QFont newFont = QgisGui::getFont( ok, mRenderer->labelFont(), tr( "Label Font" ) );
  if ( ok )
  {
    mRenderer->setLabelFont( newFont );
    emit widgetChanged();
  }
}

void QgsPointDisplacementRendererWidget::on_mCircleWidthSpinBox_valueChanged( double d )
{
  if ( mRenderer )
  {
    mRenderer->setCircleWidth( d );
    emit widgetChanged();
  }
}

void QgsPointDisplacementRendererWidget::on_mCircleColorButton_colorChanged( const QColor& newColor )
{
  if ( !mRenderer )
  {
    return;
  }

  mRenderer->setCircleColor( newColor );
  emit widgetChanged();
}

void QgsPointDisplacementRendererWidget::on_mLabelColorButton_colorChanged( const QColor& newColor )
{
  if ( !mRenderer )
  {
    return;
  }

  mRenderer->setLabelColor( newColor );
  emit widgetChanged();
}

void QgsPointDisplacementRendererWidget::on_mCircleModificationSpinBox_valueChanged( double d )
{
  if ( !mRenderer )
  {
    return;
  }

  mRenderer->setCircleRadiusAddition( d );
  emit widgetChanged();
}

void QgsPointDisplacementRendererWidget::on_mDistanceSpinBox_valueChanged( double d )
{
  if ( mRenderer )
  {
    mRenderer->setTolerance( d );
    emit widgetChanged();
  }
}

void QgsPointDisplacementRendererWidget::on_mDistanceUnitWidget_changed()
{
  if ( mRenderer )
  {
    mRenderer->setToleranceUnit( mDistanceUnitWidget->unit() );
    mRenderer->setToleranceMapUnitScale( mDistanceUnitWidget->getMapUnitScale() );
    emit widgetChanged();
  }
}

void QgsPointDisplacementRendererWidget::on_mScaleDependentLabelsCheckBox_stateChanged( int state )
{
  if ( state == Qt::Unchecked )
  {
    mMaxScaleDenominatorEdit->setText( "-1" );
    mMaxScaleDenominatorEdit->setEnabled( false );
  }
  else
  {
    mMaxScaleDenominatorEdit->setEnabled( true );
  }
}

void QgsPointDisplacementRendererWidget::on_mMaxScaleDenominatorEdit_textChanged( const QString & text )
{
  if ( !mRenderer )
  {
    return;
  }

  bool ok;
  double scaleDenominator = text.toDouble( &ok );
  if ( ok )
  {
    mRenderer->setMaxLabelScaleDenominator( scaleDenominator );
    emit widgetChanged();
  }
}

void QgsPointDisplacementRendererWidget::blockAllSignals( bool block )
{
  mLabelFieldComboBox->blockSignals( block );
  mLabelFontButton->blockSignals( block );
  mCircleWidthSpinBox->blockSignals( block );
  mCircleColorButton->blockSignals( block );
  mRendererComboBox->blockSignals( block );
  mLabelColorButton->blockSignals( block );
  mCircleModificationSpinBox->blockSignals( block );
  mScaleDependentLabelsCheckBox->blockSignals( block );
  mMaxScaleDenominatorEdit->blockSignals( block );
  mCenterSymbolPushButton->blockSignals( block );
  mDistanceSpinBox->blockSignals( block );
  mDistanceUnitWidget->blockSignals( block );
  mPlacementComboBox->blockSignals( block );
}

void QgsPointDisplacementRendererWidget::on_mCenterSymbolPushButton_clicked()
{
  if ( !mRenderer || !mRenderer->centerSymbol() )
  {
    return;
  }
  QgsMarkerSymbol* markerSymbol = mRenderer->centerSymbol()->clone();
  QgsSymbolSelectorWidget* dlg = new QgsSymbolSelectorWidget( markerSymbol, QgsStyle::defaultStyle(), mLayer, this );
  dlg->setDockMode( this->dockMode() );
  dlg->setMapCanvas( mMapCanvas );
  connect( dlg, SIGNAL( widgetChanged() ), this, SLOT( updateCenterSymbolFromWidget() ) );
  connect( dlg, SIGNAL( panelAccepted( QgsPanelWidget* ) ), this, SLOT( cleanUpSymbolSelector( QgsPanelWidget* ) ) );
  openPanel( dlg );
}

void QgsPointDisplacementRendererWidget::updateCenterSymbolFromWidget()
{
  QgsSymbolSelectorWidget* dlg = qobject_cast<QgsSymbolSelectorWidget*>( sender() );
  QgsSymbol* symbol = dlg->symbol()->clone();
  mRenderer->setCenterSymbol( static_cast< QgsMarkerSymbol* >( symbol ) );
  updateCenterIcon();
  emit widgetChanged();
}

void QgsPointDisplacementRendererWidget::cleanUpSymbolSelector( QgsPanelWidget *container )
{
  if ( container )
  {
    QgsSymbolSelectorWidget* dlg = qobject_cast<QgsSymbolSelectorWidget*>( container );
    delete dlg->symbol();
  }
}

void QgsPointDisplacementRendererWidget::updateRendererFromWidget()
{
  QgsRendererWidget* w = qobject_cast<QgsRendererWidget*>( sender() );
  if ( !w )
    return;

  mRenderer->setEmbeddedRenderer( w->renderer()->clone() );
  emit widgetChanged();
}

void QgsPointDisplacementRendererWidget::updateCenterIcon()
{
  QgsMarkerSymbol* symbol = mRenderer->centerSymbol();
  if ( !symbol )
  {
    return;
  }
  QIcon icon = QgsSymbolLayerUtils::symbolPreviewIcon( symbol, mCenterSymbolPushButton->iconSize() );
  mCenterSymbolPushButton->setIcon( icon );
}

void QgsPointDisplacementRendererWidget::setupBlankUi( const QString& layerName )
{
  QLabel* label = new QLabel( tr( "The point displacement renderer only applies to (single) point layers. \n'%1' is not a point layer and cannot be displayed by the point displacement renderer" ).arg( layerName ), this );
  QVBoxLayout* layout = new QVBoxLayout( this );
  layout->setContentsMargins( 0, 0, 0, 0 );
  layout->addWidget( label );
}
