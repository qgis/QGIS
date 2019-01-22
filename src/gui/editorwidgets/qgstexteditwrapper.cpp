/***************************************************************************
    qgstexteditwrapper.cpp
     --------------------------------------
    Date                 : 5.1.2014
    Copyright            : (C) 2014 Matthias Kuhn
    Email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstexteditwrapper.h"

#include "qgsfields.h"
#include "qgsfieldvalidator.h"
#include "qgsfilterlineedit.h"
#include "qgsapplication.h"

#include <QSettings>

QgsTextEditWrapper::QgsTextEditWrapper( QgsVectorLayer *layer, int fieldIdx, QWidget *editor, QWidget *parent )
  : QgsEditorWidgetWrapper( layer, fieldIdx, editor, parent )

{
}

QVariant QgsTextEditWrapper::value() const
{
  QString v;

  if ( mTextEdit )
  {
    if ( config( QStringLiteral( "UseHtml" ) ).toBool() )
    {
      v = mTextEdit->toHtml();
    }
    else
    {
      v = mTextEdit->toPlainText();
    }
  }

  if ( mPlainTextEdit )
  {
    v = mPlainTextEdit->toPlainText();
  }

  if ( mLineEdit )
  {
    v = mLineEdit->text();
  }

  if ( ( v.isEmpty() && ( field().type() == QVariant::Int || field().type() == QVariant::Double || field().type() == QVariant::LongLong || field().type() == QVariant::Date ) ) ||
       v == QgsApplication::nullRepresentation() )
    return QVariant( field().type() );

  if ( !defaultValue().isNull() && v == defaultValue().toString() )
  {
    return defaultValue();
  }

  QVariant res( v );
  if ( field().convertCompatible( res ) )
  {
    return res;
  }
  else if ( field().type() == QVariant::String && field().length() > 0 )
  {
    // for string fields convertCompatible may return false due to field length limit - in this case just truncate
    // input rather then discarding it entirely
    return QVariant( v.left( field().length() ) );
  }
  else
  {
    return QVariant( field().type() );
  }
}

QWidget *QgsTextEditWrapper::createWidget( QWidget *parent )
{
  if ( config( QStringLiteral( "IsMultiline" ) ).toBool() )
  {
    if ( config( QStringLiteral( "UseHtml" ) ).toBool() )
    {
      return new QTextBrowser( parent );
    }
    else
    {
      return new QPlainTextEdit( parent );
    }
  }
  else
  {
    return new QgsFilterLineEdit( parent );
  }
}

void QgsTextEditWrapper::initWidget( QWidget *editor )
{
  mTextBrowser = qobject_cast<QTextBrowser *>( editor );
  mTextEdit = qobject_cast<QTextEdit *>( editor );
  mPlainTextEdit = qobject_cast<QPlainTextEdit *>( editor );
  mLineEdit = qobject_cast<QLineEdit *>( editor );

  if ( mTextEdit )
    connect( mTextEdit, &QTextEdit::textChanged, this, &QgsEditorWidgetWrapper::emitValueChanged );

  if ( mPlainTextEdit )
    connect( mPlainTextEdit, &QPlainTextEdit::textChanged, this, &QgsEditorWidgetWrapper::emitValueChanged );

  if ( mLineEdit )
  {
    mLineEdit->setValidator( new QgsFieldValidator( mLineEdit, field(), defaultValue().toString() ) );

    QVariant defVal = defaultValue();
    if ( defVal.isNull() )
    {
      defVal = QgsApplication::nullRepresentation();
    }

    QgsFilterLineEdit *fle = qobject_cast<QgsFilterLineEdit *>( mLineEdit );
    if ( field().type() == QVariant::Int || field().type() == QVariant::Double || field().type() == QVariant::LongLong || field().type() == QVariant::Date )
    {
      mPlaceholderText = defVal.toString();
      mLineEdit->setPlaceholderText( mPlaceholderText );
    }
    else if ( fle )
    {
      fle->setNullValue( defVal.toString() );
    }

    connect( mLineEdit, &QLineEdit::textChanged, this, [ = ]( const QString & value ) { emit valueChanged( value ); } );
    connect( mLineEdit, &QLineEdit::textChanged, this, &QgsTextEditWrapper::textChanged );

    mWritablePalette = mLineEdit->palette();
    mReadOnlyPalette = mLineEdit->palette();
  }
}

bool QgsTextEditWrapper::valid() const
{
  return mLineEdit || mTextEdit || mPlainTextEdit;
}

void QgsTextEditWrapper::showIndeterminateState()
{
  //note - this is deliberately a zero length string, not a null string!
  if ( mTextEdit )
    mTextEdit->blockSignals( true );
  if ( mPlainTextEdit )
    mPlainTextEdit->blockSignals( true );
  if ( mLineEdit )
  {
    mLineEdit->blockSignals( true );
    // for interdeminate state we need to clear the placeholder text - we want an empty line edit, not
    // one showing the default value (e.g., "NULL")
    mLineEdit->setPlaceholderText( QString() );
  }

  setWidgetValue( QString() );

  if ( mTextEdit )
    mTextEdit->blockSignals( false );
  if ( mPlainTextEdit )
    mPlainTextEdit->blockSignals( false );
  if ( mLineEdit )
    mLineEdit->blockSignals( false );
}

void QgsTextEditWrapper::setValue( const QVariant &val )
{
  if ( mLineEdit )
  {
    //restore placeholder text, which may have been removed by showIndeterminateState()
    mLineEdit->setPlaceholderText( mPlaceholderText );
  }
  setWidgetValue( val );
}

void QgsTextEditWrapper::setEnabled( bool enabled )
{
  if ( mTextEdit )
    mTextEdit->setReadOnly( !enabled );

  if ( mPlainTextEdit )
    mPlainTextEdit->setReadOnly( !enabled );

  if ( mLineEdit )
  {
    mLineEdit->setReadOnly( !enabled );
    if ( enabled )
      mLineEdit->setPalette( mWritablePalette );
    else
    {
      mLineEdit->setPalette( mReadOnlyPalette );
      // removing frame + setting transparent background to distinguish the readonly lineEdit from a normal one
      // did not get this working via the Palette:
      mLineEdit->setStyleSheet( QStringLiteral( "background-color: rgba(255, 255, 255, 75%);" ) );
    }
    mLineEdit->setFrame( enabled );
  }
}

void QgsTextEditWrapper::textChanged( const QString & )
{
  if ( mLineEdit )
  {
    //restore placeholder text, which may have been removed by showIndeterminateState()
    mLineEdit->setPlaceholderText( mPlaceholderText );
  }
}

void QgsTextEditWrapper::setWidgetValue( const QVariant &val )
{
  QString v;
  if ( val.isNull() )
  {
    if ( !( field().type() == QVariant::Int || field().type() == QVariant::Double || field().type() == QVariant::LongLong || field().type() == QVariant::Date ) )
      v = QgsApplication::nullRepresentation();
  }
  else
  {
    v = field().displayString( val );
  }

  // For numbers, remove the group separator that might cause validation errors
  // when the user is editing the field value.
  // We are checking for editable layer because in the form field context we do not
  // want to strip the separator unless the layer is editable.
  // Also check that we have something like a number in the value to avoid
  // stripping out dots from nextval when we have a schema: see https://issues.qgis.org/issues/20200
  // "Wrong sequence detection with Postgres"
  bool canConvertToDouble;
  QLocale().toDouble( v, &canConvertToDouble );
  if ( canConvertToDouble && layer() && layer()->isEditable() && ! QLocale().groupSeparator().isNull() && field().isNumeric() )
  {
    v = v.remove( QLocale().groupSeparator() );
  }

  if ( mTextEdit )
  {
    if ( val != value() )
    {
      if ( config( QStringLiteral( "UseHtml" ) ).toBool() )
      {
        mTextEdit->setHtml( v );
        if ( mTextBrowser )
        {
          mTextBrowser->setTextInteractionFlags( Qt::LinksAccessibleByMouse );
          mTextBrowser->setOpenExternalLinks( true );
        }
      }
      else
        mTextEdit->setPlainText( v );
    }
  }

  if ( mPlainTextEdit )
  {
    if ( val != value() )
      mPlainTextEdit->setPlainText( v );
  }

  if ( mLineEdit )
    mLineEdit->setText( v );
}

void QgsTextEditWrapper::setHint( const QString &hintText )
{
  if ( hintText.isNull() )
    mPlaceholderText = mPlaceholderTextBackup;
  else
  {
    mPlaceholderTextBackup = mPlaceholderText;
    mPlaceholderText = hintText;
  }

  mLineEdit->setPlaceholderText( mPlaceholderText );
}
