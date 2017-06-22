/***************************************************************************
  qgsdatadefinedsizelegendwidget.cpp
  --------------------------------------
  Date                 : June 2017
  Copyright            : (C) 2017 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdatadefinedsizelegendwidget.h"

#include <QInputDialog>

#include "qgsdatadefinedsizelegend.h"
#include "qgslayertree.h"
#include "qgslayertreemodel.h"
#include "qgsmapcanvas.h"
#include "qgssinglesymbolrenderer.h"
#include "qgsstyle.h"
#include "qgssymbol.h"
#include "qgssymbollayer.h"
#include "qgssymbolselectordialog.h"
#include "qgsvectorlayer.h"

QgsDataDefinedSizeLegendWidget::QgsDataDefinedSizeLegendWidget( const QgsDataDefinedSizeLegend *ddsLegend, const QgsProperty &ddSize, QgsMarkerSymbol *overrideSymbol, QgsMapCanvas *canvas, QWidget *parent )
  : QgsPanelWidget( parent )
  , mSizeProperty( ddSize )
  , mMapCanvas( canvas )
{
  setupUi( this );
  setPanelTitle( tr( "Data-defined size legend" ) );

  QgsMarkerSymbol *symbol = nullptr;

  if ( !ddsLegend )
  {
    radDisabled->setChecked( true );
  }
  else
  {
    if ( ddsLegend->legendType() == QgsDataDefinedSizeLegend::LegendSeparated )
      radSeparated->setChecked( true );
    else
      radCollapsed->setChecked( true );

    if ( ddsLegend->verticalAlignment() == QgsDataDefinedSizeLegend::AlignBottom )
      cboAlignSymbols->setCurrentIndex( 0 );
    else
      cboAlignSymbols->setCurrentIndex( 1 );

    symbol = ddsLegend->symbol() ? ddsLegend->symbol()->clone() : nullptr;  // may be null (undefined)
  }

  if ( overrideSymbol )
  {
    symbol = overrideSymbol;   // takes ownership
    mOverrideSymbol = true;
  }

  if ( !symbol )
  {
    symbol = QgsMarkerSymbol::createSimple( QgsStringMap() );
  }
  mSourceSymbol.reset( symbol );

  btnChangeSymbol->setEnabled( !mOverrideSymbol );

  QIcon icon = QgsSymbolLayerUtils::symbolPreviewIcon( mSourceSymbol.get(), btnChangeSymbol->iconSize() );
  btnChangeSymbol->setIcon( icon );

  mSizeClassesModel = new QStandardItemModel( viewSizeClasses );
  mSizeClassesModel->setHorizontalHeaderLabels( QStringList() << tr( "Value" ) );
  mSizeClassesModel->setSortRole( Qt::UserRole + 1 );
  if ( ddsLegend )
  {
    groupManualSizeClasses->setChecked( !ddsLegend->classes().isEmpty() );
    Q_FOREACH ( const QgsDataDefinedSizeLegend::SizeClass &sc, ddsLegend->classes() )
    {
      QStandardItem *item = new QStandardItem( sc.label );
      item->setEditable( false );
      item->setData( sc.label.toInt() );
      mSizeClassesModel->appendRow( item );
    }
    mSizeClassesModel->sort( 0 );
  }

  connect( btnAddClass, &QToolButton::clicked, this, &QgsDataDefinedSizeLegendWidget::addSizeClass );
  connect( btnRemoveClass, &QToolButton::clicked, this, &QgsDataDefinedSizeLegendWidget::removeSizeClass );

  viewSizeClasses->setModel( mSizeClassesModel );

  // prepare layer and model to preview legend
  mPreviewLayer = new QgsVectorLayer( "Point?crs=EPSG:4326", "Preview", "memory" );
  mPreviewTree = new QgsLayerTree;
  mPreviewLayerNode = mPreviewTree->addLayer( mPreviewLayer );  // node owned by the tree
  mPreviewModel = new QgsLayerTreeModel( mPreviewTree );
  if ( canvas )
    mPreviewModel->setLegendMapViewData( canvas->mapUnitsPerPixel(), canvas->mapSettings().outputDpi(), canvas->scale() );
  viewLayerTree->setModel( mPreviewModel );

  connect( cboAlignSymbols, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), [ = ] { emit widgetChanged(); } );
  connect( radDisabled, &QRadioButton::clicked, this, &QgsPanelWidget::widgetChanged );
  connect( radSeparated, &QRadioButton::clicked, this, &QgsPanelWidget::widgetChanged );
  connect( radCollapsed, &QRadioButton::clicked, this, &QgsPanelWidget::widgetChanged );
  connect( groupManualSizeClasses, &QGroupBox::clicked, this, &QgsPanelWidget::widgetChanged );
  connect( btnChangeSymbol, &QPushButton::clicked, this, &QgsDataDefinedSizeLegendWidget::changeSymbol );
  connect( this, &QgsPanelWidget::widgetChanged, this, &QgsDataDefinedSizeLegendWidget::updatePreview );
  updatePreview();
}

QgsDataDefinedSizeLegendWidget::~QgsDataDefinedSizeLegendWidget()
{
  delete mPreviewModel;
  delete mPreviewTree;
  delete mPreviewLayer;
}

QgsDataDefinedSizeLegend *QgsDataDefinedSizeLegendWidget::dataDefinedSizeLegend() const
{
  if ( radDisabled->isChecked() )
    return nullptr;

  QgsDataDefinedSizeLegend *ddsLegend = new QgsDataDefinedSizeLegend;
  ddsLegend->setLegendType( radSeparated->isChecked() ? QgsDataDefinedSizeLegend::LegendSeparated : QgsDataDefinedSizeLegend::LegendCollapsed );
  ddsLegend->setVerticalAlignment( cboAlignSymbols->currentIndex() == 0 ? QgsDataDefinedSizeLegend::AlignBottom : QgsDataDefinedSizeLegend::AlignCenter );
  if ( !mOverrideSymbol )
  {
    ddsLegend->setSymbol( mSourceSymbol->clone() );
  }
  if ( groupManualSizeClasses->isChecked() )
  {
    const QgsSizeScaleTransformer *transformer = dynamic_cast<const QgsSizeScaleTransformer *>( mSizeProperty.transformer() );
    QList<QgsDataDefinedSizeLegend::SizeClass> classes;
    for ( int i = 0; i < mSizeClassesModel->rowCount(); ++i )
    {
      double value = mSizeClassesModel->data( mSizeClassesModel->index( i, 0 ), Qt::UserRole + 1 ).toDouble();
      double size = transformer ? transformer->size( value ) : value;
      classes << QgsDataDefinedSizeLegend::SizeClass( size, QString::number( value ) );
    }
    ddsLegend->setClasses( classes );
  }
  return ddsLegend;
}

void QgsDataDefinedSizeLegendWidget::updatePreview()
{
  QgsMarkerSymbol *symbol = mSourceSymbol->clone();
  symbol->setDataDefinedSize( mSizeProperty );
  QgsSingleSymbolRenderer *r = new QgsSingleSymbolRenderer( symbol );
  r->setDataDefinedSizeLegend( dataDefinedSizeLegend() );
  mPreviewLayer->setRenderer( r );
  mPreviewModel->refreshLayerLegend( mPreviewLayerNode );
  viewLayerTree->expandAll();
}

void QgsDataDefinedSizeLegendWidget::changeSymbol()
{
  std::unique_ptr<QgsMarkerSymbol> newSymbol( mSourceSymbol->clone() );
  QgsSymbolWidgetContext context;
  if ( mMapCanvas )
    context.setMapCanvas( mMapCanvas );

  QgsExpressionContext ec;
  ec << QgsExpressionContextUtils::globalScope()
     << QgsExpressionContextUtils::projectScope( QgsProject::instance() )
     << QgsExpressionContextUtils::atlasScope( nullptr );
  if ( mMapCanvas )
    ec << QgsExpressionContextUtils::mapSettingsScope( mMapCanvas->mapSettings() );
  context.setExpressionContext( &ec );

  QString crsAuthId = mMapCanvas ? mMapCanvas->mapSettings().destinationCrs().authid() : QString();
  std::unique_ptr<QgsVectorLayer> layer( new QgsVectorLayer( "Point?crs=" + crsAuthId, "tmp", "memory" ) );

  QgsSymbolSelectorDialog d( newSymbol.get(), QgsStyle::defaultStyle(), layer.get(), this );
  d.setContext( context );

  if ( d.exec() != QDialog::Accepted )
    return;

  mSourceSymbol.reset( newSymbol.release() );
  QIcon icon = QgsSymbolLayerUtils::symbolPreviewIcon( mSourceSymbol.get(), btnChangeSymbol->iconSize() );
  btnChangeSymbol->setIcon( icon );

  emit widgetChanged();
}

void QgsDataDefinedSizeLegendWidget::addSizeClass()
{
  bool ok;
  double v = QInputDialog::getDouble( this, tr( "Add Size Class" ), tr( "Enter value for a new class" ),
                                      0, -2147483647, 2147483647, 6, &ok );
  if ( !ok )
    return;

  QStandardItem *item = new QStandardItem( QString::number( v ) );
  item->setEditable( false );
  item->setData( v );
  mSizeClassesModel->appendRow( item );
  mSizeClassesModel->sort( 0 );
  emit widgetChanged();
}

void QgsDataDefinedSizeLegendWidget::removeSizeClass()
{
  QModelIndex idx = viewSizeClasses->currentIndex();
  if ( !idx.isValid() )
    return;

  mSizeClassesModel->removeRow( idx.row() );
  emit widgetChanged();
}
