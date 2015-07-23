/***************************************************************************
    qgsfilenamewidgetwrapper.cpp
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

#include "qgsfilenamewidgetwrapper.h"

#include "qgsfilterlineedit.h"

#include <QFileDialog>
#include <QSettings>
#include <QGridLayout>

QgsFileNameWidgetWrapper::QgsFileNameWidgetWrapper( QgsVectorLayer* vl, int fieldIdx, QWidget* editor, QWidget* parent )
    : QgsEditorWidgetWrapper( vl, fieldIdx, editor, parent )
    , mLineEdit( NULL )
    , mPushButton( NULL )
    , mLabel( NULL )
{
}

QVariant QgsFileNameWidgetWrapper::value()
{
  QVariant value;

  if ( mLineEdit )
  {
    if ( mLineEdit->text() == QSettings().value( "qgis/nullValue", "NULL" ).toString() )
      value = QVariant( field().type() );
    else
      value = mLineEdit->text();
  }

  if ( mLabel )
    value = mLabel->text();

  return value;
}

bool QgsFileNameWidgetWrapper::valid()
{
  return mLineEdit || mLabel;
}

QWidget* QgsFileNameWidgetWrapper::createWidget( QWidget* parent )
{
  QWidget* container = new QWidget( parent );
  container->setBackgroundRole( QPalette::Window );
  container->setAutoFillBackground( true );

  QLineEdit* le = new QgsFilterLineEdit( container );
  QPushButton* pbn = new QPushButton( tr( "..." ), container );
  QGridLayout* layout = new QGridLayout();

  layout->setMargin( 0 );
  layout->addWidget( le, 0, 0 );
  layout->addWidget( pbn, 0, 1 );

  container->setLayout( layout );

  return container;
}

void QgsFileNameWidgetWrapper::initWidget( QWidget* editor )
{
  mLineEdit = qobject_cast<QLineEdit*>( editor );
  if ( !mLineEdit )
  {
    mLineEdit = editor->findChild<QLineEdit*>();
  }

  mPushButton = editor->findChild<QPushButton*>();

  if ( mPushButton )
    connect( mPushButton, SIGNAL( clicked() ), this, SLOT( selectFileName() ) );

  mLabel = qobject_cast<QLabel*>( editor );

  if ( mLineEdit )
  {
    QgsFilterLineEdit* fle = qobject_cast<QgsFilterLineEdit*>( editor );
    if ( fle )
    {
      fle->setNullValue( QSettings().value( "qgis/nullValue", "NULL" ).toString() );
    }

    connect( mLineEdit, SIGNAL( textChanged( QString ) ), this, SLOT( valueChanged( QString ) ) );
  }
}

void QgsFileNameWidgetWrapper::setValue( const QVariant& value )
{
  if ( mLineEdit )
  {
    if ( value.isNull() )
      mLineEdit->setText( QSettings().value( "qgis/nullValue", "NULL" ).toString() );
    else
      mLineEdit->setText( value.toString() );
  }

  if ( mLabel )
    mLabel->setText( value.toString() );
}

void QgsFileNameWidgetWrapper::selectFileName()
{
  QString text;

  if ( mLineEdit )
    text = mLineEdit->text();

  if ( mLabel )
    text = mLabel->text();

  QString fileName = QFileDialog::getOpenFileName( mLineEdit, tr( "Select a file" ), QFileInfo( text ).absolutePath() );

  if ( fileName.isNull() )
    return;

  if ( mLineEdit )
    mLineEdit->setText( QDir::toNativeSeparators( fileName ) );

  if ( mLabel )
    mLineEdit->setText( QDir::toNativeSeparators( fileName ) );
}
