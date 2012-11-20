/***************************************************************************
                              qgssvgannotationdialog.cpp
                              --------------------------
  begin                : November, 2012
  copyright            : (C) 2012 by Marco Hugentobler
  email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgssvgannotationdialog.h"
#include "qgsannotationwidget.h"
#include "qgssvgannotationitem.h"
#include <QFileDialog>
#include <QFileInfo>
#include <QGraphicsScene>

QgsSvgAnnotationDialog::QgsSvgAnnotationDialog( QgsSvgAnnotationItem* item, QWidget * parent, Qt::WindowFlags f ):
    QDialog( parent, f ), mItem( item ), mEmbeddedWidget( 0 )
{
  setupUi( this );
  setWindowTitle( tr( "SVG annotation" ) );
  mEmbeddedWidget = new QgsAnnotationWidget( mItem );
  mEmbeddedWidget->show();
  mStackedWidget->addWidget( mEmbeddedWidget );
  mStackedWidget->setCurrentWidget( mEmbeddedWidget );

  if ( mItem )
  {
    mFileLineEdit->setText( mItem->filePath() );
  }

  QObject::connect( mButtonBox, SIGNAL( accepted() ), this, SLOT( applySettingsToItem() ) );
  QPushButton* deleteButton = new QPushButton( tr( "Delete" ) );
  QObject::connect( deleteButton, SIGNAL( clicked() ), this, SLOT( deleteItem() ) );
  mButtonBox->addButton( deleteButton, QDialogButtonBox::RejectRole );
}

QgsSvgAnnotationDialog::QgsSvgAnnotationDialog(): QDialog(), mItem( 0 ), mEmbeddedWidget( 0 )
{

}

QgsSvgAnnotationDialog::~QgsSvgAnnotationDialog()
{

}

void QgsSvgAnnotationDialog::on_mBrowseToolButton_clicked()
{
  QString directory;
  QFileInfo fi( mFileLineEdit->text() );
  if ( fi.exists() )
  {
    directory = fi.absolutePath();
  }
  QString filename = QFileDialog::getOpenFileName( 0, tr( "Select SVG file" ), directory, tr( "SVG files" ) + " (*.svg)" );
  mFileLineEdit->setText( filename );
}

void QgsSvgAnnotationDialog::applySettingsToItem()
{
  if ( mEmbeddedWidget )
  {
    mEmbeddedWidget->apply();
  }

  if ( mItem )
  {
    mItem->setFilePath( mFileLineEdit->text() );
    mItem->update();
  }

}

void QgsSvgAnnotationDialog::deleteItem()
{
  QGraphicsScene* scene = mItem->scene();
  if ( scene )
  {
    scene->removeItem( mItem );
  }
  delete mItem;
  mItem = 0;
}
