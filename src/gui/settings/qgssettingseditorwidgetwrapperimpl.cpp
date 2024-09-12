/***************************************************************************
  qgssettingseditorwidgetwrapperimpl.cpp
  --------------------------------------
  Date                 : February 2023
  Copyright            : (C) 2023 by Denis Rouzaud
  Email                : denis@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgssettingseditorwidgetwrapperimpl.h"
#include "qgslogger.h"
#include "qgssettingsentryimpl.h"
#include "qgscolorbutton.h"

#include <QLineEdit>
#include <QCheckBox>


// *******
// String
// *******

QString QgsSettingsStringEditorWidgetWrapper::id() const
{
  return QString::fromUtf8( sSettingsTypeMetaEnum.valueToKey( static_cast<int>( Qgis::SettingsType::String ) ) );
}

bool QgsSettingsStringEditorWidgetWrapper::setWidgetValue( const QString &value ) const
{
  if ( mEditor )
  {
    mEditor->setText( value );
    return true;
  }
  else
  {
    QgsDebugError( QStringLiteral( "Settings editor not set for %1" ).arg( mSetting->definitionKey() ) );
  }
  return false;
}

void QgsSettingsStringEditorWidgetWrapper::enableAutomaticUpdatePrivate()
{
  QObject::connect( this->mEditor, &QLineEdit::textChanged, this, [ = ]( const QString & text )
  {
    this->mSetting->setValue( text, this->mDynamicKeyPartList );
  } );
}

bool QgsSettingsStringEditorWidgetWrapper::setSettingFromWidget() const
{
  if ( mEditor )
  {
    mSetting->setValue( mEditor->text(), mDynamicKeyPartList );
    return true;
  }
  else
  {
    QgsDebugError( QStringLiteral( "Settings editor not set for %1" ).arg( mSetting->definitionKey() ) );
  }
  return false;
}

QString QgsSettingsStringEditorWidgetWrapper::valueFromWidget() const
{
  if ( mEditor )
  {
    return mEditor->text();
  }
  else
  {
    QgsDebugError( QString( "editor is not set, returning a non-existing value" ) );
  }
  return QString();
}

// *******
// Boolean
// *******

QString QgsSettingsBoolEditorWidgetWrapper::id() const
{
  return QString::fromUtf8( sSettingsTypeMetaEnum.valueToKey( static_cast<int>( Qgis::SettingsType::Bool ) ) );
}

bool QgsSettingsBoolEditorWidgetWrapper::setWidgetValue( const bool &value ) const
{
  if ( mEditor )
  {
    mEditor->setChecked( value );
    return true;
  }
  else
  {
    QgsDebugError( QStringLiteral( "Settings editor not set for %1" ).arg( mSetting->definitionKey() ) );
  }
  return false;
}

void QgsSettingsBoolEditorWidgetWrapper::enableAutomaticUpdatePrivate()
{
  QObject::connect( this->mEditor, &QCheckBox::clicked, this, [ = ]( bool checked )
  {
    this->mSetting->setValue( checked, this->mDynamicKeyPartList );
  } );
}

bool QgsSettingsBoolEditorWidgetWrapper::setSettingFromWidget() const
{
  if ( mEditor )
  {
    mSetting->setValue( mEditor->isChecked(), mDynamicKeyPartList );
    return true;
  }
  else
  {
    QgsDebugError( QStringLiteral( "Settings editor not set for %1" ).arg( mSetting->definitionKey() ) );
  }
  return false;
}

bool QgsSettingsBoolEditorWidgetWrapper::valueFromWidget() const
{

  if ( mEditor )
  {
    return mEditor->isChecked();
  }
  else
  {
    QgsDebugError( QString( "editor is not set, returning a non-existing value" ) );
  }
  return false;
}


// *******
// Integer
// *******

QString QgsSettingsIntegerEditorWidgetWrapper::id() const
{
  return QString::fromUtf8( sSettingsTypeMetaEnum.valueToKey( static_cast<int>( Qgis::SettingsType::Integer ) ) );
}

bool QgsSettingsIntegerEditorWidgetWrapper::setWidgetValue( const int &value ) const
{
  if ( mEditor )
  {
    mEditor->setValue( value );
    return true;
  }
  else
  {
    QgsDebugError( QStringLiteral( "Settings editor not set for %1" ).arg( mSetting->definitionKey() ) );
  }
  return false;
}

void QgsSettingsIntegerEditorWidgetWrapper::enableAutomaticUpdatePrivate()
{
  QObject::connect( this->mEditor, qOverload<int>( &QSpinBox::valueChanged ), this, [ = ]( int value )
  {
    this->mSetting->setValue( value, this->mDynamicKeyPartList );
  } );
}

bool QgsSettingsIntegerEditorWidgetWrapper::setSettingFromWidget() const
{
  if ( mEditor )
  {
    mSetting->setValue( mEditor->value(), mDynamicKeyPartList );
    return true;
  }
  else
  {
    QgsDebugError( QStringLiteral( "Settings editor not set for %1" ).arg( mSetting->definitionKey() ) );
  }
  return false;
}

int QgsSettingsIntegerEditorWidgetWrapper::valueFromWidget() const
{
  if ( mEditor )
  {
    return mEditor->value();
  }
  else
  {
    QgsDebugError( QString( "editor is not set, returning a non-existing value" ) );
  }
  return std::numeric_limits<int>::quiet_NaN();
}



// *******
// Double
// *******

QString QgsSettingsDoubleEditorWidgetWrapper::id() const
{
  return QString::fromUtf8( sSettingsTypeMetaEnum.valueToKey( static_cast<int>( Qgis::SettingsType::Double ) ) );
}

bool QgsSettingsDoubleEditorWidgetWrapper::setWidgetValue( const double &value ) const
{
  if ( mEditor )
  {
    mEditor->setValue( value );
    return true;
  }
  else
  {
    QgsDebugError( QStringLiteral( "Settings editor not set for %1" ).arg( mSetting->definitionKey() ) );
  }
  return false;
}

void QgsSettingsDoubleEditorWidgetWrapper::enableAutomaticUpdatePrivate()
{
  QObject::connect( this->mEditor, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, [ = ]( double value )
  {
    this->mSetting->setValue( value, this->mDynamicKeyPartList );
  } );
}

bool QgsSettingsDoubleEditorWidgetWrapper::setSettingFromWidget() const
{
  if ( mEditor )
  {
    mSetting->setValue( mEditor->value(), mDynamicKeyPartList );
    return true;
  }
  else
  {
    QgsDebugError( QStringLiteral( "Settings editor not set for %1" ).arg( mSetting->definitionKey() ) );
  }
  return false;
}

double QgsSettingsDoubleEditorWidgetWrapper::valueFromWidget() const
{
  if ( mEditor )
  {
    return mEditor->value();
  }
  else
  {
    QgsDebugError( QString( "editor is not set, returning a non-existing value" ) );
  }
  return std::numeric_limits<double>::quiet_NaN();
}

// *******
// Color
// *******

QString QgsSettingsColorEditorWidgetWrapper::id() const
{
  return QString::fromUtf8( sSettingsTypeMetaEnum.valueToKey( static_cast<int>( Qgis::SettingsType::Color ) ) );
}

bool QgsSettingsColorEditorWidgetWrapper::setWidgetValue( const QColor &value ) const
{
  if ( mEditor )
  {
    mEditor->setColor( value );
    return true;
  }
  else
  {
    QgsDebugError( QStringLiteral( "Settings editor not set for %1" ).arg( mSetting->definitionKey() ) );
  }
  return false;
}

void QgsSettingsColorEditorWidgetWrapper::configureEditorPrivateImplementation()
{
  if ( mEditor )
  {
    mEditor->setAllowOpacity( mSetting->allowAlpha() );
  }
  else
  {
    QgsDebugError( QStringLiteral( "Settings editor not set for %1" ).arg( mSetting->definitionKey() ) );
  }
}

void QgsSettingsColorEditorWidgetWrapper::enableAutomaticUpdatePrivate()
{
  QObject::connect( this->mEditor, &QgsColorButton::colorChanged, this, [ = ]( const QColor & color )
  {
    this->mSetting->setValue( color, this->mDynamicKeyPartList );
  } );
}

bool QgsSettingsColorEditorWidgetWrapper::setSettingFromWidget() const
{
  if ( mEditor )
  {
    mSetting->setValue( mEditor->color(), mDynamicKeyPartList );
    return true;
  }
  else
  {
    QgsDebugError( QStringLiteral( "Settings editor not set for %1" ).arg( mSetting->definitionKey() ) );
  }
  return false;
}

QColor QgsSettingsColorEditorWidgetWrapper::valueFromWidget() const
{
  if ( mEditor )
  {
    return mEditor->color();
  }
  else
  {
    QgsDebugError( QString( "editor is not set, returning a non-existing value" ) );
  }
  return QColor();
}

// *******
// StringList
// *******

//QString QgsSettingsStringListEditorWidgetWrapper::id() const
//{
//  return QString::fromUtf8( sSettingsTypeMetaEnum.valueToKey( static_cast<int>( Qgis::SettingsType::StringList ) ) );
//}

//bool QgsSettingsStringListEditorWidgetWrapper::setWidgetFromSetting() const
//{
//  if ( mEditor )
//  {
//    mEditor->setValue( mSetting->value( mDynamicKeyPartList ) );
//    return true;
//  }
//  else
//  {
//    QgsDebugError( QStringLiteral( "Settings editor not set for %1" ).arg( mSetting->definitionKey() ) );
//  }
//  return false;
//}

//bool QgsSettingsStringListEditorWidgetWrapper::setSettingFromWidget() const
//{
//  if ( mEditor )
//  {
//    mSetting->setValue( mEditor->value(), mDynamicKeyPartList );
//    return true;
//  }
//  else
//  {
//    QgsDebugError( QStringLiteral( "Settings editor not set for %1" ).arg( mSetting->definitionKey() ) );
//  }
//  return false;
//}

//QVariant QgsSettingsStringListEditorWidgetWrapper::valueFromWidget() const
//{
//  if ( mEditor )
//  {
//    return mEditor->value();
//  }
//  else
//  {
//    QgsDebugError(QString("editor is not set, returning a non-existing value"));
//  }
//  return QStringList();
//}
