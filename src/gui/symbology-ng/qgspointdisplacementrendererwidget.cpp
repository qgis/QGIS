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
#include "qgsrendererv2registry.h"
#include "qgsfield.h"
#include "qgsstylev2.h"
#include "qgssymbolv2selectordialog.h"
#include "qgssymbollayerv2utils.h"
#include "qgsvectorlayer.h"
#include "qgisgui.h"

QgsRendererV2Widget* QgsPointDisplacementRendererWidget::create( QgsVectorLayer* layer, QgsStyleV2* style, QgsFeatureRendererV2* renderer )
{
  return new QgsPointDisplacementRendererWidget( layer, style, renderer );
}

QgsPointDisplacementRendererWidget::QgsPointDisplacementRendererWidget( QgsVectorLayer* layer, QgsStyleV2* style, QgsFeatureRendererV2* renderer )
    : QgsRendererV2Widget( layer, style )
    , mRenderer( nullptr )
    , mEmbeddedRendererWidget( nullptr )
{
  if ( !layer )
  {
    return;
  }

  //the renderer only applies to point vector layers
  if ( layer->wkbType() != QGis::WKBPoint && layer->wkbType()  != QGis::WKBPoint25D )
  {
    //setup blank dialog
    mRenderer = nullptr;
    setupBlankUi( layer->name() );
    return;
  }
  setupUi( this );

  mDistanceUnitWidget->setUnits( QgsSymbolV2::OutputUnitList() << QgsSymbolV2::MM << QgsSymbolV2::MapUnit << QgsSymbolV2::Pixel );

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
  QStringList rendererList = QgsRendererV2Registry::instance()->renderersList( QgsRendererV2AbstractMetadata::PointLayer );
  QStringList::const_iterator it = rendererList.constBegin();
  for ( ; it != rendererList.constEnd(); ++it )
  {
    if ( *it != "pointDisplacement" )
    {
      QgsRendererV2AbstractMetadata* m = QgsRendererV2Registry::instance()->rendererMetadata( *it );
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
  delete mEmbeddedRendererWidget;
}

QgsFeatureRendererV2* QgsPointDisplacementRendererWidget::renderer()
{
  if ( mRenderer && mEmbeddedRendererWidget )
  {
    QgsFeatureRendererV2* embeddedRenderer = mEmbeddedRendererWidget->renderer();
    if ( embeddedRenderer )
    {
      mRenderer->setEmbeddedRenderer( embeddedRenderer->clone() );
    }
  }
  return mRenderer;
}

void QgsPointDisplacementRendererWidget::setMapCanvas( QgsMapCanvas* canvas )
{
  QgsRendererV2Widget::setMapCanvas( canvas );
  if ( mDistanceUnitWidget )
    mDistanceUnitWidget->setMapCanvas( canvas );
  if ( mEmbeddedRendererWidget )
    mEmbeddedRendererWidget->setMapCanvas( canvas );
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
  QgsRendererV2AbstractMetadata* m = QgsRendererV2Registry::instance()->rendererMetadata( rendererId );
  if ( m )
  {
    delete mEmbeddedRendererWidget;
    mEmbeddedRendererWidget = m->createRendererWidget( mLayer, mStyle, mRenderer->embeddedRenderer()->clone() );
    mEmbeddedRendererWidget->setMapCanvas( mMapCanvas );
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
  if ( mEmbeddedRendererWidget )
  {
    //create a dialog with the embedded widget
#ifdef Q_OS_MAC
    QDialog* d = new QDialog( this->window() );
    d->setWindowModality( Qt::WindowModal );
#else
    QDialog* d = new QDialog();
#endif
    QGridLayout* layout = new QGridLayout( d );
    mEmbeddedRendererWidget->setParent( d );
    QDialogButtonBox* buttonBox = new QDialogButtonBox( d );
    buttonBox->addButton( QDialogButtonBox::Ok );
    QObject::connect( buttonBox, SIGNAL( accepted() ), d, SLOT( accept() ) );
    layout->addWidget( mEmbeddedRendererWidget, 0, 0 );
    layout->addWidget( buttonBox, 1, 0 );
    d->exec();
    mEmbeddedRendererWidget->setParent( nullptr );
    delete d;
    emit widgetChanged();
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
  QgsMarkerSymbolV2* markerSymbol = mRenderer->centerSymbol()->clone();
  QgsSymbolV2SelectorDialog dlg( markerSymbol, QgsStyleV2::defaultStyle(), mLayer, this );
  dlg.setMapCanvas( mMapCanvas );
  if ( dlg.exec() == QDialog::Rejected )
  {
    delete markerSymbol;
    return;
  }
  mRenderer->setCenterSymbol( markerSymbol );
  updateCenterIcon();
  emit widgetChanged();
}

void QgsPointDisplacementRendererWidget::updateCenterIcon()
{
  QgsMarkerSymbolV2* symbol = mRenderer->centerSymbol();
  if ( !symbol )
  {
    return;
  }
  QIcon icon = QgsSymbolLayerV2Utils::symbolPreviewIcon( symbol, mCenterSymbolPushButton->iconSize() );
  mCenterSymbolPushButton->setIcon( icon );
}

void QgsPointDisplacementRendererWidget::setupBlankUi( const QString& layerName )
{
  QLabel* label = new QLabel( tr( "The point displacement renderer only applies to (single) point layers. \n'%1' is not a point layer and cannot be displayed by the point displacement renderer" ).arg( layerName ), this );
  QVBoxLayout* layout = new QVBoxLayout( this );
  layout->setContentsMargins( 0, 0, 0, 0 );
  layout->addWidget( label );
}
