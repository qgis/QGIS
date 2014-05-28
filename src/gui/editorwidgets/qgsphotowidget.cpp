/***************************************************************************
    qgsphotowidget.cpp
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

#include "qgsphotowidget.h"

#include <QGridLayout>
#include <QFileDialog>


#include "qgsfilterlineedit.h"

QgsPhotoWidget::QgsPhotoWidget( QgsVectorLayer* vl, int fieldIdx, QWidget* editor, QWidget* parent )
    :  QgsEditorWidgetWrapper( vl, fieldIdx, editor, parent )
{
}

void QgsPhotoWidget::selectFileName()
{
  if ( mLineEdit )
  {
    QString fileName = QFileDialog::getOpenFileName( 0 , tr( "Select a picture" ), QFileInfo( mLineEdit->text() ).absolutePath() );
    if ( !fileName.isNull() )
      mLineEdit->setText( QDir::toNativeSeparators( fileName ) );
  }
}

void QgsPhotoWidget::loadPixmap( const QString &fileName )
{
  QPixmap pm( fileName );
  if ( !pm.isNull() && mPhotoLabel )
  {
    QSize size( config( "Width" ).toInt(), config( "Height" ).toInt() );
    if ( size.width() == 0 && size.height() > 0 )
    {
      size.setWidth( size.height() * pm.size().width() / pm.size().height() );
    }
    else if ( size.width() > 0 && size.height() == 0 )
    {
      size.setHeight( size.width() * pm.size().height() / pm.size().width() );
    }

    pm = pm.scaled( size, Qt::KeepAspectRatio, Qt::SmoothTransformation );


    mPhotoLabel->setPixmap( pm );
    mPhotoLabel->setMinimumSize( size );
  }
}

QVariant QgsPhotoWidget::value()
{
  QVariant v;

  if ( mLineEdit )
    v = mLineEdit->text();

  return v;
}

QWidget* QgsPhotoWidget::createWidget( QWidget* parent )
{
  QWidget* container = new QWidget( parent );
  QGridLayout* layout = new QGridLayout( container );
  QgsFilterLineEdit* le = new QgsFilterLineEdit( container );
  QLabel* label = new QLabel( parent );
  label->setObjectName( "PhotoLabel" );
  QPushButton* pb = new QPushButton( tr( "..." ), container );
  pb->setObjectName( "FileChooserButton" );

  layout->addWidget( label, 0, 0, 1, 2 );
  layout->addWidget( le, 1, 0 );
  layout->addWidget( pb, 1, 1 );

  container->setLayout( layout );

  return container;
}

void QgsPhotoWidget::initWidget( QWidget* editor )
{
  QWidget* container;

  mLineEdit = qobject_cast<QLineEdit*>( editor );

  if ( mLineEdit )
    container = qobject_cast<QWidget*>( mLineEdit->parent() );
  else
  {
    container = editor;
    mLineEdit = container->findChild<QLineEdit*>();
  }

  mButton = container->findChild<QPushButton*>( "FileChooserButton" );
  if ( !mButton )
    mButton = container->findChild<QPushButton*>();

  mPhotoLabel = container->findChild<QLabel*>( "PhotoLabel" );
  if ( !mPhotoLabel )
    mPhotoLabel = container->findChild<QLabel*>();

  if ( mButton )
    connect( mButton, SIGNAL( clicked() ), this, SLOT( selectFileName() ) );

  if ( mLineEdit )
  {
    connect( mLineEdit, SIGNAL( textChanged( QString ) ), this, SLOT( valueChanged( QString ) ) );
    connect( mLineEdit, SIGNAL( textChanged( QString ) ), this, SLOT( loadPixmap( QString ) ) );
  }
}

void QgsPhotoWidget::setValue( const QVariant& value )
{
  if ( mLineEdit )
    mLineEdit->setText( value.toString() );


}

void QgsPhotoWidget::setEnabled( bool enabled )
{
  if ( mLineEdit )
    mLineEdit->setEnabled( enabled );

  if ( mButton )
    mButton->setEnabled( enabled );
}
