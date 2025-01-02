/***************************************************************************
    qgswidgetwrapper.cpp
     --------------------------------------
    Date                 : 14.5.2013
    Copyright            : (C) 2013 Matthias Kuhn
    Email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgswidgetwrapper.h"
#include "moc_qgswidgetwrapper.cpp"
#include "qgsvectorlayer.h"

#include <QWidget>


const QgsPropertiesDefinition &QgsWidgetWrapper::propertyDefinitions()
{
  static QgsPropertiesDefinition properties;

  if ( properties.isEmpty() )
  {
    properties = {
      { static_cast<int>( Property::RootPath ), QgsPropertyDefinition( "propertyRootPath", QgsPropertyDefinition::DataTypeString, QObject::tr( "Root path" ), QObject::tr( "string of variable length representing root path to attachment" ) ) },
      { static_cast<int>( Property::DocumentViewerContent ), QgsPropertyDefinition( "documentViewerContent", QgsPropertyDefinition::DataTypeString, QObject::tr( "Document viewer content" ), QObject::tr( "string" ) + "<b>NoContent</b>|<b>Image</b>|<b>Audio</b>|<b>Video</b>|<b>Web</b>" ) },
      { static_cast<int>( Property::StorageUrl ), QgsPropertyDefinition( "storageUrl", QgsPropertyDefinition::DataTypeString, QObject::tr( "Storage Url" ), QObject::tr( "String of variable length representing the URL used to store document with an external storage" ) ) }
    };
  }
  return properties;
}

QgsWidgetWrapper::QgsWidgetWrapper( QgsVectorLayer *vl, QWidget *editor, QWidget *parent )
  : QObject( parent )
  , mWidget( editor )
  , mParent( parent )
  , mLayer( vl )
  , mInitialized( false )
{
}

QWidget *QgsWidgetWrapper::widget()
{
  if ( !mWidget )
    mWidget = createWidget( mParent );

  if ( !mInitialized )
  {
    mWidget->setProperty( "EWV2Wrapper", QVariant::fromValue<QgsWidgetWrapper *>( this ) );
    initWidget( mWidget );
    mInitialized = true;
  }

  return mWidget;
}

void QgsWidgetWrapper::setConfig( const QVariantMap &config )
{
  mConfig = config;
}

void QgsWidgetWrapper::setContext( const QgsAttributeEditorContext &context )
{
  mContext = context;
  emit contextChanged();
}

QVariant QgsWidgetWrapper::config( const QString &key, const QVariant &defaultVal ) const
{
  if ( mConfig.contains( key ) )
  {
    return mConfig[key];
  }
  return defaultVal;
}

QVariantMap QgsWidgetWrapper::config() const
{
  return mConfig;
}

const QgsAttributeEditorContext &QgsWidgetWrapper::context() const
{
  return mContext;
}

QgsVectorLayer *QgsWidgetWrapper::layer() const
{
  return mLayer;
}

QgsWidgetWrapper *QgsWidgetWrapper::fromWidget( QWidget *widget )
{
  return widget->property( "EWV2Wrapper" ).value<QgsWidgetWrapper *>();
}

void QgsWidgetWrapper::notifyAboutToSave()
{
  aboutToSave();
}

void QgsWidgetWrapper::initWidget( QWidget *editor )
{
  Q_UNUSED( editor )
}

void QgsWidgetWrapper::setEnabled( bool enabled )
{
  Q_UNUSED( enabled )
}

void QgsWidgetWrapper::aboutToSave()
{
}
