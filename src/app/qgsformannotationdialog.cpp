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
#include "qgsgui.h"
#include "qgshelp.h"
#include <QFileDialog>
#include <QFileInfo>
#include <QGraphicsScene>
#include <QPushButton>

QgsFormAnnotationDialog::QgsFormAnnotationDialog( QgsMapCanvasAnnotationItem *item, QWidget *parent, Qt::WindowFlags f )
  : QDialog( parent, f )
  , mItem( item )

{
  setupUi( this );
  connect( mBrowseToolButton, &QToolButton::clicked, this, &QgsFormAnnotationDialog::mBrowseToolButton_clicked );
  connect( mButtonBox, &QDialogButtonBox::clicked, this, &QgsFormAnnotationDialog::mButtonBox_clicked );
  mEmbeddedWidget = new QgsAnnotationWidget( mItem );
  mStackedWidget->addWidget( mEmbeddedWidget );
  mStackedWidget->setCurrentWidget( mEmbeddedWidget );

  if ( item && item->annotation() )
  {
    QgsFormAnnotation *annotation = static_cast< QgsFormAnnotation * >( item->annotation() );
    mFileLineEdit->setText( annotation->designerForm() );
  }

  QObject::connect( mButtonBox, &QDialogButtonBox::accepted, this, &QgsFormAnnotationDialog::applySettingsToItem );
  QObject::connect( mButtonBox, &QDialogButtonBox::helpRequested, this, &QgsFormAnnotationDialog::showHelp );
  QPushButton *deleteButton = new QPushButton( tr( "Delete" ) );
  QObject::connect( deleteButton, &QPushButton::clicked, this, &QgsFormAnnotationDialog::deleteItem );
  mButtonBox->addButton( deleteButton, QDialogButtonBox::RejectRole );

  QgsGui::enableAutoGeometryRestore( this );
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

void QgsFormAnnotationDialog::mBrowseToolButton_clicked()
{
  QString directory;
  const QFileInfo fi( mFileLineEdit->text() );
  if ( fi.exists() )
  {
    directory = fi.absolutePath();
  }
  const QString filename = QFileDialog::getOpenFileName( nullptr, tr( "Qt designer file" ), directory, QStringLiteral( "*.ui" ) );
  mFileLineEdit->setText( filename );
}

void QgsFormAnnotationDialog::deleteItem()
{
  if ( mItem && mItem->annotation() )
    QgsProject::instance()->annotationManager()->removeAnnotation( mItem->annotation() );
  mItem = nullptr;
}

void QgsFormAnnotationDialog::mButtonBox_clicked( QAbstractButton *button )
{
  if ( mButtonBox->buttonRole( button ) == QDialogButtonBox::ApplyRole )
  {
    applySettingsToItem();
  }
}

void QgsFormAnnotationDialog::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "introduction/general_tools.html#annotation-tools" ) );
}
