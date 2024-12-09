/***************************************************************************
    qgscalloutpanelwidget.cpp
    ---------------------
    begin                : July 2024
    copyright            : (C) 2024 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscalloutpanelwidget.h"
#include "moc_qgscalloutpanelwidget.cpp"
#include "qgsapplication.h"
#include "qgscalloutsregistry.h"
#include "qgscalloutwidget.h"
#include "qgsgui.h"

QgsCalloutPanelWidget::QgsCalloutPanelWidget( QWidget *parent, QgsMapLayer *layer )
  : QgsPanelWidget( parent )
  , mLayer( layer )
{
  QgsGui::initCalloutWidgets();

  setupUi( this );

  const QStringList calloutTypes = QgsApplication::calloutRegistry()->calloutTypes();
  for ( const QString &type : calloutTypes )
  {
    mCalloutStyleComboBox->addItem( QgsApplication::calloutRegistry()->calloutMetadata( type )->icon(), QgsApplication::calloutRegistry()->calloutMetadata( type )->visibleName(), type );
  }

  connect( mCalloutStyleComboBox, qOverload<int>( &QComboBox::currentIndexChanged ), this, &QgsCalloutPanelWidget::calloutTypeChanged );
  calloutTypeChanged();
}

void QgsCalloutPanelWidget::setGeometryType( Qgis::GeometryType type )
{
  mGeometryType = type;
}

void QgsCalloutPanelWidget::setContext( const QgsSymbolWidgetContext &context )
{
  mContext = context;
  if ( QgsCalloutWidget *cw = qobject_cast<QgsCalloutWidget *>( mCalloutStackedWidget->currentWidget() ) )
  {
    cw->setContext( context );
  }
}

QgsSymbolWidgetContext QgsCalloutPanelWidget::context() const
{
  return mContext;
}

void QgsCalloutPanelWidget::setCallout( const QgsCallout *callout )
{
  if ( callout )
  {
    whileBlocking( mCalloutStyleComboBox )->setCurrentIndex( mCalloutStyleComboBox->findData( callout->type() ) );
    updateCalloutWidget( callout );
    emit calloutChanged();
  }
}

QgsCallout *QgsCalloutPanelWidget::callout()
{
  const QString calloutType = mCalloutStyleComboBox->currentData().toString();
  std::unique_ptr<QgsCallout> callout;
  if ( QgsCalloutWidget *pew = qobject_cast<QgsCalloutWidget *>( mCalloutStackedWidget->currentWidget() ) )
  {
    callout.reset( pew->callout()->clone() );
  }
  if ( !callout )
    callout.reset( QgsApplication::calloutRegistry()->createCallout( calloutType ) );

  callout->setEnabled( true );
  return callout.release();
}

void QgsCalloutPanelWidget::calloutTypeChanged()
{
  const QString newCalloutType = mCalloutStyleComboBox->currentData().toString();
  QgsCalloutWidget *pew = qobject_cast<QgsCalloutWidget *>( mCalloutStackedWidget->currentWidget() );
  if ( pew )
  {
    if ( pew->callout() && pew->callout()->type() == newCalloutType )
      return;
  }

  // get creation function for new callout from registry
  QgsCalloutRegistry *registry = QgsApplication::calloutRegistry();
  QgsCalloutAbstractMetadata *am = registry->calloutMetadata( newCalloutType );
  if ( !am ) // check whether the metadata is assigned
    return;

  // change callout to a new one (with different type)
  // base new callout on existing callout's properties
  const std::unique_ptr<QgsCallout> newCallout( am->createCallout( pew && pew->callout() ? pew->callout()->properties( QgsReadWriteContext() ) : QVariantMap(), QgsReadWriteContext() ) );
  if ( !newCallout )
    return;

  updateCalloutWidget( newCallout.get() );
  emit calloutChanged();
}

void QgsCalloutPanelWidget::updateCalloutWidget( const QgsCallout *callout )
{
  if ( !callout )
  {
    mCalloutStackedWidget->setCurrentWidget( pageDummy );
    return;
  }

  if ( mCalloutStackedWidget->currentWidget() != pageDummy )
  {
    // stop updating from the original widget
    if ( QgsCalloutWidget *pew = qobject_cast<QgsCalloutWidget *>( mCalloutStackedWidget->currentWidget() ) )
      disconnect( pew, &QgsCalloutWidget::changed, this, &QgsCalloutPanelWidget::calloutChanged );
  }

  QgsCalloutRegistry *registry = QgsApplication::calloutRegistry();
  if ( QgsCalloutAbstractMetadata *am = registry->calloutMetadata( callout->type() ) )
  {
    if ( QgsCalloutWidget *w = am->createCalloutWidget( mLayer ) )
    {
      Qgis::GeometryType geometryType = mGeometryType;
      if ( QgsVectorLayer *vLayer = qobject_cast<QgsVectorLayer *>( mLayer ) )
        geometryType = vLayer->geometryType();
      w->setGeometryType( geometryType );
      w->setCallout( callout );

      w->setContext( context() );
      mCalloutStackedWidget->addWidget( w );
      mCalloutStackedWidget->setCurrentWidget( w );
      // start receiving updates from widget
      connect( w, &QgsCalloutWidget::changed, this, &QgsCalloutPanelWidget::calloutChanged );
      return;
    }
  }
  // When anything is not right
  mCalloutStackedWidget->setCurrentWidget( pageDummy );
}
