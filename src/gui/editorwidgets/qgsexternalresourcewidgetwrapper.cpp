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
    return mQgsWidget->documentPath( field().type() );
  }

  if ( mLineEdit )
  {
    if ( mLineEdit->text().isEmpty() || mLineEdit->text() == QSettings().value( QStringLiteral( "qgis/nullValue" ), "NULL" ).toString() )
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

void QgsExternalResourceWidgetWrapper::showIndeterminateState()
{
  if ( mLineEdit )
  {
    whileBlocking( mLineEdit )->clear();
  }

  if ( mLabel )
  {
    mLabel->clear();
  }

  if ( mQgsWidget )
  {
    whileBlocking( mQgsWidget )->setDocumentPath( QString() );
  }
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
      fle->setNullValue( QSettings().value( QStringLiteral( "qgis/nullValue" ), "NULL" ).toString() );
    }
  }
  else
    mLineEdit = editor->findChild<QLineEdit*>();

  if ( mQgsWidget )
  {
    mQgsWidget->fileWidget()->setStorageMode( QgsFileWidget::GetFile );
    if ( config().contains( QStringLiteral( "UseLink" ) ) )
    {
      mQgsWidget->fileWidget()->setUseLink( config( QStringLiteral( "UseLink" ) ).toBool() );
    }
    if ( config().contains( QStringLiteral( "FullUrl" ) ) )
    {
      mQgsWidget->fileWidget()->setFullUrl( config( QStringLiteral( "FullUrl" ) ).toBool() );
    }
    if ( config().contains( QStringLiteral( "DefaultRoot" ) ) )
    {
      mQgsWidget->setDefaultRoot( config( QStringLiteral( "DefaultRoot" ) ).toString() );
    }
    if ( config().contains( QStringLiteral( "StorageMode" ) ) )
    {
      mQgsWidget->fileWidget()->setStorageMode(( QgsFileWidget::StorageMode )config( QStringLiteral( "StorageMode" ) ).toInt() );
    }
    if ( config().contains( QStringLiteral( "RelativeStorage" ) ) )
    {
      mQgsWidget->setRelativeStorage(( QgsFileWidget::RelativeStorage )config( QStringLiteral( "RelativeStorage" ) ).toInt() );
    }
    if ( config().contains( QStringLiteral( "FileWidget" ) ) )
    {
      mQgsWidget->setFileWidgetVisible( config( QStringLiteral( "FileWidget" ) ).toBool() );
    }
    if ( config().contains( QStringLiteral( "FileWidgetButton" ) ) )
    {
      mQgsWidget->fileWidget()->setFileWidgetButtonVisible( config( QStringLiteral( "FileWidgetButton" ) ).toBool() );
    }
    if ( config().contains( QStringLiteral( "DocumentViewer" ) ) )
    {
      mQgsWidget->setDocumentViewerContent(( QgsExternalResourceWidget::DocumentViewerContent )config( QStringLiteral( "DocumentViewer" ) ).toInt() );
    }
    if ( config().contains( QStringLiteral( "FileWidgetFilter" ) ) )
    {
      mQgsWidget->fileWidget()->setFilter( config( QStringLiteral( "FileWidgetFilter" ) ).toString() );
    }
  }

  if ( mLineEdit )
    connect( mLineEdit, SIGNAL( textChanged( QString ) ), this, SLOT( valueChanged( QString ) ) );

}

void QgsExternalResourceWidgetWrapper::setValue( const QVariant& value )
{
  if ( mLineEdit )
  {
    if ( value.isNull() )
    {
      mLineEdit->setText( QSettings().value( QStringLiteral( "qgis/nullValue" ), "NULL" ).toString() );
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
      mQgsWidget->setDocumentPath( QSettings().value( QStringLiteral( "qgis/nullValue" ), "NULL" ).toString() );
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

void QgsExternalResourceWidgetWrapper::updateConstraintWidgetStatus( ConstraintResult status )
{
  if ( mLineEdit )
  {
    switch ( status )
    {
      case ConstraintResultPass:
        mLineEdit->setStyleSheet( QString() );
        break;

      case ConstraintResultFailHard:
        mLineEdit->setStyleSheet( QStringLiteral( "QgsFilterLineEdit { background-color: #dd7777; }" ) );
        break;

      case ConstraintResultFailSoft:
        mLineEdit->setStyleSheet( QStringLiteral( "QgsFilterLineEdit { background-color: #ffd85d; }" ) );
        break;
    }
  }
}
