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
#include "qgsgui.h"
#include "qgshelp.h"
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

  connect( mFileRadioButton, &QToolButton::toggled, this, &QgsHtmlAnnotationDialog::fileRadioButtonToggled );
  connect( mSourceRadioButton, &QToolButton::toggled, this, &QgsHtmlAnnotationDialog::sourceRadioButtonToggled );

  setWindowTitle( tr( "HTML Annotation" ) );
  mEmbeddedWidget = new QgsAnnotationWidget( mItem );
  mStackedWidget->addWidget( mEmbeddedWidget );
  mStackedWidget->setCurrentWidget( mEmbeddedWidget );

  if ( item && item->annotation() )
  {
    QgsHtmlAnnotation *annotation = static_cast< QgsHtmlAnnotation * >( item->annotation() );
    const QString file = annotation->sourceFile();
    if ( !file.isEmpty() )
    {
      mFileLineEdit->setText( file );
      mFileRadioButton->setChecked( true );
    }
    else
    {
      mHtmlSourceTextEdit->setText( annotation->htmlSource() );
      mSourceRadioButton->setChecked( true );
    }
  }

  QObject::connect( mButtonBox, &QDialogButtonBox::accepted, this, &QgsHtmlAnnotationDialog::applySettingsToItem );
  QObject::connect( mButtonBox, &QDialogButtonBox::helpRequested, this, &QgsHtmlAnnotationDialog::showHelp );
  QPushButton *deleteButton = new QPushButton( tr( "Delete" ) );
  QObject::connect( deleteButton, &QPushButton::clicked, this, &QgsHtmlAnnotationDialog::deleteItem );
  mButtonBox->addButton( deleteButton, QDialogButtonBox::RejectRole );

  QgsGui::enableAutoGeometryRestore( this );
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
    if ( mFileRadioButton->isChecked() )
    {
      annotation->setSourceFile( mFileLineEdit->text() );
    }
    else
    {
      annotation->setHtmlSource( mHtmlSourceTextEdit->text() );
    }
    mItem->update();
  }
}

void QgsHtmlAnnotationDialog::mBrowseToolButton_clicked()
{
  QString directory;
  const QFileInfo fi( mFileLineEdit->text() );
  if ( fi.exists() )
  {
    directory = fi.absolutePath();
  }
  else
  {
    directory = QDir::homePath();
  }
  const QString filename = QFileDialog::getOpenFileName( nullptr, tr( "html" ), directory, QStringLiteral( "HTML (*.html *.htm);;All files (*.*)" ) );
  mFileLineEdit->setText( filename );
}

void QgsHtmlAnnotationDialog::fileRadioButtonToggled( bool checked )
{
  mFileLineEdit->setEnabled( checked );
}

void QgsHtmlAnnotationDialog::sourceRadioButtonToggled( bool checked )
{
  mHtmlSourceTextEdit->setEnabled( checked );
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

void QgsHtmlAnnotationDialog::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "introduction/general_tools.html#annotation-tools" ) );
}
