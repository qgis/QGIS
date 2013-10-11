/***************************************************************************
    qgseditorwidgetregistry.cpp
     --------------------------------------
    Date                 : 24.4.2013
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

#include "qgseditorwidgetregistry.h"

#include "qgseditorwidgetfactory.h"
#include "qgsproject.h"
#include "qgsvectorlayer.h"
#include "qgsmessagelog.h"

QgsEditorWidgetRegistry* QgsEditorWidgetRegistry::instance()
{
  static QgsEditorWidgetRegistry sInstance;
  return &sInstance;
}

QgsEditorWidgetRegistry::QgsEditorWidgetRegistry()
{
  connect( QgsProject::instance(), SIGNAL( readMapLayer( QgsMapLayer*, const QDomElement& ) ), this, SLOT( readMapLayer( QgsMapLayer*, const QDomElement& ) ) );
  connect( QgsProject::instance(), SIGNAL( writeMapLayer( QgsMapLayer*, QDomElement&, QDomDocument& ) ), this, SLOT( writeMapLayer( QgsMapLayer*, QDomElement&, QDomDocument& ) ) );
}

QgsEditorWidgetRegistry::~QgsEditorWidgetRegistry()
{
  qDeleteAll( mWidgetFactories.values() );
}

QgsEditorWidgetWrapper* QgsEditorWidgetRegistry::create( const QString& widgetId, QgsVectorLayer* vl, int fieldIdx, const QgsEditorWidgetConfig& config, QWidget* editor, QWidget* parent )
{
  if ( mWidgetFactories.contains( widgetId ) )
  {
    QgsEditorWidgetWrapper* ww = mWidgetFactories[widgetId]->create( vl, fieldIdx, editor, parent );
    if ( ww )
    {
      ww->setConfig( config );
      return ww;
    }
  }
  return 0;
}

QgsEditorConfigWidget* QgsEditorWidgetRegistry::createConfigWidget( const QString& widgetId, QgsVectorLayer* vl, int fieldIdx, QWidget* parent )
{
  if ( mWidgetFactories.contains( widgetId ) )
  {
    return mWidgetFactories[widgetId]->configWidget( vl, fieldIdx, parent );
  }
  return 0;
}

QString QgsEditorWidgetRegistry::name( const QString& widgetId )
{
  if ( mWidgetFactories.contains( widgetId ) )
  {
    return mWidgetFactories[widgetId]->name();
  }

  return QString();
}

const QMap<QString, QgsEditorWidgetFactory*> QgsEditorWidgetRegistry::factories()
{
  return mWidgetFactories;
}

bool QgsEditorWidgetRegistry::registerWidget( const QString& widgetId, QgsEditorWidgetFactory* widgetFactory )
{
  if ( !widgetFactory )
  {
    QgsMessageLog::instance()->logMessage( "QgsEditorWidgetRegistry: Factory not valid." );
    return false;
  }
  else if ( mWidgetFactories.contains( widgetId ) )
  {
    QgsMessageLog::instance()->logMessage( QString( "QgsEditorWidgetRegistry: Factory with id %1 already registered." ).arg( widgetId ) );
    return false;
  }
  else
  {
    mWidgetFactories.insert( widgetId, widgetFactory );
    return true;
  }
}

void QgsEditorWidgetRegistry::readMapLayer( QgsMapLayer* mapLayer, const QDomElement& layerElem )
{
  if ( mapLayer->type() != QgsMapLayer::VectorLayer )
  {
    return;
  }

  QgsVectorLayer* vectorLayer = qobject_cast<QgsVectorLayer*>( mapLayer );
  if ( !vectorLayer )
  {
    return;
  }

  for ( int idx = 0; idx < vectorLayer->pendingFields().count(); ++idx )
  {
    if ( vectorLayer->editType( idx ) != QgsVectorLayer::EditorWidgetV2 )
    {
      continue;
    }

    QDomNodeList editTypeNodes = layerElem.namedItem( "edittypes" ).childNodes();

    for ( int i = 0; i < editTypeNodes.size(); i++ )
    {
      QDomNode editTypeNode = editTypeNodes.at( i );
      QDomElement editTypeElement = editTypeNode.toElement();

      QString name = editTypeElement.attribute( "name" );

      if ( vectorLayer->fieldNameIndex( name ) != idx )
        continue;

      QgsVectorLayer::EditType editType =
        ( QgsVectorLayer::EditType ) editTypeElement.attribute( "type" ).toInt();

      if ( editType != QgsVectorLayer::EditorWidgetV2 )
        continue;

      QString ewv2Type = editTypeElement.attribute( "widgetv2type" );

      if ( mWidgetFactories.contains( ewv2Type ) )
      {
        vectorLayer->setEditorWidgetV2( idx, ewv2Type );
        QDomElement ewv2CfgElem = editTypeElement.namedItem( "widgetv2config" ).toElement();

        if ( !ewv2CfgElem.isNull() )
        {
          QMap<QString, QVariant> cfg = mWidgetFactories[ewv2Type]->readConfig( ewv2CfgElem, vectorLayer, idx );
          vectorLayer->setEditorWidgetV2Config( idx, cfg );
        }
      }
      else
      {
        QgsMessageLog::logMessage( tr( "Unknown attribute editor widget '%1'" ).arg( ewv2Type ) );
      }
    }
  }
}

void QgsEditorWidgetRegistry::writeMapLayer( QgsMapLayer* mapLayer, QDomElement& layerElem, QDomDocument& doc )
{
  if ( mapLayer->type() != QgsMapLayer::VectorLayer )
  {
    return;
  }

  QgsVectorLayer* vectorLayer = qobject_cast<QgsVectorLayer*>( mapLayer );
  if ( !vectorLayer )
  {
    return;
  }

  for ( int idx = 0; idx < vectorLayer->pendingFields().count(); ++idx )
  {
    if ( vectorLayer->editType( idx ) != QgsVectorLayer::EditorWidgetV2 )
    {
      continue;
    }

    const QString& widgetType = vectorLayer->editorWidgetV2( idx );
    if ( !mWidgetFactories.contains( widgetType ) )
    {
      QgsMessageLog::logMessage( tr( "Could not save unknown editor widget type '%1'." ).arg( widgetType ) );
      continue;
    }

    QDomNodeList editTypeNodes = layerElem.namedItem( "edittypes" ).childNodes();

    for ( int i = 0; i < editTypeNodes.size(); i++ )
    {
      QDomElement editTypeElement = editTypeNodes.at( i ).toElement();

      QString name = editTypeElement.attribute( "name" );

      if ( vectorLayer->fieldNameIndex( name ) != idx )
        continue;

      QgsVectorLayer::EditType editType =
        ( QgsVectorLayer::EditType ) editTypeElement.attribute( "type" ).toInt();

      if ( editType != QgsVectorLayer::EditorWidgetV2 )
        continue;

      editTypeElement.setAttribute( "widgetv2type", widgetType );

      if ( mWidgetFactories.contains( widgetType ) )
      {
        QDomElement ewv2CfgElem = doc.createElement( "widgetv2config" );

        mWidgetFactories[widgetType]->writeConfig( vectorLayer->editorWidgetV2Config( idx ), ewv2CfgElem, doc, vectorLayer, idx );

        editTypeElement.appendChild( ewv2CfgElem );
      }
      else
      {
        QgsMessageLog::logMessage( tr( "Unknown attribute editor widget '%1'" ).arg( widgetType ) );
      }
    }
  }
}
