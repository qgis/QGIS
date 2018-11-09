/***************************************************************************
  qgsvectorlayerlegendwidget.cpp
  ---------------------
  Date                 : April 2018
  Copyright            : (C) 2018 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsvectorlayerlegendwidget.h"

#include <QBoxLayout>
#include <QStandardItemModel>
#include <QTreeView>

#include "qgsexpressionbuilderdialog.h"
#include "qgsmapcanvas.h"
#include "qgsmaplayerlegend.h"
#include "qgsrenderer.h"
#include "qgssymbollayerutils.h"
#include "qgstextformatwidget.h"
#include "qgsvectorlayer.h"
#include "qgsfontbutton.h"

QgsVectorLayerLegendWidget::QgsVectorLayerLegendWidget( QWidget *parent )
  : QWidget( parent )
{
  mLegendTreeView = new QTreeView;
  mLegendTreeView->setRootIsDecorated( false );

  mTextOnSymbolFormatButton = new QgsFontButton( nullptr, tr( "Legend Text Format" ) );
  mTextOnSymbolFormatButton->setText( tr( "Text Format" ) );
  mTextOnSymbolFormatButton->setMode( QgsFontButton::ModeTextRenderer );

  mTextOnSymbolFromExpressionButton = new QPushButton( tr( "Set Labels from Expression…" ) );
  connect( mTextOnSymbolFromExpressionButton, &QPushButton::clicked, this, &QgsVectorLayerLegendWidget::labelsFromExpression );

  mTextOnSymbolGroupBox = new QgsCollapsibleGroupBox;

  QHBoxLayout *buttonsLayout = new QHBoxLayout;
  buttonsLayout->addWidget( mTextOnSymbolFormatButton );
  buttonsLayout->addWidget( mTextOnSymbolFromExpressionButton );
  buttonsLayout->addStretch();

  QVBoxLayout *groupLayout = new QVBoxLayout;
  groupLayout->addWidget( mLegendTreeView );
  groupLayout->addLayout( buttonsLayout );

  mTextOnSymbolGroupBox->setTitle( tr( "Text on Symbols" ) );
  mTextOnSymbolGroupBox->setCheckable( true );
  mTextOnSymbolGroupBox->setLayout( groupLayout );
  mTextOnSymbolGroupBox->setCollapsed( false );

  QVBoxLayout *layout = new QVBoxLayout;
  layout->setMargin( 0 );
  layout->setContentsMargins( 0, 0, 0, 0 );
  layout->addWidget( mTextOnSymbolGroupBox );
  setLayout( layout );
}

void QgsVectorLayerLegendWidget::setMapCanvas( QgsMapCanvas *canvas )
{
  mCanvas = canvas;
  mTextOnSymbolFormatButton->setMapCanvas( mCanvas );
}

void QgsVectorLayerLegendWidget::setLayer( QgsVectorLayer *layer )
{
  mLayer = layer;

  QgsDefaultVectorLayerLegend *legend = qobject_cast<QgsDefaultVectorLayerLegend *>( layer->legend() );
  if ( !legend )
    return;

  mTextOnSymbolGroupBox->setChecked( legend->textOnSymbolEnabled() );
  mTextOnSymbolFormatButton->setTextFormat( legend->textOnSymbolTextFormat() );
  populateLegendTreeView( legend->textOnSymbolContent() );
}


void QgsVectorLayerLegendWidget::populateLegendTreeView( const QHash<QString, QString> &content )
{
  QStandardItemModel *model = new QStandardItemModel;
  model->setColumnCount( 2 );
  model->setHorizontalHeaderLabels( QStringList() << tr( "Symbol" ) << tr( "Text" ) );

  const QgsLegendSymbolList lst = mLayer->renderer() ? mLayer->renderer()->legendSymbolItems() : QgsLegendSymbolList();
  for ( const QgsLegendSymbolItem &symbolItem : lst )
  {
    if ( !symbolItem.symbol() )
      continue;

    QgsRenderContext context;
    QSize iconSize( 16, 16 );
    QIcon icon = QgsSymbolLayerUtils::symbolPreviewPixmap( symbolItem.symbol(), iconSize, 0, &context );

    QStandardItem *item1 = new QStandardItem( icon, symbolItem.label() );
    item1->setEditable( false );
    QStandardItem *item2 = new QStandardItem;
    if ( symbolItem.ruleKey().isEmpty() )
    {
      item1->setEnabled( false );
      item2->setEnabled( false );
    }
    else
    {
      item1->setData( symbolItem.ruleKey() );
      if ( content.contains( symbolItem.ruleKey() ) )
        item2->setText( content.value( symbolItem.ruleKey() ) );
    }
    model->appendRow( QList<QStandardItem *>() << item1 << item2 );
  }
  mLegendTreeView->setModel( model );
  mLegendTreeView->resizeColumnToContents( 0 );
}


void QgsVectorLayerLegendWidget::applyToLayer()
{
  QgsDefaultVectorLayerLegend *legend = new QgsDefaultVectorLayerLegend( mLayer );
  legend->setTextOnSymbolEnabled( mTextOnSymbolGroupBox->isChecked() );
  legend->setTextOnSymbolTextFormat( mTextOnSymbolFormatButton->textFormat() );

  QHash<QString, QString> content;
  if ( QStandardItemModel *model = qobject_cast<QStandardItemModel *>( mLegendTreeView->model() ) )
  {
    for ( int i = 0; i < model->rowCount(); ++i )
    {
      QString ruleKey = model->item( i, 0 )->data().toString();
      QString label = model->item( i, 1 )->text();
      if ( !label.isEmpty() )
        content[ruleKey] = label;
    }
  }
  legend->setTextOnSymbolContent( content );

  mLayer->setLegend( legend );
}

void QgsVectorLayerLegendWidget::labelsFromExpression()
{
  QHash<QString, QString> content;
  QgsRenderContext context( QgsRenderContext::fromMapSettings( mCanvas->mapSettings() ) );

  QgsExpressionBuilderDialog dlgExpression( mLayer );
  dlgExpression.setExpressionContext( context.expressionContext() );
  if ( !dlgExpression.exec() )
    return;

  QgsExpression expr( dlgExpression.expressionText() );
  expr.prepare( &context.expressionContext() );

  std::unique_ptr< QgsFeatureRenderer > r( mLayer->renderer()->clone() );

  QgsFeature f;
  QgsFeatureRequest request;
  request.setSubsetOfAttributes( r->usedAttributes( context ), mLayer->fields() );
  QgsFeatureIterator fi = mLayer->getFeatures();

  r->startRender( context, mLayer->fields() );
  while ( fi.nextFeature( f ) )
  {
    context.expressionContext().setFeature( f );
    const QSet<QString> keys = r->legendKeysForFeature( f, context );
    for ( const QString &key : keys )
    {
      if ( content.contains( key ) )
        continue;

      QString label = expr.evaluate( &context.expressionContext() ).toString();
      if ( !label.isEmpty() )
        content[key] = label;
    }
  }
  r->stopRender( context );

  populateLegendTreeView( content );
}
