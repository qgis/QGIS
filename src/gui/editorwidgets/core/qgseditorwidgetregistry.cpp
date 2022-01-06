/***************************************************************************
    qgseditorwidgetregistry.cpp
     --------------------------------------
    Date                 : 24.4.2013
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

#include "qgseditorwidgetregistry.h"

#include "qgsattributeeditorcontext.h"
#include "qgsmessagelog.h"
#include "qgsproject.h"
#include "qgsvectorlayer.h"
#include "qgseditorwidgetwrapper.h"
#include "qgssearchwidgetwrapper.h"
#include "qgsapplication.h"

// Editors
#include "qgsbinarywidgetfactory.h"
#include "qgsclassificationwidgetwrapperfactory.h"
#include "qgscheckboxwidgetfactory.h"
#include "qgscolorwidgetfactory.h"
#include "qgsdatetimeeditfactory.h"
#include "qgsenumerationwidgetfactory.h"
#include "qgsexternalresourcewidgetfactory.h"
#include "qgshiddenwidgetfactory.h"
#include "qgskeyvaluewidgetfactory.h"
#include "qgsjsoneditwidgetfactory.h"
#include "qgslistwidgetfactory.h"
#include "qgsrangewidgetfactory.h"
#include "qgsrelationreferencefactory.h"
#include "qgstexteditwidgetfactory.h"
#include "qgsuniquevaluewidgetfactory.h"
#include "qgsuuidwidgetfactory.h"
#include "qgsvaluemapwidgetfactory.h"
#include "qgsvaluerelationwidgetfactory.h"

QgsEditorWidgetRegistry::QgsEditorWidgetRegistry()
{
  mFallbackWidgetFactory.reset( new QgsTextEditWidgetFactory( tr( "Text Edit" ) ) );
}

void QgsEditorWidgetRegistry::initEditors( QgsMapCanvas *mapCanvas, QgsMessageBar *messageBar )
{
  registerWidget( QStringLiteral( "TextEdit" ), new QgsTextEditWidgetFactory( tr( "Text Edit" ) ) );
  registerWidget( QStringLiteral( "Classification" ), new QgsClassificationWidgetWrapperFactory( tr( "Classification" ) ) );
  registerWidget( QStringLiteral( "Range" ), new QgsRangeWidgetFactory( tr( "Range" ) ) );
  registerWidget( QStringLiteral( "UniqueValues" ), new QgsUniqueValueWidgetFactory( tr( "Unique Values" ) ) );
  registerWidget( QStringLiteral( "ValueMap" ), new QgsValueMapWidgetFactory( tr( "Value Map" ) ) );
  registerWidget( QStringLiteral( "Enumeration" ), new QgsEnumerationWidgetFactory( tr( "Enumeration" ) ) );
  registerWidget( QStringLiteral( "Hidden" ), new QgsHiddenWidgetFactory( tr( "Hidden" ) ) );
  registerWidget( QStringLiteral( "CheckBox" ), new QgsCheckboxWidgetFactory( tr( "Checkbox" ) ) );
  registerWidget( QStringLiteral( "ValueRelation" ), new QgsValueRelationWidgetFactory( tr( "Value Relation" ) ) );
  registerWidget( QStringLiteral( "UuidGenerator" ), new QgsUuidWidgetFactory( tr( "Uuid Generator" ) ) );
  registerWidget( QStringLiteral( "Color" ), new QgsColorWidgetFactory( tr( "Color" ) ) );
  registerWidget( QStringLiteral( "RelationReference" ), new QgsRelationReferenceFactory( tr( "Relation Reference" ), mapCanvas, messageBar ) );
  registerWidget( QStringLiteral( "DateTime" ), new QgsDateTimeEditFactory( tr( "Date/Time" ) ) );
  registerWidget( QStringLiteral( "ExternalResource" ), new QgsExternalResourceWidgetFactory( tr( "Attachment" ), messageBar ) );
  registerWidget( QStringLiteral( "KeyValue" ), new QgsKeyValueWidgetFactory( tr( "Key/Value" ) ) );
  registerWidget( QStringLiteral( "List" ), new QgsListWidgetFactory( tr( "List" ) ) );
  registerWidget( QStringLiteral( "Binary" ), new QgsBinaryWidgetFactory( tr( "Binary (BLOB)" ), messageBar ) );
  registerWidget( QStringLiteral( "JsonEdit" ), new QgsJsonEditWidgetFactory( tr( "Json View" ) ) );
}

QgsEditorWidgetRegistry::~QgsEditorWidgetRegistry()
{
  qDeleteAll( mWidgetFactories );
}

QgsEditorWidgetSetup QgsEditorWidgetRegistry::findBest( const QgsVectorLayer *vl, const QString &fieldName ) const
{
  const QgsFields fields = vl->fields();
  const int index = fields.indexOf( fieldName );

  if ( index > -1 )
  {
    QgsEditorWidgetSetup setup = fields.at( index ).editorWidgetSetup();
    if ( !setup.isNull() )
      return setup;
  }
  return mAutoConf.editorWidgetSetup( vl, fieldName );
}

QgsEditorWidgetWrapper *QgsEditorWidgetRegistry::create( QgsVectorLayer *vl, int fieldIdx, QWidget *editor, QWidget *parent, const QgsAttributeEditorContext &context )
{
  const QString fieldName = vl->fields().field( fieldIdx ).name();
  const QgsEditorWidgetSetup setup = findBest( vl, fieldName );
  return create( setup.type(), vl, fieldIdx, setup.config(), editor, parent, context );
}

QgsEditorWidgetWrapper *QgsEditorWidgetRegistry::create( const QString &widgetId, QgsVectorLayer *vl, int fieldIdx, const QVariantMap &config, QWidget *editor, QWidget *parent, const QgsAttributeEditorContext &context )
{
  if ( mWidgetFactories.contains( widgetId ) )
  {
    QgsEditorWidgetWrapper *ww = mWidgetFactories[widgetId]->create( vl, fieldIdx, editor, parent );

    if ( ww )
    {
      ww->setConfig( config );
      ww->setContext( context );
      // Make sure that there is a widget created at this point
      // so setValue() et al won't crash
      ww->widget();

      // If we tried to set a widget which is not supported by this wrapper
      if ( !ww->valid() )
      {
        delete ww;
        const QString wid = findSuitableWrapper( editor, QStringLiteral( "TextEdit" ) );
        ww = mWidgetFactories[wid]->create( vl, fieldIdx, editor, parent );
        ww->setConfig( config );
        ww->setContext( context );
      }

      return ww;
    }
  }

  return nullptr;
}

QgsSearchWidgetWrapper *QgsEditorWidgetRegistry::createSearchWidget( const QString &widgetId, QgsVectorLayer *vl, int fieldIdx, const QVariantMap &config, QWidget *parent, const QgsAttributeEditorContext &context )
{
  if ( mWidgetFactories.contains( widgetId ) )
  {
    QgsSearchWidgetWrapper *ww = mWidgetFactories[widgetId]->createSearchWidget( vl, fieldIdx, parent );

    if ( ww )
    {
      ww->setConfig( config );
      ww->setContext( context );
      // Make sure that there is a widget created at this point
      // so setValue() et al won't crash
      ww->widget();
      ww->clearWidget();
      return ww;
    }
  }
  return nullptr;
}

QgsEditorConfigWidget *QgsEditorWidgetRegistry::createConfigWidget( const QString &widgetId, QgsVectorLayer *vl, int fieldIdx, QWidget *parent )
{
  if ( mWidgetFactories.contains( widgetId ) )
  {
    return mWidgetFactories[widgetId]->configWidget( vl, fieldIdx, parent );
  }
  return nullptr;
}

QString QgsEditorWidgetRegistry::name( const QString &widgetId )
{
  if ( mWidgetFactories.contains( widgetId ) )
  {
    return mWidgetFactories[widgetId]->name();
  }

  return QString();
}

QMap<QString, QgsEditorWidgetFactory *> QgsEditorWidgetRegistry::factories()
{
  return mWidgetFactories;
}

QgsEditorWidgetFactory *QgsEditorWidgetRegistry::factory( const QString &widgetId )
{
  return mWidgetFactories.value( widgetId, mFallbackWidgetFactory.get() );
}

bool QgsEditorWidgetRegistry::registerWidget( const QString &widgetId, QgsEditorWidgetFactory *widgetFactory )
{
  if ( !widgetFactory )
  {
    QgsMessageLog::logMessage( tr( "QgsEditorWidgetRegistry: Factory not valid." ) );
    return false;
  }
  else if ( mWidgetFactories.contains( widgetId ) )
  {
    QgsMessageLog::logMessage( tr( "QgsEditorWidgetRegistry: Factory with id %1 already registered." ).arg( widgetId ) );
    return false;
  }
  else
  {
    mWidgetFactories.insert( widgetId, widgetFactory );

    // Use this factory as default where it provides the highest priority
    const QHash<const char *, int> types = widgetFactory->supportedWidgetTypes();
    QHash<const char *, int>::ConstIterator it;
    it = types.constBegin();

    for ( ; it != types.constEnd(); ++it )
    {
      if ( it.value() > mFactoriesByType[it.key()].first )
      {
        mFactoriesByType[it.key()] = qMakePair( it.value(), widgetId );
      }
    }

    return true;
  }
}

QString QgsEditorWidgetRegistry::findSuitableWrapper( QWidget *editor, const QString &defaultWidget )
{
  QMap<const char *, QPair<int, QString> >::ConstIterator it;

  QString widgetid;

  // Editor can be null
  if ( editor )
  {
    int weight = 0;

    it = mFactoriesByType.constBegin();
    for ( ; it != mFactoriesByType.constEnd(); ++it )
    {
      if ( QWidget::staticMetaObject.className() == it.key() )
      {
        // if it's a perfect match: return it directly
        return it.value().second;
      }
      else if ( editor->inherits( it.key() ) )
      {
        // if it's a subclass, continue evaluating, maybe we find a more-specific or one with more weight
        if ( it.value().first > weight )
        {
          weight = it.value().first;
          widgetid = it.value().second;
        }
      }
    }
  }

  if ( widgetid.isNull() )
    widgetid = defaultWidget;
  return widgetid;
}
