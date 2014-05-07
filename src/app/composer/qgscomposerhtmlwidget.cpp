/***************************************************************************
    qgscomposerhtmlwidget.cpp
    ---------------------
    begin                : July 2012
    copyright            : (C) 2012 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgscomposerhtmlwidget.h"
#include "qgscomposerframe.h"
#include "qgscomposeritemwidget.h"
#include "qgscomposermultiframecommand.h"
#include "qgscomposerhtml.h"
#include "qgscomposition.h"
#include <QFileDialog>
#include <QSettings>

QgsComposerHtmlWidget::QgsComposerHtmlWidget( QgsComposerHtml* html, QgsComposerFrame* frame ): QWidget(), mHtml( html ), mFrame( frame )
{
  setupUi( this );

  blockSignals( true );
  mResizeModeComboBox->addItem( tr( "Use existing frames" ), QgsComposerMultiFrame::UseExistingFrames );
  mResizeModeComboBox->addItem( tr( "Extend to next page" ), QgsComposerMultiFrame::ExtendToNextPage );
  mResizeModeComboBox->addItem( tr( "Repeat on every page" ), QgsComposerMultiFrame::RepeatOnEveryPage );
  mResizeModeComboBox->addItem( tr( "Repeat until finished" ), QgsComposerMultiFrame::RepeatUntilFinished );
  blockSignals( false );
  setGuiElementValues();

  if ( mHtml )
  {
    QObject::connect( mHtml, SIGNAL( changed() ), this, SLOT( setGuiElementValues() ) );
  }

  //embed widget for general options
  if ( mFrame )
  {
    //add widget for general composer item properties
    QgsComposerItemWidget* itemPropertiesWidget = new QgsComposerItemWidget( this, mFrame );
    mainLayout->addWidget( itemPropertiesWidget );
  }
}

QgsComposerHtmlWidget::QgsComposerHtmlWidget()
{
}

QgsComposerHtmlWidget::~QgsComposerHtmlWidget()
{
}

void QgsComposerHtmlWidget::blockSignals( bool block )
{
  mUrlLineEdit->blockSignals( block );
  mFileToolButton->blockSignals( block );
  mResizeModeComboBox->blockSignals( block );
  mUseSmartBreaksCheckBox->blockSignals( block );
  mMaxDistanceSpinBox->blockSignals( block );
}

void QgsComposerHtmlWidget::on_mUrlLineEdit_editingFinished()
{
  if ( mHtml )
  {
    QUrl newUrl( mUrlLineEdit->text() );
    if ( newUrl == mHtml->url() )
    {
      return;
    }

    QgsComposition* composition = mHtml->composition();
    if ( composition )
    {
      composition->beginMultiFrameCommand( mHtml, tr( "Change html url" ) );
      mHtml->setUrl( newUrl );
      mHtml->update();
      composition->endMultiFrameCommand();
    }
  }
}

void QgsComposerHtmlWidget::on_mFileToolButton_clicked()
{
  QSettings s;
  QString lastDir = s.value( "/UI/lastHtmlDir", "" ).toString();
  QString file = QFileDialog::getOpenFileName( this, tr( "Select HTML document" ), lastDir, "HTML (*.html *.htm);;All files (*.*)" );
  if ( !file.isEmpty() )
  {
    QUrl url = QUrl::fromLocalFile( file );
    mUrlLineEdit->setText( url.toString() );
    on_mUrlLineEdit_editingFinished();
    mHtml->update();
    s.setValue( "/UI/lastHtmlDir", QFileInfo( file ).absolutePath() );
  }
}

void QgsComposerHtmlWidget::on_mResizeModeComboBox_currentIndexChanged( int index )
{
  if ( !mHtml )
  {
    return;
  }

  QgsComposition* composition = mHtml->composition();
  if ( composition )
  {
    composition->beginMultiFrameCommand( mHtml, tr( "Change resize mode" ) );
    mHtml->setResizeMode(( QgsComposerMultiFrame::ResizeMode )mResizeModeComboBox->itemData( index ).toInt() );
    composition->endMultiFrameCommand();
  }

  mAddFramePushButton->setEnabled( mHtml->resizeMode() == QgsComposerMultiFrame::UseExistingFrames );
}

void QgsComposerHtmlWidget::on_mUseSmartBreaksCheckBox_toggled( bool checked )
{
  if ( !mHtml )
  {
    return;
  }

  QgsComposition* composition = mHtml->composition();
  if ( composition )
  {
    blockSignals( true );
    composition->beginMultiFrameCommand( mHtml, tr( "Use smart breaks changed" ) );
    mHtml->setUseSmartBreaks( checked );
    composition->endMultiFrameCommand();
    blockSignals( false );
  }
}

void QgsComposerHtmlWidget::on_mMaxDistanceSpinBox_valueChanged( double val )
{
  if ( !mHtml )
  {
    return;
  }

  mHtml->setMaxBreakDistance( val );
}

void QgsComposerHtmlWidget::on_mReloadPushButton_clicked()
{
  if ( !mHtml )
  {
    return;
  }

  mHtml->loadHtml();
}

void QgsComposerHtmlWidget::on_mAddFramePushButton_clicked()
{
  if ( !mHtml || !mFrame )
  {
    return;
  }

  //create a new frame based on the current frame
  QPointF pos = mFrame->pos();
  //shift new frame so that it sits 10 units below current frame
  pos.ry() += mFrame->rect().height() + 10;

  QgsComposerFrame * newFrame = mHtml->createNewFrame( mFrame, pos, mFrame->rect().size() );
  mHtml->recalculateFrameSizes();

  //set new frame as selection
  QgsComposition* composition = mHtml->composition();
  if ( composition )
  {
    composition->setSelectedItem( newFrame );
  }
}

void QgsComposerHtmlWidget::setGuiElementValues()
{
  if ( !mHtml )
  {
    return;
  }

  blockSignals( true );
  mUrlLineEdit->setText( mHtml->url().toString() );
  mResizeModeComboBox->setCurrentIndex( mResizeModeComboBox->findData( mHtml->resizeMode() ) );
  mUseSmartBreaksCheckBox->setChecked( mHtml->useSmartBreaks() );
  mMaxDistanceSpinBox->setValue( mHtml->maxBreakDistance() );

  mAddFramePushButton->setEnabled( mHtml->resizeMode() == QgsComposerMultiFrame::UseExistingFrames );
  blockSignals( false );
}
