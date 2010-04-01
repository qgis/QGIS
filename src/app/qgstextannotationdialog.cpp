/***************************************************************************
                              qgstextannotationdialog.cpp
                              ---------------------------
  begin                : February 24, 2010
  copyright            : (C) 2010 by Marco Hugentobler
  email                : marco dot hugentobler at hugis dot net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstextannotationdialog.h"
#include "qgsannotationwidget.h"
#include "qgstextannotationitem.h"
#include <QColorDialog>
#include <QGraphicsScene>

QgsTextAnnotationDialog::QgsTextAnnotationDialog( QgsTextAnnotationItem* item, QWidget * parent, Qt::WindowFlags f ): QDialog( parent, f ), mItem( item ), mTextDocument( 0 )
{
  setupUi( this );
  mEmbeddedWidget = new QgsAnnotationWidget( mItem );
  mEmbeddedWidget->show();
  mStackedWidget->addWidget( mEmbeddedWidget );
  mStackedWidget->setCurrentWidget( mEmbeddedWidget );
  if ( mItem )
  {
    mTextDocument = mItem->document();
    mTextEdit->setDocument( mTextDocument );
    mBackgroundColorButton->setColor( mItem->frameBackgroundColor() );
  }
  setCurrentFontPropertiesToGui();

  QObject::connect( mButtonBox, SIGNAL( accepted() ), this, SLOT( applyTextToItem() ) );
  QObject::connect( mFontComboBox, SIGNAL( currentFontChanged( const QFont& ) ), this, SLOT( changeCurrentFormat() ) );
  QObject::connect( mFontSizeSpinBox, SIGNAL( valueChanged( int ) ), this, SLOT( changeCurrentFormat() ) );
  QObject::connect( mBoldPushButton, SIGNAL( toggled( bool ) ), this, SLOT( changeCurrentFormat() ) );
  QObject::connect( mItalicsPushButton, SIGNAL( toggled( bool ) ), this, SLOT( changeCurrentFormat() ) );
  QObject::connect( mTextEdit, SIGNAL( cursorPositionChanged() ), this, SLOT( setCurrentFontPropertiesToGui() ) );

  QObject::connect( mButtonBox, SIGNAL( accepted() ), this, SLOT( applySettingsToItem() ) );
  QPushButton* deleteButton = new QPushButton( tr( "Delete" ) );
  QObject::connect( deleteButton, SIGNAL( clicked() ), this, SLOT( deleteItem() ) );
  mButtonBox->addButton( deleteButton, QDialogButtonBox::RejectRole );
}

QgsTextAnnotationDialog::~QgsTextAnnotationDialog()
{
  delete mTextDocument;
}

void QgsTextAnnotationDialog::applyTextToItem()
{
  if ( mItem && mTextDocument )
  {
    //apply settings from embedded item widget
    if ( mEmbeddedWidget )
    {
      mEmbeddedWidget->apply();
    }
    mItem->setDocument( mTextDocument );
    mItem->update();
  }
}

void QgsTextAnnotationDialog::changeCurrentFormat()
{
  QFont newFont;
  newFont.setFamily( mFontComboBox->currentFont().family() );

  //bold
  if ( mBoldPushButton->isChecked() )
  {
    newFont.setBold( true );
  }
  else
  {
    newFont.setBold( false );
  }

  //italic
  if ( mItalicsPushButton->isChecked() )
  {
    newFont.setItalic( true );
  }
  else
  {
    newFont.setItalic( false );
  }

  //size
  newFont.setPointSize( mFontSizeSpinBox->value() );
  mTextEdit->setCurrentFont( newFont );

  //color
  mTextEdit->setTextColor( mFontColorButton->color() );
}

void QgsTextAnnotationDialog::on_mFontColorButton_clicked()
{
#if QT_VERSION >= 0x040500
  QColor newColor = QColorDialog::getColor( mFontColorButton->color(), 0, tr( "Select font color" ), QColorDialog::ShowAlphaChannel );
#else
  QColor newColor = QColorDialog::getColor( mFontColorButton->color() );
#endif
  if ( !newColor.isValid() )
  {
    return; //dialog canceled
  }
  mFontColorButton->setColor( newColor );
  changeCurrentFormat();
}

void QgsTextAnnotationDialog::setCurrentFontPropertiesToGui()
{
  blockAllSignals( true );
  QFont currentFont = mTextEdit->currentFont();
  mFontComboBox->setCurrentFont( currentFont );
  mFontSizeSpinBox->setValue( currentFont.pointSize() );
  mBoldPushButton->setChecked( currentFont.bold() );
  mItalicsPushButton->setChecked( currentFont.italic() );
  mFontColorButton->setColor( mTextEdit->textColor() );
  blockAllSignals( false );
}

void QgsTextAnnotationDialog::blockAllSignals( bool block )
{
  mFontComboBox->blockSignals( block );
  mFontSizeSpinBox->blockSignals( block );
  mBoldPushButton->blockSignals( block );
  mItalicsPushButton->blockSignals( block );
  mFontColorButton->blockSignals( block );
}

void QgsTextAnnotationDialog::deleteItem()
{
  QGraphicsScene* scene = mItem->scene();
  if ( scene )
  {
    scene->removeItem( mItem );
  }
  delete mItem;
  mItem = 0;
}

void QgsTextAnnotationDialog::on_mBackgroundColorButton_clicked()
{
  if ( !mItem )
  {
    return;
  }

  QColor bgColor;
#if QT_VERSION >= 0x040500
  bgColor = QColorDialog::getColor( mItem->frameBackgroundColor(), 0, tr( "Select background color" ), QColorDialog::ShowAlphaChannel );
#else
  bgColor = QColorDialog::getColor( mItem->frameBackgroundColor() );
#endif

  if ( bgColor.isValid() )
  {
    mItem->setFrameBackgroundColor( bgColor );
    mBackgroundColorButton->setColor( bgColor );
  }
}

