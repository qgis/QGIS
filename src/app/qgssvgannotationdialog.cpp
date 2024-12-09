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
#include "moc_qgssvgannotationdialog.cpp"
#include "qgsannotationwidget.h"
#include "qgssvgannotation.h"
#include "qgsmapcanvasannotationitem.h"
#include "qgsproject.h"
#include "qgsannotationmanager.h"
#include "qgsgui.h"
#include "qgshelp.h"
#include "qgssettingsentryimpl.h"

#include <QFileDialog>
#include <QFileInfo>
#include <QGraphicsScene>
#include <QPushButton>

QgsSvgAnnotationDialog::QgsSvgAnnotationDialog( QgsMapCanvasAnnotationItem *item, QWidget *parent, Qt::WindowFlags f )
  : QDialog( parent, f )
  , mItem( item )

{
  setupUi( this );
  connect( mBrowseToolButton, &QToolButton::clicked, this, &QgsSvgAnnotationDialog::mBrowseToolButton_clicked );
  connect( mButtonBox, &QDialogButtonBox::clicked, this, &QgsSvgAnnotationDialog::mButtonBox_clicked );
  setWindowTitle( tr( "SVG Annotation" ) );
  mEmbeddedWidget = new QgsAnnotationWidget( mItem );
  mStackedWidget->addWidget( mEmbeddedWidget );
  mStackedWidget->setCurrentWidget( mEmbeddedWidget );

  // SVG annotation can only be created from an svg file
  // Mask the source radio button and the source text edit
  mFileRadioButton->setChecked( true );
  mFileRadioButton->hide();
  mSourceRadioButton->hide();
  mHtmlSourceTextEdit->hide();

  if ( mItem && mItem->annotation() )
  {
    QgsSvgAnnotation *annotation = static_cast<QgsSvgAnnotation *>( mItem->annotation() );
    mFileLineEdit->setText( annotation->filePath() );
  }

  QObject::connect( mButtonBox, &QDialogButtonBox::accepted, this, &QgsSvgAnnotationDialog::applySettingsToItem );
  QObject::connect( mButtonBox, &QDialogButtonBox::helpRequested, this, &QgsSvgAnnotationDialog::showHelp );
  QPushButton *deleteButton = new QPushButton( tr( "Delete" ) );
  QObject::connect( deleteButton, &QPushButton::clicked, this, &QgsSvgAnnotationDialog::deleteItem );
  mButtonBox->addButton( deleteButton, QDialogButtonBox::RejectRole );

  connect( mLiveCheckBox, &QCheckBox::toggled, this, &QgsSvgAnnotationDialog::onLiveUpdateToggled );
  mLiveCheckBox->setChecked( QgsAnnotationWidget::settingLiveUpdate->value() );
  connect( mEmbeddedWidget, &QgsAnnotationWidget::changed, this, &QgsSvgAnnotationDialog::onSettingsChanged );
  connect( mFileLineEdit, &QLineEdit::textChanged, this, &QgsSvgAnnotationDialog::onSettingsChanged );
  connect( mLiveCheckBox, &QCheckBox::toggled, this, &QgsSvgAnnotationDialog::onSettingsChanged );

  QgsGui::enableAutoGeometryRestore( this );
}

void QgsSvgAnnotationDialog::mBrowseToolButton_clicked()
{
  QString directory;
  const QFileInfo fi( mFileLineEdit->text() );
  if ( fi.exists() )
  {
    directory = fi.absolutePath();
  }
  const QString filename = QFileDialog::getOpenFileName( nullptr, tr( "Select SVG file" ), directory, tr( "SVG files" ) + " (*.svg)" );
  if ( filename.isEmpty() )
  {
    return;
  }
  mFileLineEdit->setText( filename );
}

void QgsSvgAnnotationDialog::applySettingsToItem()
{
  if ( mEmbeddedWidget )
  {
    mEmbeddedWidget->apply();
  }

  if ( mItem && mItem->annotation() )
  {
    if ( !mFileLineEdit->text().isEmpty() )
    {
      QgsSvgAnnotation *annotation = static_cast<QgsSvgAnnotation *>( mItem->annotation() );
      annotation->setFilePath( mFileLineEdit->text() );
      mItem->update();
    }
  }
}

void QgsSvgAnnotationDialog::deleteItem()
{
  if ( mItem && mItem->annotation() )
    QgsProject::instance()->annotationManager()->removeAnnotation( mItem->annotation() );
  mItem = nullptr;
}

void QgsSvgAnnotationDialog::mButtonBox_clicked( QAbstractButton *button )
{
  if ( mButtonBox->buttonRole( button ) == QDialogButtonBox::ApplyRole )
  {
    applySettingsToItem();
  }
}

void QgsSvgAnnotationDialog::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "map_views/map_view.html#sec-annotations" ) );
}

void QgsSvgAnnotationDialog::onSettingsChanged()
{
  if ( mLiveCheckBox->isChecked() )
  {
    applySettingsToItem();
  }
}

void QgsSvgAnnotationDialog::onLiveUpdateToggled( bool checked )
{
  // Apply and Cancel buttons make no sense when live update is on
  mButtonBox->button( QDialogButtonBox::Apply )->setHidden( checked );
  mButtonBox->button( QDialogButtonBox::Cancel )->setHidden( checked );
  QgsAnnotationWidget::settingLiveUpdate->setValue( checked );
}
