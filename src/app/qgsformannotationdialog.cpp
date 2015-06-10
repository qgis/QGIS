/***************************************************************************
    qgsformannotationdialog.cpp
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
#include "qgsformannotationdialog.h"
#include "qgsannotationwidget.h"
#include "qgsvectorlayer.h"
#include <QFileDialog>
#include <QFileInfo>
#include <QGraphicsScene>

QgsFormAnnotationDialog::QgsFormAnnotationDialog( QgsFormAnnotationItem* item, QWidget * parent, Qt::WindowFlags f )
    : QDialog( parent, f ), mItem( item ), mEmbeddedWidget( 0 )
{
  setupUi( this );
  mEmbeddedWidget = new QgsAnnotationWidget( mItem );
  mEmbeddedWidget->show();
  mStackedWidget->addWidget( mEmbeddedWidget );
  mStackedWidget->setCurrentWidget( mEmbeddedWidget );

  if ( item )
  {
    mFileLineEdit->setText( item->designerForm() );
  }

  QObject::connect( mButtonBox, SIGNAL( accepted() ), this, SLOT( applySettingsToItem() ) );
  QPushButton* deleteButton = new QPushButton( tr( "Delete" ) );
  QObject::connect( deleteButton, SIGNAL( clicked() ), this, SLOT( deleteItem() ) );
  mButtonBox->addButton( deleteButton, QDialogButtonBox::RejectRole );
}

QgsFormAnnotationDialog::~QgsFormAnnotationDialog()
{

}

void QgsFormAnnotationDialog::applySettingsToItem()
{
  //apply settings from embedded item widget
  if ( mEmbeddedWidget )
  {
    mEmbeddedWidget->apply();
  }

  if ( mItem )
  {
    mItem->setDesignerForm( mFileLineEdit->text() );
    QgsVectorLayer* layer = mItem->vectorLayer();
    if ( layer )
    {
      //set last used annotation form as default for the layer
      layer->setAnnotationForm( mFileLineEdit->text() );
    }
    mItem->update();
  }
}

void QgsFormAnnotationDialog::on_mBrowseToolButton_clicked()
{
  QString directory;
  QFileInfo fi( mFileLineEdit->text() );
  if ( fi.exists() )
  {
    directory = fi.absolutePath();
  }
  QString filename = QFileDialog::getOpenFileName( 0, tr( "Qt designer file" ), directory, "*.ui" );
  mFileLineEdit->setText( filename );
}

void QgsFormAnnotationDialog::deleteItem()
{
  QGraphicsScene* scene = mItem->scene();
  if ( scene )
  {
    scene->removeItem( mItem );
  }
  delete mItem;
  mItem = 0;
}

