/***************************************************************************
                         qgsrelationwidgetregistry.h
                         ----------------------
    begin                : October 2020
    copyright            : (C) 2020 by Ivan Ivanov
    email                : ivan@opengis.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsrelationwidgetregistry.h"
#include "qgsrelationwidget.h"
#include "qgsrelationconfigwidget.h"
#include "qgsbasicrelationwidgetfactory.h"

QgsRelationWidgetRegistry::QgsRelationWidgetRegistry()
{
  addRelationWidget( new QgsBasicRelationWidgetFactory() );
}

QgsRelationWidgetRegistry::~QgsRelationWidgetRegistry()
{
  qDeleteAll( mRelationWidgetFactories );
  mRelationWidgetFactories.clear();
}

void QgsRelationWidgetRegistry::addRelationWidget( QgsRelationWidgetFactory *widgetFactory )
{
  if ( !widgetFactory )
    return;

  if ( mRelationWidgetFactories.contains( widgetFactory->type() ) )
    return;

  mRelationWidgetFactories.insert( widgetFactory->type(), widgetFactory );
}

void QgsRelationWidgetRegistry::removeRelationWidget( const QString &widgetType )
{
  // protect the basic widget from removing, so the user has at least one relation widget type
  if ( dynamic_cast<QgsBasicRelationWidgetFactory *>( mRelationWidgetFactories.value( widgetType ) ) )
    return;

  mRelationWidgetFactories.remove( widgetType );
}

QStringList QgsRelationWidgetRegistry::relationWidgetNames()
{
  return mRelationWidgetFactories.keys();
}

QMap<QString, QgsRelationWidgetFactory *> QgsRelationWidgetRegistry::factories() const
{
  return mRelationWidgetFactories;
}

QgsRelationWidget *QgsRelationWidgetRegistry::create( const QString &widgetType, const QVariantMap &config, QWidget *parent ) const
{
  if ( ! mRelationWidgetFactories.contains( widgetType ) )
    return nullptr;

  return mRelationWidgetFactories.value( widgetType )->create( config, parent );
}

QgsRelationConfigWidget *QgsRelationWidgetRegistry::createConfigWidget( const QString &widgetType, const QgsRelation &relation, QWidget *parent ) const
{
  if ( ! mRelationWidgetFactories.contains( widgetType ) )
    return nullptr;

  return mRelationWidgetFactories.value( widgetType )->configWidget( relation, parent );
}
