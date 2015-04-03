/***************************************************************************
    qgsoptionsdialogbase.cpp - base vertical tabs option dialog

    ---------------------
    begin                : March 24, 2013
    copyright            : (C) 2013 by Larry Shaffer
    email                : larrys at dakcarto dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsoptionsdialogbase.h"

#include <QDialog>
#include <QDialogButtonBox>
#include <QLayout>
#include <QListWidget>
#include <QMessageBox>
#include <QScrollBar>
#include <QStackedWidget>
#include <QSplitter>
#include <QTimer>


QgsOptionsDialogBase::QgsOptionsDialogBase( QString settingsKey, QWidget* parent, Qt::WindowFlags fl, QSettings* settings )
    : QDialog( parent, fl )
    , mOptsKey( settingsKey )
    , mInit( false )
    , mOptListWidget( NULL )
    , mOptStackedWidget( NULL )
    , mOptSplitter( NULL )
    , mOptButtonBox( NULL )
    , mDialogTitle( "" )
    , mIconOnly( false )
    , mSettings( settings )
    , mDelSettings( false )
{
}

QgsOptionsDialogBase::~QgsOptionsDialogBase()
{
  if ( mInit )
  {
    mSettings->setValue( QString( "/Windows/%1/geometry" ).arg( mOptsKey ), saveGeometry() );
    mSettings->setValue( QString( "/Windows/%1/splitState" ).arg( mOptsKey ), mOptSplitter->saveState() );
    mSettings->setValue( QString( "/Windows/%1/tab" ).arg( mOptsKey ), mOptStackedWidget->currentIndex() );
  }

  if ( mDelSettings ) // local settings obj to delete
  {
    delete mSettings;
  }

  mSettings = 0; // null the pointer (in case of outside settings obj)
}

void QgsOptionsDialogBase::initOptionsBase( bool restoreUi, QString title )
{
  // use pointer to app QSettings if no custom QSettings specified
  // custom QSettings object may be from Python plugin
  mDelSettings = false;

  if ( !mSettings )
  {
    mSettings = new QSettings();
    mDelSettings = true; // only delete obj created by class
  }

  // save dialog title so it can be used to be concatenated
  // with category title in icon-only mode
  if ( title.isEmpty() )
    mDialogTitle = windowTitle();
  else
    mDialogTitle = title;

  // don't add to dialog margins
  // redefine now, or those in inherited .ui file will be added
  if ( layout() )
  {
    layout()->setContentsMargins( 0, 0, 0, 0 ); // Qt default spacing
  }

  // start with copy of qgsoptionsdialog_template.ui to ensure existence of these objects
  mOptListWidget = findChild<QListWidget*>( "mOptionsListWidget" );
  QFrame* optionsFrame = findChild<QFrame*>( "mOptionsFrame" );
  mOptStackedWidget = findChild<QStackedWidget*>( "mOptionsStackedWidget" );
  mOptSplitter = findChild<QSplitter*>( "mOptionsSplitter" );
  mOptButtonBox = findChild<QDialogButtonBox*>( "buttonBox" );
  QFrame* buttonBoxFrame = findChild<QFrame*>( "mButtonBoxFrame" );

  if ( !mOptListWidget || !mOptStackedWidget || !mOptSplitter || !optionsFrame )
  {
    return;
  }

  int size = mSettings->value( "/IconSize", 24 ).toInt();
  // buffer size to match displayed icon size in toolbars, and expected geometry restore
  // newWidth (above) may need adjusted if you adjust iconBuffer here
  int iconBuffer = 4;
  mOptListWidget->setIconSize( QSize( size + iconBuffer, size + iconBuffer ) );
  mOptListWidget->setFrameStyle( QFrame::NoFrame );

  optionsFrame->layout()->setContentsMargins( 0, 3, 3, 3 );
  QVBoxLayout* layout = static_cast<QVBoxLayout*>( optionsFrame->layout() );

  if ( buttonBoxFrame )
  {
    buttonBoxFrame->layout()->setContentsMargins( 0, 0, 0, 0 );
    layout->insertWidget( layout->count() + 1, buttonBoxFrame );
  }
  else
  {
    layout->insertWidget( layout->count() + 1, mOptButtonBox );
  }

  if ( mOptButtonBox )
  {
    // enforce only one connection per signal, in case added in Qt Designer
    disconnect( mOptButtonBox, SIGNAL( accepted() ), this, SLOT( accept() ) );
    connect( mOptButtonBox, SIGNAL( accepted() ), this, SLOT( accept() ) );
    disconnect( mOptButtonBox, SIGNAL( rejected() ), this, SLOT( reject() ) );
    connect( mOptButtonBox, SIGNAL( rejected() ), this, SLOT( reject() ) );
  }
  connect( mOptSplitter, SIGNAL( splitterMoved( int, int ) ), this, SLOT( updateOptionsListVerticalTabs() ) );
  connect( mOptStackedWidget, SIGNAL( currentChanged( int ) ), this, SLOT( optionsStackedWidget_CurrentChanged( int ) ) );
  connect( mOptStackedWidget, SIGNAL( widgetRemoved( int ) ), this, SLOT( optionsStackedWidget_WidgetRemoved( int ) ) );

  mInit = true;

  if ( restoreUi )
    restoreOptionsBaseUi( mDialogTitle );
}

void QgsOptionsDialogBase::setSettings( QSettings* settings )
{
  if ( mDelSettings ) // local settings obj to delete
  {
    delete mSettings;
  }

  mSettings = settings;
  mDelSettings = false; // don't delete outside obj
}

void QgsOptionsDialogBase::restoreOptionsBaseUi( QString title )
{
  if ( !mInit )
  {
    return;
  }

  if ( !title.isEmpty() )
  {
    mDialogTitle = title;
    updateWindowTitle();
  }

  // re-save original dialog title in case it was changed after dialog initialization
  mDialogTitle = windowTitle();

  restoreGeometry( mSettings->value( QString( "/Windows/%1/geometry" ).arg( mOptsKey ) ).toByteArray() );
  // mOptListWidget width is fixed to take up less space in QtDesigner
  // revert it now unless the splitter's state hasn't been saved yet
  mOptListWidget->setMaximumWidth(
    mSettings->value( QString( "/Windows/%1/splitState" ).arg( mOptsKey ) ).isNull() ? 150 : 16777215 );
  mOptSplitter->restoreState( mSettings->value( QString( "/Windows/%1/splitState" ).arg( mOptsKey ) ).toByteArray() );
  int curIndx = mSettings->value( QString( "/Windows/%1/tab" ).arg( mOptsKey ), 0 ).toInt();

  // if the last used tab is out of range or not enabled display the first enabled one
  if ( mOptStackedWidget->count() < ( curIndx + 1 )
       || !mOptStackedWidget->widget( curIndx )->isEnabled() )
  {
    curIndx = 0;
    for ( int i = 0; i < mOptStackedWidget->count(); i++ )
    {
      if ( mOptStackedWidget->widget( i )->isEnabled() )
      {
        curIndx = i;
        break;
      }
    }
  }

  if ( mOptStackedWidget->count() != 0 && mOptListWidget->count() != 0 )
  {
    mOptStackedWidget->setCurrentIndex( curIndx );
    mOptListWidget->setCurrentRow( curIndx );
  }

  // get rid of annoying outer focus rect on Mac
  mOptListWidget->setAttribute( Qt::WA_MacShowFocusRect, false );
}

void QgsOptionsDialogBase::showEvent( QShowEvent* e )
{
  if ( mInit )
  {
    updateOptionsListVerticalTabs();
    optionsStackedWidget_CurrentChanged( mOptListWidget->currentRow() );
  }
  else
  {
    QTimer::singleShot( 0, this, SLOT( warnAboutMissingObjects() ) );
  }

  QDialog::showEvent( e );
}

void QgsOptionsDialogBase::paintEvent( QPaintEvent* e )
{
  if ( mInit )
    QTimer::singleShot( 0, this, SLOT( updateOptionsListVerticalTabs() ) );

  QDialog::paintEvent( e );
}

void QgsOptionsDialogBase::updateWindowTitle()
{
  QListWidgetItem *curitem = mOptListWidget->currentItem();
  if ( curitem )
  {
    setWindowTitle( QString( "%1 | %2" ).arg( mDialogTitle ).arg( curitem->text() ) );
  }
  else
  {
    setWindowTitle( mDialogTitle );
  }
}

void QgsOptionsDialogBase::updateOptionsListVerticalTabs()
{
  if ( !mInit )
    return;

  if ( mOptListWidget->maximumWidth() != 16777215 )
    mOptListWidget->setMaximumWidth( 16777215 );
  // auto-resize splitter for vert scrollbar without covering icons in icon-only mode
  // TODO: mOptListWidget has fixed 32px wide icons for now, allow user-defined
  // Note: called on splitter resize and dialog paint event, so only update when necessary
  int iconWidth = mOptListWidget->iconSize().width();
  int snapToIconWidth = iconWidth + 32;

  QList<int> splitSizes = mOptSplitter->sizes();
  mIconOnly = ( splitSizes.at( 0 ) <= snapToIconWidth );

  // iconBuffer (above) may need adjusted if you adjust iconWidth here
  int newWidth = mOptListWidget->verticalScrollBar()->isVisible() ? iconWidth + 22 : iconWidth + 9;
  bool diffWidth = mOptListWidget->minimumWidth() != newWidth;

  if ( diffWidth )
    mOptListWidget->setMinimumWidth( newWidth );

  if ( mIconOnly && ( diffWidth || mOptListWidget->width() != newWidth ) )
  {
    splitSizes[1] = splitSizes.at( 1 ) - ( splitSizes.at( 0 ) - newWidth );
    splitSizes[0] = newWidth;
    mOptSplitter->setSizes( splitSizes );
  }

  if ( mOptListWidget->wordWrap() && mIconOnly )
    mOptListWidget->setWordWrap( false );
  if ( !mOptListWidget->wordWrap() && !mIconOnly )
    mOptListWidget->setWordWrap( true );
}

void QgsOptionsDialogBase::optionsStackedWidget_CurrentChanged( int indx )
{
  mOptListWidget->blockSignals( true );
  mOptListWidget->setCurrentRow( indx );
  mOptListWidget->blockSignals( false );

  updateWindowTitle();
}

void QgsOptionsDialogBase::optionsStackedWidget_WidgetRemoved( int indx )
{
  // will need to take item first, if widgets are set for item in future
  delete mOptListWidget->item( indx );
}

void QgsOptionsDialogBase::warnAboutMissingObjects()
{
  QMessageBox::warning( 0, tr( "Missing objects" ),
                        tr( "Base options dialog could not be initialized.\n\n"
                            "Missing some of the .ui template objects:\n" )
                        + " mOptionsListWidget,\n mOptionsStackedWidget,\n mOptionsSplitter",
                        QMessageBox::Ok,
                        QMessageBox::Ok );
}
