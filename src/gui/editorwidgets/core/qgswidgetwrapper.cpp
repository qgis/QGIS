/***************************************************************************
    qgswidgetwrapper.cpp
     --------------------------------------
    Date                 : 14.5.2013
    Copyright            : (C) 2013 Matthias Kuhn
    Email                : matthias dot kuhn at gmx dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgswidgetwrapper.h"
#include "qgsvectorlayer.h"

#include <QWidget>

QgsWidgetWrapper::QgsWidgetWrapper( QgsVectorLayer* vl, QWidget* editor, QWidget* parent )
    : QObject( parent )
    , mWidget( editor )
    , mParent( parent )
    , mLayer( vl )
    , mInitialized( false )
{
}

QWidget* QgsWidgetWrapper::widget()
{
  if ( !mWidget )
    mWidget = createWidget( mParent );

  if ( !mInitialized )
  {
    mWidget->setProperty( "EWV2Wrapper", QVariant::fromValue<QgsWidgetWrapper*>( this ) );
    initWidget( mWidget );
    mInitialized = true;
  }

  return mWidget;
}

void QgsWidgetWrapper::setConfig( const QgsEditorWidgetConfig& config )
{
  mConfig = config;
}

void QgsWidgetWrapper::setContext( const QgsAttributeEditorContext context )
{
  mContext = context;
}

QVariant QgsWidgetWrapper::config( QString key, QVariant defaultVal )
{
  if ( mConfig.contains( key ) )
  {
    return mConfig[key];
  }
  return defaultVal;
}

const QgsEditorWidgetConfig QgsWidgetWrapper::config()
{
  return mConfig;
}

const QgsAttributeEditorContext& QgsWidgetWrapper::context()
{
  return mContext;
}

QgsVectorLayer* QgsWidgetWrapper::layer()
{
  return mLayer;
}

QgsWidgetWrapper* QgsWidgetWrapper::fromWidget( QWidget* widget )
{
  return widget->property( "EWV2Wrapper" ).value<QgsWidgetWrapper*>();
}

void QgsWidgetWrapper::initWidget( QWidget* editor )
{
  Q_UNUSED( editor )
}

void QgsWidgetWrapper::setEnabled( bool enabled )
{
  Q_UNUSED( enabled );
}
