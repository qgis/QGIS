/***************************************************************************
    qgsexternalresourcewidgetwrapper.cpp
     --------------------------------------
 begin                : 16.12.2015
 copyright            : (C) 2015 by Denis Rouzaud
 email                : denis.rouzaud@gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsexternalresourcewidgetwrapper.h"

#include <QPushButton>
#include <QSettings>
#include <QLabel>


#include "qgsexternalresourcewidget.h"
#include "qgsfilterlineedit.h"


QgsExternalResourceWidgetWrapper::QgsExternalResourceWidgetWrapper( QgsVectorLayer* vl, int fieldIdx, QWidget* editor, QWidget* parent )
    : QgsEditorWidgetWrapper( vl, fieldIdx, editor, parent )
    , mLineEdit( nullptr )
    , mLabel( nullptr )
    , mQgsWidget( nullptr )
{
}

QVariant QgsExternalResourceWidgetWrapper::value() const
{
  if ( mQgsWidget )
  {
    mQgsWidget->documentPath( field().type() );
  }

  if ( mLineEdit )
  {
    if ( mLineEdit->text().isEmpty() || mLineEdit->text() == QSettings().value( "qgis/nullValue", "NULL" ).toString() )
    {
      return QVariant( field().type() );
    }
    else
    {
      return mLineEdit->text();
    }
  }

  return QVariant( field().type() );
}

bool QgsExternalResourceWidgetWrapper::valid() const
{
  return mLineEdit || mLabel || mQgsWidget;
}

QWidget* QgsExternalResourceWidgetWrapper::createWidget( QWidget* parent )
{
  return new QgsExternalResourceWidget( parent );
}

void QgsExternalResourceWidgetWrapper::initWidget( QWidget* editor )
{
  mLineEdit = qobject_cast<QLineEdit*>( editor );
  mLabel = qobject_cast<QLabel*>( editor );
  mQgsWidget = qobject_cast<QgsExternalResourceWidget*>( editor );

  if ( mLineEdit )
  {
    QgsFilterLineEdit* fle = qobject_cast<QgsFilterLineEdit*>( editor );
    if ( fle )
    {
      fle->setNullValue( QSettings().value( "qgis/nullValue", "NULL" ).toString() );
    }
    connect( mLineEdit, SIGNAL( textChanged( QString ) ), this, SLOT( valueChanged( QString ) ) );
  }

  if ( mQgsWidget )
  {
    if ( config().contains( "UseLink" ) )
    {
      mQgsWidget->filePickerwidget()->setUseLink( config( "UseLink" ).toBool() );
    }
    if ( config().contains( "FullUrl" ) )
    {
      mQgsWidget->filePickerwidget()->setFullUrl( config( "FullUrl" ).toBool() );
    }
    if ( config().contains( "DefaultRoot" ) )
    {
      mQgsWidget->filePickerwidget()->setDefaultRoot( config( "DefaultRoot" ).toString() );
    }
    if ( config().contains( "StorageMode" ) )
    {
      mQgsWidget->filePickerwidget()->setStorageMode(( QgsFilePickerWidget::StorageMode )config( "StorageMode" ).toInt() );
    }
    if ( config().contains( "RelativeStorage" ) )
    {
      mQgsWidget->filePickerwidget()->setRelativeStorage(( QgsFilePickerWidget::RelativeStorage )config( "RelativeStorage" ).toInt() );
    }
    if ( config().contains( "FilePicker" ) )
    {
      mQgsWidget->setFilePickerVisible( config( "FilePicker" ).toBool() );
    }
    if ( config().contains( "FilePickerButton" ) )
    {
      mQgsWidget->filePickerwidget()->setFilePickerButtonVisible( config( "FilePickerButton" ).toBool() );
    }
    if ( config().contains( "DocumentViewer" ) )
    {
      mQgsWidget->setDocumentViewerContent(( QgsExternalResourceWidget::DocumentViewerContent )config( "DocumentViewer" ).toInt() );
    }
  }
}

void QgsExternalResourceWidgetWrapper::setValue( const QVariant& value )
{
  if ( mLineEdit )
  {
    if ( value.isNull() )
    {
      mLineEdit->setText( QSettings().value( "qgis/nullValue", "NULL" ).toString() );
    }
    else
    {
      mLineEdit->setText( value.toString() );
    }
  }

  if ( mLabel )
  {
    mLabel->setText( value.toString() ) ;
    valueChanged( value.toString() ); // emit signal that value has changed, do not do it for other widgets
  }

  if ( mQgsWidget )
  {
    if ( value.isNull() )
    {
      mQgsWidget->setDocumentPath( QSettings().value( "qgis/nullValue", "NULL" ).toString() );
    }
    else
    {
      mQgsWidget->setDocumentPath( value.toString() );
    }
  }

}

void QgsExternalResourceWidgetWrapper::setEnabled( bool enabled )
{
  if ( mLineEdit )
    mLineEdit->setReadOnly( !enabled );

  if ( mQgsWidget )
    mQgsWidget->setReadOnly( !enabled );
}
