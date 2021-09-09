/***************************************************************************
    qgsundowidget.cpp
    ---------------------
    begin                : June 2009
    copyright            : (C) 2009 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsundowidget.h"

#include "qgsapplication.h"
#include "qgslogger.h"
#include "qgsmaplayer.h"
#include "qgsmapcanvas.h"


QgsUndoWidget::QgsUndoWidget( QWidget *parent, QgsMapCanvas *mapCanvas )
  : QgsPanelWidget( parent )
{
  setObjectName( QStringLiteral( "UndoWidget" ) );
  resize( 200, 223 );
  setMinimumSize( QSize( 200, 220 ) );
  mDockWidgetContents = new QWidget( this );
  mDockWidgetContents->setObjectName( QStringLiteral( "dockWidgetContents" ) );
  mGridLayout = new QGridLayout( mDockWidgetContents );
  mGridLayout->setObjectName( QStringLiteral( "gridLayout" ) );
  mGridLayout->setContentsMargins( 0, 0, 0, 0 );
  QSpacerItem *spacerItem = new QSpacerItem( 20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding );

  mGridLayout->addItem( spacerItem, 0, 0, 1, 1 );

  mUndoButton = new QPushButton( mDockWidgetContents );
  mUndoButton->setObjectName( QStringLiteral( "undoButton" ) );
  mUndoButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "mActionUndo.svg" ) ) );

  mGridLayout->addWidget( mUndoButton, 1, 0, 1, 1 );

  mRedoButton = new QPushButton( mDockWidgetContents );
  mRedoButton->setObjectName( QStringLiteral( "redoButton" ) );
  mRedoButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "mActionRedo.svg" ) ) );

  mGridLayout->addWidget( mRedoButton, 1, 1, 1, 1 );

  QSpacerItem *spacerItem1 = new QSpacerItem( 20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding );

  mGridLayout->addItem( spacerItem1, 0, 1, 1, 1 );

  setLayout( mGridLayout );

  setWindowTitle( QApplication::translate( "UndoWidget", "Undo/Redo" ) );
  mUndoButton->setText( QApplication::translate( "UndoWidget", "Undo" ) );
  mRedoButton->setText( QApplication::translate( "UndoWidget", "Redo" ) );

  connect( mUndoButton, &QAbstractButton::clicked, this, &QgsUndoWidget::undo );
  connect( mRedoButton, &QAbstractButton::clicked, this, &QgsUndoWidget::redo );

  mUndoButton->setDisabled( true );
  mRedoButton->setDisabled( true );
  mMapCanvas = mapCanvas;
  mUndoView = new QUndoView( this );
  mGridLayout->addWidget( mUndoView, 0, 0, 1, 2 );
}

void QgsUndoWidget::setButtonsVisible( bool show )
{
  mUndoButton->setVisible( show );
  mRedoButton->setVisible( show );
}

void QgsUndoWidget::unsetStack()
{
  // we don't have ownership of the stack, don't delete it!
  mUndoStack = nullptr;

  if ( mUndoView )
  {
    mUndoView->close();
    delete mUndoView;
    mUndoView = new QUndoView( mDockWidgetContents );
    mGridLayout->addWidget( mUndoView, 0, 0, 1, 2 );
  }
}

void QgsUndoWidget::undoChanged( bool value )
{
  mUndoButton->setDisabled( !value );
  emit undoStackChanged();
}

void QgsUndoWidget::redoChanged( bool value )
{
  mRedoButton->setDisabled( !value );
  emit undoStackChanged();
}

void QgsUndoWidget::indexChanged( int curIndx )
{
  // this is called twice when a non-current command is clicked in QUndoView
  //   first call has offset, second call will have offset of 0
  int curCount = 0;
  bool canRedo = true;
  if ( mUndoStack )
  {
    canRedo = mUndoStack->canRedo();
    curCount = mUndoStack->count();
  }
  const int offset = std::abs( mPreviousIndex - curIndx );

  // when individually redoing, differentiate between last redo and a new command added to stack
  const bool lastRedo = ( mPreviousIndex == ( mPreviousCount - 1 ) && mPreviousCount == curCount && !canRedo );

  if ( offset != 0 )
  {
    QgsDebugMsgLevel( QStringLiteral( "curIndx : %1" ).arg( curIndx ), 4 );
    QgsDebugMsgLevel( QStringLiteral( "offset  : %1" ).arg( offset ), 4 );
    QgsDebugMsgLevel( QStringLiteral( "curCount: %1" ).arg( curCount ), 4 );
    if ( lastRedo )
    {
      QgsDebugMsgLevel( QStringLiteral( "lastRedo: true" ), 4 );
    }
  }

  // avoid canvas redraws when only new command was added to stack (i.e. no user undo/redo action)
  // or when user has clicked back in QUndoView history then added a new command to the stack
  if ( offset > 1 || ( offset == 1 && ( canRedo || lastRedo ) ) )
  {
    if ( mMapCanvas )
    {
      mMapCanvas->refresh();
    }
  }

  mPreviousIndex = curIndx;
  mPreviousCount = curCount;
}

void QgsUndoWidget::undo()
{
  if ( mUndoStack )
    mUndoStack->undo();
}

void QgsUndoWidget::redo()
{
  if ( mUndoStack )
    mUndoStack->redo();
}

void QgsUndoWidget::setUndoStack( QUndoStack *undoStack )
{
  if ( mUndoView )
  {
    mUndoView->close();
    delete mUndoView;
    mUndoView = nullptr;
  }

  mUndoStack = undoStack;
  mPreviousIndex = mUndoStack->index();
  mPreviousCount = mUndoStack->count();

  mUndoView = new QUndoView( mDockWidgetContents );
  mUndoView->setStack( undoStack );
  mUndoView->setObjectName( QStringLiteral( "undoView" ) );
  mGridLayout->addWidget( mUndoView, 0, 0, 1, 2 );
//  setWidget( dockWidgetContents );
  connect( mUndoStack, &QUndoStack::canUndoChanged, this, &QgsUndoWidget::undoChanged );
  connect( mUndoStack, &QUndoStack::canRedoChanged, this, &QgsUndoWidget::redoChanged );

  // gets triggered also when a new command is added to stack, and twice when clicking a command in QUndoView
  connect( mUndoStack, &QUndoStack::indexChanged, this, &QgsUndoWidget::indexChanged );

  mUndoButton->setDisabled( !mUndoStack->canUndo() );
  mRedoButton->setDisabled( !mUndoStack->canRedo() );
}

