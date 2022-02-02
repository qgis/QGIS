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
#include <QStyledItemDelegate>

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
#include "qgsexpressioncontextutils.h"
#include "qgsdoublevalidator.h"
#include "qgsmarkersymbol.h"
#include "qgslinesymbol.h"

QgsDataDefinedSizeLegendWidget::QgsDataDefinedSizeLegendWidget( const QgsDataDefinedSizeLegend *ddsLegend, const QgsProperty &ddSize, QgsMarkerSymbol *overrideSymbol, QgsMapCanvas *canvas, QWidget *parent )
  : QgsPanelWidget( parent )
  , mSizeProperty( ddSize )
  , mMapCanvas( canvas )
{
  setupUi( this );
  setPanelTitle( tr( "Data-defined Size Legend" ) );

  mLineSymbolButton->setSymbolType( Qgis::SymbolType::Line );

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

    if ( ddsLegend->lineSymbol() )
      mLineSymbolButton->setSymbol( ddsLegend->lineSymbol()->clone() );

    symbol = ddsLegend->symbol() ? ddsLegend->symbol()->clone() : nullptr;  // may be null (undefined)
  }
  groupBoxOptions->setEnabled( radSeparated->isChecked() );

  if ( overrideSymbol )
  {
    symbol = overrideSymbol;   // takes ownership
    mOverrideSymbol = true;
  }

  if ( !symbol )
  {
    symbol = QgsMarkerSymbol::createSimple( QVariantMap() );
  }
  mSourceSymbol.reset( symbol );

  btnChangeSymbol->setEnabled( !mOverrideSymbol );

  const QIcon icon = QgsSymbolLayerUtils::symbolPreviewIcon( mSourceSymbol.get(), btnChangeSymbol->iconSize() );
  btnChangeSymbol->setIcon( icon );

  editTitle->setText( ddsLegend ? ddsLegend->title() : QString() );

  mSizeClassesModel = new QStandardItemModel( viewSizeClasses );
  mSizeClassesModel->setHorizontalHeaderLabels( QStringList() << tr( "Value" ) << tr( "Label" ) );
  mSizeClassesModel->setSortRole( Qt::UserRole + 1 );
  if ( ddsLegend )
  {
    groupManualSizeClasses->setChecked( !ddsLegend->classes().isEmpty() );
    const auto constClasses = ddsLegend->classes();
    for ( const QgsDataDefinedSizeLegend::SizeClass &sc : constClasses )
    {
      QStandardItem *item = new QStandardItem( QLocale().toString( sc.size ) );
      item->setData( sc.size );
      QStandardItem *itemLabel = new QStandardItem( sc.label );
      mSizeClassesModel->appendRow( QList<QStandardItem *>() << item << itemLabel );
    }
    mSizeClassesModel->sort( 0 );
  }

  connect( btnAddClass, &QToolButton::clicked, this, &QgsDataDefinedSizeLegendWidget::addSizeClass );
  connect( btnRemoveClass, &QToolButton::clicked, this, &QgsDataDefinedSizeLegendWidget::removeSizeClass );

  viewSizeClasses->setItemDelegateForColumn( 0, new SizeClassDelegate( viewSizeClasses ) );
  viewSizeClasses->setModel( mSizeClassesModel );
  connect( mSizeClassesModel, &QStandardItemModel::dataChanged, this, &QgsDataDefinedSizeLegendWidget::onSizeClassesChanged );

  // prepare layer and model to preview legend
  const QgsVectorLayer::LayerOptions options { QgsProject::instance()->transformContext() };
  mPreviewLayer = new QgsVectorLayer( QStringLiteral( "Point?crs=EPSG:4326" ), QStringLiteral( "Preview" ), QStringLiteral( "memory" ), options );
  mPreviewTree = new QgsLayerTree;
  mPreviewLayerNode = mPreviewTree->addLayer( mPreviewLayer );  // node owned by the tree
  mPreviewModel = new QgsLayerTreeModel( mPreviewTree );
  if ( canvas )
    mPreviewModel->setLegendMapViewData( canvas->mapUnitsPerPixel(), canvas->mapSettings().outputDpi(), canvas->scale() );
  viewLayerTree->setModel( mPreviewModel );

  connect( cboAlignSymbols, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, [ = ] { emit widgetChanged(); } );
  connect( radDisabled, &QRadioButton::clicked, this, &QgsPanelWidget::widgetChanged );
  connect( radSeparated, &QRadioButton::clicked, this, &QgsPanelWidget::widgetChanged );
  connect( radCollapsed, &QRadioButton::clicked, this, &QgsPanelWidget::widgetChanged );
  connect( groupManualSizeClasses, &QGroupBox::clicked, this, &QgsPanelWidget::widgetChanged );
  connect( btnChangeSymbol, &QPushButton::clicked, this, &QgsDataDefinedSizeLegendWidget::changeSymbol );
  connect( editTitle, &QLineEdit::textChanged, this, &QgsPanelWidget::widgetChanged );
  connect( mLineSymbolButton, &QgsSymbolButton::changed, this, &QgsPanelWidget::widgetChanged );
  connect( this, &QgsPanelWidget::widgetChanged, this, &QgsDataDefinedSizeLegendWidget::updatePreview );
  connect( radCollapsed, &QRadioButton::toggled, this, [ = ]( bool toggled ) {groupBoxOptions->setEnabled( toggled );} );
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

  ddsLegend->setTitle( editTitle->text() );

  if ( groupManualSizeClasses->isChecked() )
  {
    QList<QgsDataDefinedSizeLegend::SizeClass> classes;
    for ( int i = 0; i < mSizeClassesModel->rowCount(); ++i )
    {
      const double value = mSizeClassesModel->item( i, 0 )->data().toDouble();
      const QString label = mSizeClassesModel->item( i, 1 )->text();
      classes << QgsDataDefinedSizeLegend::SizeClass( value, label );
    }
    ddsLegend->setClasses( classes );
  }

  ddsLegend->setLineSymbol( mLineSymbolButton->clonedSymbol< QgsLineSymbol >() );
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

  const QString crsAuthId = mMapCanvas ? mMapCanvas->mapSettings().destinationCrs().authid() : QString();
  const QgsVectorLayer::LayerOptions options { QgsProject::instance()->transformContext() };
  const std::unique_ptr<QgsVectorLayer> layer = std::make_unique<QgsVectorLayer>( QStringLiteral( "Point?crs=%1" ).arg( crsAuthId ),
      QStringLiteral( "tmp" ),
      QStringLiteral( "memory" ),
      options ) ;

  QgsSymbolSelectorDialog d( newSymbol.get(), QgsStyle::defaultStyle(), layer.get(), this );
  d.setContext( context );

  if ( d.exec() != QDialog::Accepted )
    return;

  mSourceSymbol = std::move( newSymbol );
  const QIcon icon = QgsSymbolLayerUtils::symbolPreviewIcon( mSourceSymbol.get(), btnChangeSymbol->iconSize() );
  btnChangeSymbol->setIcon( icon );

  emit widgetChanged();
}

