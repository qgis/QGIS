/***************************************************************************
    qgstexteditwidget.cpp
     --------------------------------------
    Date                 : 5.1.2014
    Copyright            : (C) 2014 Matthias Kuhn
    Email                : matthias dot kuhn at gmx dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstexteditwidget.h"

#include "qgsfield.h"
#include "qgsfieldvalidator.h"
#include "qgsfilterlineedit.h"

#include <QSettings>

QgsTextEditWidget::QgsTextEditWidget( QgsVectorLayer* vl, int fieldIdx, QWidget* editor, QWidget* parent )
    :  QgsEditorWidgetWrapper( vl, fieldIdx, editor, parent )
{
}

QVariant QgsTextEditWidget::value()
{
  QString v;

  if ( mTextEdit && mTextEdit->document()->isModified() )
  {
    if ( config( "UseHtml" ).toBool() )
    {
      v = mTextEdit->toHtml();
    }
    else
    {
      v = mTextEdit->toPlainText();
    }
  }

  if ( mPlainTextEdit && mPlainTextEdit->document()->isModified() )
  {
    v = mPlainTextEdit->toPlainText();
  }

  if ( mLineEdit )
  {
    v = mLineEdit->text();
  }

  if (( v.isEmpty() && ( field().type() == QVariant::Int || field().type() == QVariant::Double || field().type() == QVariant::LongLong || field().type() == QVariant::Date ) ) ||
      v == QSettings().value( "qgis/nullValue", "NULL" ).toString() )
    return QVariant( field().type() );
  else
    return QVariant( v );
}

QWidget* QgsTextEditWidget::createWidget( QWidget* parent )
{
  if ( config( "IsMultiline" ).toBool() )
  {
    if ( config( "UseHtml" ).toBool() )
    {
      return new QTextEdit( parent );
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

void QgsTextEditWidget::initWidget( QWidget* editor )
{
  mTextEdit = qobject_cast<QTextEdit*>( editor );
  mPlainTextEdit = qobject_cast<QPlainTextEdit*>( editor );
  mLineEdit = qobject_cast<QLineEdit*>( editor );

  if ( mTextEdit )
    connect( mTextEdit, SIGNAL( textChanged() ), this, SLOT( valueChanged() ) );

  if ( mPlainTextEdit )
    connect( mPlainTextEdit, SIGNAL( textChanged() ), this, SLOT( valueChanged() ) );

  if ( mLineEdit )
  {
    mLineEdit->setValidator( new QgsFieldValidator( mLineEdit, field() ) );

    QgsFilterLineEdit *fle = qobject_cast<QgsFilterLineEdit*>( mLineEdit );
    if ( fle && !( field().type() == QVariant::Int || field().type() == QVariant::Double || field().type() == QVariant::LongLong || field().type() == QVariant::Date ) )
    {
      fle->setNullValue( QSettings().value( "qgis/nullValue", "NULL" ).toString() );
    }

    connect( mLineEdit, SIGNAL( textChanged( QString ) ), this, SLOT( valueChanged( QString ) ) );
  }
}

void QgsTextEditWidget::setValue( const QVariant& value )
{
  QString v;
  if ( value.isNull() )
  {
    if ( !( field().type() == QVariant::Int || field().type() == QVariant::Double || field().type() == QVariant::LongLong || field().type() == QVariant::Date ) )
      v = QSettings().value( "qgis/nullValue", "NULL" ).toString();
  }
  else
    v = value.toString();

  if ( mTextEdit )
  {
    if ( config( "UseHtml" ).toBool() )
      mTextEdit->setHtml( v );
    else
      mTextEdit->setPlainText( v );
  }

  if ( mPlainTextEdit )
    mPlainTextEdit->setPlainText( v );

  if ( mLineEdit )
    mLineEdit->setText( v );
}
