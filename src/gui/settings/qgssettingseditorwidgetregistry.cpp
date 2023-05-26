/***************************************************************************
    qgssettingseditorwidgetregistry.cpp
    ---------------------
    begin                : April 2023
    copyright            : (C) 2023 by Denis Rouzaud
    email                : denis@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgssettingseditorwidgetregistry.h"

#include "qgslogger.h"
#include "qgssettingseditorwidgetwrapper.h"
#include "qgssettingseditorwidgetwrapperimpl.h"
#include "qgssettingsentry.h"

QgsSettingsEditorWidgetRegistry::QgsSettingsEditorWidgetRegistry()
{
  addWrapper( new QgsSettingsStringEditorWidgetWrapper() );
  addWrapper( new QgsSettingsBoolEditorWidgetWrapper() );
  addWrapper( new QgsSettingsIntegerEditorWidgetWrapper() );
  addWrapper( new QgsSettingsDoubleEditorWidgetWrapper() );
  addWrapper( new QgsSettingsColorEditorWidgetWrapper() );

}

QgsSettingsEditorWidgetRegistry::~QgsSettingsEditorWidgetRegistry()
{
  qDeleteAll( mWrappers );
}

bool QgsSettingsEditorWidgetRegistry::addWrapper( QgsSettingsEditorWidgetWrapper *wrapper )
{
  if ( mWrappers.contains( wrapper->id() ) )
    return false;

  mWrappers.insert( wrapper->id(), wrapper );
  return true;
}

QgsSettingsEditorWidgetWrapper *QgsSettingsEditorWidgetRegistry::createWrapper( const QString &id, QObject *parent ) const
{
  QgsSettingsEditorWidgetWrapper *wrapper = mWrappers.value( id );
  if ( wrapper )
  {
    return wrapper->createWrapper( parent );
  }
  else
  {
    QgsDebugError( QStringLiteral( "Setting factory was not found for '%1', returning the default string factory" ).arg( id ) );
    return nullptr;
  }
}

QWidget *QgsSettingsEditorWidgetRegistry::createEditor( const QgsSettingsEntryBase *setting, const QStringList &dynamicKeyPartList, QWidget *parent ) const
{
  QgsSettingsEditorWidgetWrapper *eww = createWrapper( setting->typeId(), parent );
  if ( eww )
    return eww->createEditor( setting, dynamicKeyPartList, parent );
  else
    return nullptr;
}