void QgsDataDefinedSizeLegendWidget::addSizeClass()
{
  bool ok;
  const double v = QInputDialog::getDouble( this, tr( "Add Size Class" ), tr( "Enter value for a new class" ),
                   0, -2147483647, 2147483647, 6, &ok );
  if ( !ok )
    return;

  QStandardItem *item = new QStandardItem( QLocale().toString( v ) );
  item->setData( v );
  QStandardItem *itemLabel = new QStandardItem( QLocale().toString( v ) );
  mSizeClassesModel->appendRow( QList<QStandardItem *>() << item << itemLabel );
  mSizeClassesModel->sort( 0 );
  emit widgetChanged();
}

void QgsDataDefinedSizeLegendWidget::removeSizeClass()
{
  const QModelIndex idx = viewSizeClasses->currentIndex();
  if ( !idx.isValid() )
    return;

  mSizeClassesModel->removeRow( idx.row() );
  emit widgetChanged();
}

void QgsDataDefinedSizeLegendWidget::onSizeClassesChanged()
{
  for ( int row = 0; row < mSizeClassesModel->rowCount(); ++row )
  {
    QStandardItem *item = mSizeClassesModel->item( row, 0 );
    item->setData( QgsDoubleValidator::toDouble( item->text() ) );
  }

  mSizeClassesModel->sort( 0 );
  emit widgetChanged();
}
