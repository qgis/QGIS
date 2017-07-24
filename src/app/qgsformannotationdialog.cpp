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
#include "qgsformannotation.h"
#include "qgsmapcanvasannotationitem.h"
#include "qgsvectorlayer.h"
#include "qgsproject.h"
#include "qgsannotationmanager.h"
#include <QFileDialog>
#include <QFileInfo>
#include <QGraphicsScene>
#include <QPushButton>

QgsFormAnnotationDialog::QgsFormAnnotationDialog( QgsMapCanvasAnnotationItem *item, QWidget *parent, Qt::WindowFlags f )
  : QDialog( parent, f )
  , mItem( item )
  , mEmbeddedWidget( nullptr )
{
  setupUi( this );
  mEmbeddedWidget = new QgsAnnotationWidget( mItem );
  mStackedWidget->addWidget( mEmbeddedWidget );
  mStackedWidget->setCurrentWidget( mEmbeddedWidget );

  if ( item && item->annotation() )
  {
    QgsFormAnnotation *annotation = static_cast< QgsFormAnnotation * >( item->annotation() );
    mFileLineEdit->setText( annotation->designerForm() );
  }

  QObject::connect( mButtonBox, &QDialogButtonBox::accepted, this, &QgsFormAnnotationDialog::applySettingsToItem );
  QPushButton *deleteButton = new QPushButton( tr( "Delete" ) );
  QObject::connect( deleteButton, &QPushButton::clicked, this, &QgsFormAnnotationDialog::deleteItem );
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

  if ( mItem && mItem->annotation() )
  {
    QgsFormAnnotation *annotation = static_cast< QgsFormAnnotation * >( mItem->annotation() );
    annotation->setDesignerForm( mFileLineEdit->text() );
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
  QString filename = QFileDialog::getOpenFileName( nullptr, tr( "Qt designer file" ), directory, QStringLiteral( "*.ui" ) );
  mFileLineEdit->setText( filename );
}

void QgsFormAnnotationDialog::deleteItem()
{
  if ( mItem && mItem->annotation() )
    QgsProject::instance()->annotationManager()->removeAnnotation( mItem->annotation() );
  mItem = nullptr;
}

void QgsFormAnnotationDialog::on_mButtonBox_clicked( QAbstractButton *button )
{
  if ( mButtonBox->buttonRole( button ) == QDialogButtonBox::ApplyRole )
  {
    applySettingsToItem();
  }
}

