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

#include <QSettings>

QgsTextEditWidget::QgsTextEditWidget( QgsVectorLayer* vl, int fieldIdx, QWidget* editor, QWidget* parent )
    :  QgsEditorWidgetWrapper( vl, fieldIdx, editor, parent )
{
}

QVariant QgsTextEditWidget::value()
{
  QSettings settings;
  QVariant v;

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

  if ( v.toString() == settings.value( "qgis/nullValue", "NULL" ).toString() )
  {
    v = QVariant( field().type() );
  }

  return v;
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
    return new QLineEdit( parent );
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
    connect( mLineEdit, SIGNAL( textChanged( QString ) ), this, SLOT( valueChanged( QString ) ) );
}

void QgsTextEditWidget::setValue( const QVariant& value )
{
  if ( mTextEdit )
  {
    if ( config( "UseHtml" ).toBool() )
      mTextEdit->setHtml( value.toString() );
    else
      mTextEdit->setPlainText( value.toString() );
  }

  if ( mPlainTextEdit )
    mPlainTextEdit->setPlainText( value.toString() );

  if ( mLineEdit )
    mLineEdit->setText( value.toString() );
}
