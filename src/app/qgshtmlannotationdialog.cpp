/***************************************************************************
    QgsHTMLAnnotationDialog.cpp
    ---------------------
    begin                : March 2010
    copyright            : (C) 2010 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgshtmlannotationdialog.h"
#include "qgshtmlannotation.h"
#include "qgsannotationwidget.h"
#include "qgsmapcanvasannotationitem.h"
#include "qgsvectorlayer.h"
#include "qgsproject.h"
#include "qgsannotationmanager.h"
#include <QFileDialog>
#include <QFileInfo>
#include <QGraphicsScene>
#include <QPushButton>

QgsHtmlAnnotationDialog::QgsHtmlAnnotationDialog( QgsMapCanvasAnnotationItem *item, QWidget *parent, Qt::WindowFlags f )
  : QDialog( parent, f )
  , mItem( item )

{
  setupUi( this );
  connect( mBrowseToolButton, &QToolButton::clicked, this, &QgsHtmlAnnotationDialog::mBrowseToolButton_clicked );
  connect( mButtonBox, &QDialogButtonBox::clicked, this, &QgsHtmlAnnotationDialog::mButtonBox_clicked );
  setWindowTitle( tr( "HTML Annotation" ) );
  mEmbeddedWidget = new QgsAnnotationWidget( mItem );
  mStackedWidget->addWidget( mEmbeddedWidget );
  mStackedWidget->setCurrentWidget( mEmbeddedWidget );

  if ( item && item->annotation() )
  {
    QgsHtmlAnnotation *annotation = static_cast< QgsHtmlAnnotation * >( item->annotation() );
    mFileLineEdit->setText( annotation->sourceFile() );
  }

  QObject::connect( mButtonBox, &QDialogButtonBox::accepted, this, &QgsHtmlAnnotationDialog::applySettingsToItem );
  QPushButton *deleteButton = new QPushButton( tr( "Delete" ) );
  QObject::connect( deleteButton, &QPushButton::clicked, this, &QgsHtmlAnnotationDialog::deleteItem );
  mButtonBox->addButton( deleteButton, QDialogButtonBox::RejectRole );
}

void QgsHtmlAnnotationDialog::applySettingsToItem()
{
  //apply settings from embedded item widget
  if ( mEmbeddedWidget )
  {
    mEmbeddedWidget->apply();
  }

  if ( mItem && mItem->annotation() )
  {
    QgsHtmlAnnotation *annotation = static_cast< QgsHtmlAnnotation * >( mItem->annotation() );
    annotation->setSourceFile( mFileLineEdit->text() );
    mItem->update();
  }
}

void QgsHtmlAnnotationDialog::mBrowseToolButton_clicked()
{
  QString directory;
  QFileInfo fi( mFileLineEdit->text() );
  if ( fi.exists() )
  {
    directory = fi.absolutePath();
  }
  else
  {
    directory = QDir::homePath();
  }
  QString filename = QFileDialog::getOpenFileName( nullptr, tr( "html" ), directory, QStringLiteral( "HTML (*.html *.htm);;All files (*.*)" ) );
  mFileLineEdit->setText( filename );
}

void QgsHtmlAnnotationDialog::deleteItem()
{
  if ( mItem && mItem->annotation() )
    QgsProject::instance()->annotationManager()->removeAnnotation( mItem->annotation() );
  mItem = nullptr;
}

void QgsHtmlAnnotationDialog::mButtonBox_clicked( QAbstractButton *button )
{
  if ( mButtonBox->buttonRole( button ) == QDialogButtonBox::ApplyRole )
  {
    applySettingsToItem();
  }
}

