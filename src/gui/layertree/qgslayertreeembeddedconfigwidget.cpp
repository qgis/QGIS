/***************************************************************************
  qgslayertreeembeddedconfigwidget.cpp
  --------------------------------------
  Date                 : May 2016
  Copyright            : (C) 2016 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayertreeembeddedconfigwidget.h"

#include "qgsmaplayer.h"
#include "qgslayertreeembeddedwidgetregistry.h"
#include "qgsgui.h"
#include <QStringListModel>
#include <QStandardItemModel>

QgsLayerTreeEmbeddedConfigWidget::QgsLayerTreeEmbeddedConfigWidget( QWidget *parent )
  : QWidget( parent )

{
  setupUi( this );
}

void QgsLayerTreeEmbeddedConfigWidget::setLayer( QgsMapLayer *layer )
{
  mLayer = layer;

  connect( mBtnAdd, &QAbstractButton::clicked, this, &QgsLayerTreeEmbeddedConfigWidget::onAddClicked );
  connect( mBtnRemove, &QAbstractButton::clicked, this, &QgsLayerTreeEmbeddedConfigWidget::onRemoveClicked );

  QStandardItemModel *modelAvailable = new QStandardItemModel( this );
  QStandardItemModel *modelUsed = new QStandardItemModel( this );

  // populate available
  const auto constProviders = QgsGui::layerTreeEmbeddedWidgetRegistry()->providers();
  for ( const QString &providerId : constProviders )
  {
    QgsLayerTreeEmbeddedWidgetProvider *provider = QgsGui::layerTreeEmbeddedWidgetRegistry()->provider( providerId );
    if ( provider->supportsLayer( mLayer ) )
    {
      QStandardItem *item = new QStandardItem( provider->name() );
      item->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable );
      item->setData( provider->id(), Qt::UserRole + 1 );
      modelAvailable->appendRow( item );
    }
  }
  mListAvailable->setModel( modelAvailable );

  // populate used
  const int widgetsCount = layer->customProperty( QStringLiteral( "embeddedWidgets/count" ), 0 ).toInt();
  for ( int i = 0; i < widgetsCount; ++i )
  {
    const QString providerId = layer->customProperty( QStringLiteral( "embeddedWidgets/%1/id" ).arg( i ) ).toString();
    if ( QgsLayerTreeEmbeddedWidgetProvider *provider = QgsGui::layerTreeEmbeddedWidgetRegistry()->provider( providerId ) )
    {
      QStandardItem *item = new QStandardItem( provider->name() );
      item->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable );
      item->setData( provider->id(), Qt::UserRole + 1 );
      modelUsed->appendRow( item );
    }
  }
  mListUsed->setModel( modelUsed );
}

void QgsLayerTreeEmbeddedConfigWidget::onAddClicked()
{
  if ( !mListAvailable->currentIndex().isValid() )
    return;

  const QString providerId = mListAvailable->model()->data( mListAvailable->currentIndex(), Qt::UserRole + 1 ).toString();
  QgsLayerTreeEmbeddedWidgetProvider *provider = QgsGui::layerTreeEmbeddedWidgetRegistry()->provider( providerId );
  if ( !provider )
    return;

  if ( QStandardItemModel *model = qobject_cast<QStandardItemModel *>( mListUsed->model() ) )
  {
    QStandardItem *item = new QStandardItem( provider->name() );
    item->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable );
    item->setData( provider->id(), Qt::UserRole + 1 );
    model->appendRow( item );
  }
}

void QgsLayerTreeEmbeddedConfigWidget::onRemoveClicked()
{
  if ( !mListUsed->currentIndex().isValid() )
    return;

  const int row = mListUsed->currentIndex().row();
  mListUsed->model()->removeRow( row );
}

void QgsLayerTreeEmbeddedConfigWidget::applyToLayer()
{
  if ( !mLayer )
    return;

  // clear old properties
  const int widgetsCount = mLayer->customProperty( QStringLiteral( "embeddedWidgets/count" ), 0 ).toInt();
  for ( int i = 0; i < widgetsCount; ++i )
  {
    mLayer->removeCustomProperty( QStringLiteral( "embeddedWidgets/%1/id" ).arg( i ) );
  }

  // setup new properties
  const int newCount = mListUsed->model()->rowCount();
  mLayer->setCustomProperty( QStringLiteral( "embeddedWidgets/count" ), newCount );
  for ( int i = 0; i < newCount; ++i )
  {
    const QString providerId = mListUsed->model()->data( mListUsed->model()->index( i, 0 ), Qt::UserRole + 1 ).toString();
    mLayer->setCustomProperty( QStringLiteral( "embeddedWidgets/%1/id" ).arg( i ), providerId );
  }
}
