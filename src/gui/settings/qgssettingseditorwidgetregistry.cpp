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

#include "qgis.h"
#include "qgslogger.h"
#include "qgssettingseditorwidgetwrapper.h"
#include "qgssettingseditorwidgetwrapperimpl.h"
#include "qgssettingsenumflageditorwidgetwrapper.h"
#include "qgssettingsentry.h"

#if defined( HAVE_QTSERIALPORT )
#include <QSerialPort>
#endif


QgsSettingsEditorWidgetRegistry::QgsSettingsEditorWidgetRegistry()
{
  addWrapper( new QgsSettingsStringLineEditWrapper() );
  addWrapper( new QgsSettingsBoolCheckBoxWrapper() );
  addWrapper( new QgsSettingsIntegerSpinBoxWrapper() );
  addWrapper( new QgsSettingsDoubleSpinBoxWrapper() );
  addWrapper( new QgsSettingsColorButtonWrapper() );

  // enum
#if defined( HAVE_QTSERIALPORT )
  addWrapper( new QgsSettingsEnumEditorWidgetWrapper<QSerialPort::DataBits>() );
  addWrapper( new QgsSettingsEnumEditorWidgetWrapper<QSerialPort::FlowControl>() );
  addWrapper( new QgsSettingsEnumEditorWidgetWrapper<QSerialPort::Parity>() );
  addWrapper( new QgsSettingsEnumEditorWidgetWrapper<QSerialPort::StopBits>() );
#endif
  addWrapper( new QgsSettingsEnumEditorWidgetWrapper<Qgis::CaptureTechnique>() );
  addWrapper( new QgsSettingsEnumEditorWidgetWrapper<Qgis::DpiMode>() );
  addWrapper( new QgsSettingsEnumEditorWidgetWrapper<Qgis::EndCapStyle>() );
  addWrapper( new QgsSettingsEnumEditorWidgetWrapper<Qgis::GpsConnectionType>() );
  addWrapper( new QgsSettingsEnumEditorWidgetWrapper<Qgis::GpsInformationComponent>() );
  addWrapper( new QgsSettingsEnumEditorWidgetWrapper<Qgis::JoinStyle>() );
  addWrapper( new QgsSettingsEnumEditorWidgetWrapper<Qgis::MapRecenteringMode>() );
  addWrapper( new QgsSettingsEnumEditorWidgetWrapper<Qgis::MapToolUnit>() );
  addWrapper( new QgsSettingsEnumEditorWidgetWrapper<Qgis::SnappingMode>() );
  addWrapper( new QgsSettingsEnumEditorWidgetWrapper<Qgis::SnappingType>() );
  addWrapper( new QgsSettingsEnumEditorWidgetWrapper<Qgis::TilePixelRatio>() );

  addWrapper( new QgsSettingsEnumEditorWidgetWrapper<Qgis::VectorSimplificationAlgorithm>() );
  addWrapper( new QgsSettingsEnumEditorWidgetWrapper<Qgis::VectorRenderingSimplificationFlags>() );
  addWrapper( new QgsSettingsEnumEditorWidgetWrapper<Qt::TimeSpec>() );

  // flags
  addWrapper( new QgsSettingsFlagsEditorWidgetWrapper<Qgis::GpsInformationComponent, Qgis::GpsInformationComponents>() );
}

QgsSettingsEditorWidgetRegistry::~QgsSettingsEditorWidgetRegistry()
{
  qDeleteAll( mWrappers );
}

bool QgsSettingsEditorWidgetRegistry::addWrapper( QgsSettingsEditorWidgetWrapper *wrapper )
{
  if ( mWrappers.contains( wrapper->id() ) )
  {
    QgsDebugMsgLevel( QString( "Settings editor widget registry already contains a wrapper with id '%1'" ).arg( wrapper->id() ), 2 );
    delete wrapper;
    return false;
  }

  mWrappers.insert( wrapper->id(), wrapper );
  return true;
}

void QgsSettingsEditorWidgetRegistry::addWrapperForSetting( QgsSettingsEditorWidgetWrapper *wrapper, const QgsSettingsEntryBase *setting )
{
  mSpecificWrappers.insert( setting, wrapper );
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
  if ( mSpecificWrappers.contains( setting ) )
  {
    return mSpecificWrappers.value( setting )->createEditor( setting, dynamicKeyPartList, parent );
  }
  QgsSettingsEditorWidgetWrapper *eww = createWrapper( setting->typeId(), parent );
  if ( eww )
    return eww->createEditor( setting, dynamicKeyPartList, parent );
  else
    return nullptr;
}
