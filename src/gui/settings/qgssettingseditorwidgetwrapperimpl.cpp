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
#include "moc_qgssettingseditorwidgetwrapperimpl.cpp"
#include "qgslogger.h"
#include "qgssettingsentryimpl.h"
#include "qgscolorbutton.h"

#include <QLineEdit>
#include <QCheckBox>


// *******
// String with line edit (= default)
// *******

QString QgsSettingsStringLineEditWrapper::id() const
{
  return QString::fromUtf8( sSettingsTypeMetaEnum.valueToKey( static_cast<int>( Qgis::SettingsType::String ) ) );
}

bool QgsSettingsStringLineEditWrapper::setWidgetValue( const QString &value ) const
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

void QgsSettingsStringLineEditWrapper::enableAutomaticUpdatePrivate()
{
  QObject::connect( this->mEditor, &QLineEdit::textChanged, this, [=]( const QString &text ) {
    this->mSetting->setValue( text, this->mDynamicKeyPartList );
  } );
}

bool QgsSettingsStringLineEditWrapper::setSettingFromWidget() const
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

QString QgsSettingsStringLineEditWrapper::valueFromWidget() const
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
// String with combo box
// *******

QString QgsSettingsStringComboBoxWrapper::id() const
{
  return QString::fromUtf8( sSettingsTypeMetaEnum.valueToKey( static_cast<int>( Qgis::SettingsType::String ) ) );
}

bool QgsSettingsStringComboBoxWrapper::setWidgetValue( const QString &value ) const
{
  if ( mEditor )
  {
    int idx = mMode == Mode::Data ? mEditor->findData( value ) : mEditor->findText( value );
    if ( idx >= 0 )
    {
      mEditor->setCurrentIndex( idx );
      return true;
    }
    else
    {
      return false;
    }
  }
  else
  {
    QgsDebugError( QStringLiteral( "Settings editor not set for %1" ).arg( mSetting->definitionKey() ) );
  }
  return false;
}

void QgsSettingsStringComboBoxWrapper::enableAutomaticUpdatePrivate()
{
  QObject::connect( mEditor, &QComboBox::currentTextChanged, this, [=]( const QString &currentText ) {
    QString textValue = currentText;
    if ( mMode == Mode::Data )
      textValue = mEditor->currentData().toString();
    mSetting->setValue( textValue, mDynamicKeyPartList );
  } );
}

bool QgsSettingsStringComboBoxWrapper::setSettingFromWidget() const
{
  if ( mEditor )
  {
    mSetting->setValue( valueFromWidget(), mDynamicKeyPartList );
    return true;
  }
  else
  {
    QgsDebugError( QStringLiteral( "Settings editor not set for %1" ).arg( mSetting->definitionKey() ) );
  }
  return false;
}

QString QgsSettingsStringComboBoxWrapper::valueFromWidget() const
{
  if ( mEditor )
  {
    return mMode == Mode::Data ? mEditor->currentData().toString() : mEditor->currentText();
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

QString QgsSettingsBoolCheckBoxWrapper::id() const
{
  return QString::fromUtf8( sSettingsTypeMetaEnum.valueToKey( static_cast<int>( Qgis::SettingsType::Bool ) ) );
}

bool QgsSettingsBoolCheckBoxWrapper::setWidgetValue( const bool &value ) const
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

void QgsSettingsBoolCheckBoxWrapper::enableAutomaticUpdatePrivate()
{
  QObject::connect( this->mEditor, &QCheckBox::clicked, this, [=]( bool checked ) {
    this->mSetting->setValue( checked, this->mDynamicKeyPartList );
  } );
}

bool QgsSettingsBoolCheckBoxWrapper::setSettingFromWidget() const
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

bool QgsSettingsBoolCheckBoxWrapper::valueFromWidget() const
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

QString QgsSettingsIntegerSpinBoxWrapper::id() const
{
  return QString::fromUtf8( sSettingsTypeMetaEnum.valueToKey( static_cast<int>( Qgis::SettingsType::Integer ) ) );
}

bool QgsSettingsIntegerSpinBoxWrapper::setWidgetValue( const int &value ) const
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

void QgsSettingsIntegerSpinBoxWrapper::enableAutomaticUpdatePrivate()
{
  QObject::connect( this->mEditor, qOverload<int>( &QSpinBox::valueChanged ), this, [=]( int value ) {
    this->mSetting->setValue( value, this->mDynamicKeyPartList );
  } );
}

bool QgsSettingsIntegerSpinBoxWrapper::setSettingFromWidget() const
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

int QgsSettingsIntegerSpinBoxWrapper::valueFromWidget() const
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

QString QgsSettingsDoubleSpinBoxWrapper::id() const
{
  return QString::fromUtf8( sSettingsTypeMetaEnum.valueToKey( static_cast<int>( Qgis::SettingsType::Double ) ) );
}

bool QgsSettingsDoubleSpinBoxWrapper::setWidgetValue( const double &value ) const
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

void QgsSettingsDoubleSpinBoxWrapper::enableAutomaticUpdatePrivate()
{
  QObject::connect( this->mEditor, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, [=]( double value ) {
    this->mSetting->setValue( value, this->mDynamicKeyPartList );
  } );
}

bool QgsSettingsDoubleSpinBoxWrapper::setSettingFromWidget() const
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

double QgsSettingsDoubleSpinBoxWrapper::valueFromWidget() const
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

QString QgsSettingsColorButtonWrapper::id() const
{
  return QString::fromUtf8( sSettingsTypeMetaEnum.valueToKey( static_cast<int>( Qgis::SettingsType::Color ) ) );
}

bool QgsSettingsColorButtonWrapper::setWidgetValue( const QColor &value ) const
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

void QgsSettingsColorButtonWrapper::configureEditorPrivateImplementation()
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

void QgsSettingsColorButtonWrapper::enableAutomaticUpdatePrivate()
{
  QObject::connect( this->mEditor, &QgsColorButton::colorChanged, this, [=]( const QColor &color ) {
    this->mSetting->setValue( color, this->mDynamicKeyPartList );
  } );
}

bool QgsSettingsColorButtonWrapper::setSettingFromWidget() const
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

QColor QgsSettingsColorButtonWrapper::valueFromWidget() const
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
