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

#include <QCheckBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QEvent>
#include <QGroupBox>
#include <QLabel>
#include <QLayout>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QPainter>
#include <QScrollBar>
#include <QSplitter>
#include <QStackedWidget>
#include <QTimer>
#include <QTreeView>
#include <QTreeWidget>
#include <QAbstractItemModel>

#include "qgsfilterlineedit.h"
#include "qgsmessagebaritem.h"
#include "qgslogger.h"

QgsOptionsDialogBase::QgsOptionsDialogBase( const QString &settingsKey, QWidget *parent, Qt::WindowFlags fl, QgsSettings *settings )
  : QDialog( parent, fl )
  , mOptsKey( settingsKey )
  , mInit( false )
  , mIconOnly( false )
  , mSettings( settings )
  , mDelSettings( false )
{
}

QgsOptionsDialogBase::~QgsOptionsDialogBase()
{
  if ( mInit )
  {
    mSettings->setValue( QStringLiteral( "/Windows/%1/geometry" ).arg( mOptsKey ), saveGeometry() );
    mSettings->setValue( QStringLiteral( "/Windows/%1/splitState" ).arg( mOptsKey ), mOptSplitter->saveState() );
    mSettings->setValue( QStringLiteral( "/Windows/%1/tab" ).arg( mOptsKey ), mOptStackedWidget->currentIndex() );
  }

  if ( mDelSettings ) // local settings obj to delete
  {
    delete mSettings;
  }

  mSettings = nullptr; // null the pointer (in case of outside settings obj)
}

void QgsOptionsDialogBase::initOptionsBase( bool restoreUi, const QString &title )
{
  // use pointer to app QgsSettings if no custom QgsSettings specified
  // custom QgsSettings object may be from Python plugin
  mDelSettings = false;

  if ( !mSettings )
  {
    mSettings = new QgsSettings();
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
  mOptListWidget = findChild<QListWidget *>( QStringLiteral( "mOptionsListWidget" ) );
  QFrame *optionsFrame = findChild<QFrame *>( QStringLiteral( "mOptionsFrame" ) );
  mOptStackedWidget = findChild<QStackedWidget *>( QStringLiteral( "mOptionsStackedWidget" ) );
  mOptSplitter = findChild<QSplitter *>( QStringLiteral( "mOptionsSplitter" ) );
  mOptButtonBox = findChild<QDialogButtonBox *>( QStringLiteral( "buttonBox" ) );
  QFrame *buttonBoxFrame = findChild<QFrame *>( QStringLiteral( "mButtonBoxFrame" ) );
  mSearchLineEdit = findChild<QgsFilterLineEdit *>( QStringLiteral( "mSearchLineEdit" ) );

  if ( !mOptListWidget || !mOptStackedWidget || !mOptSplitter || !optionsFrame )
  {
    return;
  }

  int size = mSettings->value( QStringLiteral( "/IconSize" ), 24 ).toInt();
  // buffer size to match displayed icon size in toolbars, and expected geometry restore
  // newWidth (above) may need adjusted if you adjust iconBuffer here
  int iconBuffer = 4;
  mOptListWidget->setIconSize( QSize( size + iconBuffer, size + iconBuffer ) );
  mOptListWidget->setFrameStyle( QFrame::NoFrame );

  optionsFrame->layout()->setContentsMargins( 0, 3, 3, 3 );
  QVBoxLayout *layout = static_cast<QVBoxLayout *>( optionsFrame->layout() );

  if ( buttonBoxFrame )
  {
    buttonBoxFrame->layout()->setContentsMargins( 0, 0, 0, 0 );
    layout->insertWidget( layout->count() + 1, buttonBoxFrame );
  }
  else if ( mOptButtonBox )
  {
    layout->insertWidget( layout->count() + 1, mOptButtonBox );
  }

  if ( mOptButtonBox )
  {
    // enforce only one connection per signal, in case added in Qt Designer
    disconnect( mOptButtonBox, &QDialogButtonBox::accepted, this, &QDialog::accept );
    connect( mOptButtonBox, &QDialogButtonBox::accepted, this, &QDialog::accept );
    disconnect( mOptButtonBox, &QDialogButtonBox::rejected, this, &QDialog::reject );
    connect( mOptButtonBox, &QDialogButtonBox::rejected, this, &QDialog::reject );
  }
  connect( mOptSplitter, &QSplitter::splitterMoved, this, &QgsOptionsDialogBase::updateOptionsListVerticalTabs );
  connect( mOptStackedWidget, &QStackedWidget::currentChanged, this, &QgsOptionsDialogBase::optionsStackedWidget_CurrentChanged );
  connect( mOptStackedWidget, &QStackedWidget::widgetRemoved, this, &QgsOptionsDialogBase::optionsStackedWidget_WidgetRemoved );

  if ( mSearchLineEdit )
  {
    mSearchLineEdit->setShowSearchIcon( true );
    connect( mSearchLineEdit, &QgsFilterLineEdit::textChanged, this, &QgsOptionsDialogBase::searchText );
  }

  mInit = true;

  if ( restoreUi )
    restoreOptionsBaseUi( mDialogTitle );
}

void QgsOptionsDialogBase::setSettings( QgsSettings *settings )
{
  if ( mDelSettings ) // local settings obj to delete
  {
    delete mSettings;
  }

  mSettings = settings;
  mDelSettings = false; // don't delete outside obj
}

void QgsOptionsDialogBase::restoreOptionsBaseUi( const QString &title )
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

  restoreGeometry( mSettings->value( QStringLiteral( "/Windows/%1/geometry" ).arg( mOptsKey ) ).toByteArray() );
  // mOptListWidget width is fixed to take up less space in QtDesigner
  // revert it now unless the splitter's state hasn't been saved yet
  mOptListWidget->setMaximumWidth(
    mSettings->value( QStringLiteral( "/Windows/%1/splitState" ).arg( mOptsKey ) ).isNull() ? 150 : 16777215 );
  mOptSplitter->restoreState( mSettings->value( QStringLiteral( "/Windows/%1/splitState" ).arg( mOptsKey ) ).toByteArray() );
  int curIndx = mSettings->value( QStringLiteral( "/Windows/%1/tab" ).arg( mOptsKey ), 0 ).toInt();

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

void QgsOptionsDialogBase::searchText( const QString &text )
{
  mSearchLineEdit->setMinimumWidth( text.isEmpty() ? 0 : 70 );

  if ( !mOptStackedWidget )
    return;

  if ( mOptStackedWidget->isHidden() )
    mOptStackedWidget->show();
  if ( mOptButtonBox && mOptButtonBox->isHidden() )
    mOptButtonBox->show();
  // hide all page if text has to be search, show them all otherwise
  for ( int r = 0; r < mOptListWidget->count(); ++r )
  {
    mOptListWidget->setRowHidden( r, !text.isEmpty() );
  }

  for ( const QPair< QgsSearchHighlightOptionWidget *, int > &rsw : qgis::as_const( mRegisteredSearchWidgets ) )
  {
    rsw.first->reset();
    if ( !text.isEmpty() && rsw.first->searchHighlight( text ) )
    {
      QgsDebugMsgLevel( QString( "Found %1 in %2 (tab: %3)" )
                        .arg( text )
                        .arg( rsw.first->isValid() ? rsw.first->widget()->objectName() : "no widget" )
                        .arg( mOptListWidget->item( rsw.second )->text() ), 4 );
      mOptListWidget->setRowHidden( rsw.second, false );
    }
  }

  if ( mOptListWidget->isRowHidden( mOptStackedWidget->currentIndex() ) )
  {
    for ( int r = 0; r < mOptListWidget->count(); ++r )
    {
      if ( !mOptListWidget->isRowHidden( r ) )
      {
        mOptListWidget->setCurrentRow( r );
        return;
      }
    }

    // if no page can be shown, hide stack widget
    mOptStackedWidget->hide();
    if ( mOptButtonBox )
      mOptButtonBox->hide();
  }
}

void QgsOptionsDialogBase::registerTextSearchWidgets()
{
  mRegisteredSearchWidgets.clear();

  for ( int i = 0; i < mOptStackedWidget->count(); i++ )
  {
    Q_FOREACH ( QWidget *w, mOptStackedWidget->widget( i )->findChildren<QWidget *>() )
    {
      QgsSearchHighlightOptionWidget *shw = new QgsSearchHighlightOptionWidget( w );
      if ( shw->isValid() )
      {
        QgsDebugMsgLevel( QString( "Registering: %1" ).arg( w->objectName() ), 4 );
        mRegisteredSearchWidgets.append( qMakePair( shw, i ) );
      }
      else
      {
        delete shw;
      }
    }
  }
}

void QgsOptionsDialogBase::showEvent( QShowEvent *e )
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

  if ( mSearchLineEdit )
  {
    registerTextSearchWidgets();
  }

  QDialog::showEvent( e );
}

void QgsOptionsDialogBase::paintEvent( QPaintEvent *e )
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
    setWindowTitle( QStringLiteral( "%1 | %2" ).arg( mDialogTitle, curitem->text() ) );
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

void QgsOptionsDialogBase::optionsStackedWidget_CurrentChanged( int index )
{
  mOptListWidget->blockSignals( true );
  mOptListWidget->setCurrentRow( index );
  mOptListWidget->blockSignals( false );

  updateWindowTitle();
}

void QgsOptionsDialogBase::optionsStackedWidget_WidgetRemoved( int index )
{
  // will need to take item first, if widgets are set for item in future
  delete mOptListWidget->item( index );

  QList<QPair< QgsSearchHighlightOptionWidget *, int > >::iterator it = mRegisteredSearchWidgets.begin();
  while ( it != mRegisteredSearchWidgets.end() )
  {
    if ( ( *it ).second == index )
      it = mRegisteredSearchWidgets.erase( it );
    else
      ++it;
  }
}

void QgsOptionsDialogBase::warnAboutMissingObjects()
{
  QMessageBox::warning( nullptr, tr( "Missing objects" ),
                        tr( "Base options dialog could not be initialized.\n\n"
                            "Missing some of the .ui template objects:\n" )
                        + " mOptionsListWidget,\n mOptionsStackedWidget,\n mOptionsSplitter,\n mOptionsListFrame",
                        QMessageBox::Ok,
                        QMessageBox::Ok );
}


QgsSearchHighlightOptionWidget::QgsSearchHighlightOptionWidget( QWidget *widget )
  : QObject( widget )
  , mWidget( widget )
{
  QWidget *parent = widget;
  while ( ( parent = parent->parentWidget() ) )
  {
    // do not register message bar content, items disappear and causes QGIS to crash
    if ( qobject_cast< QgsMessageBarItem * >( parent ) )
    {
      mValid = false;
      return;
    }
  }

  QString styleSheet;
  if ( qobject_cast<QLabel *>( widget ) )
  {
    styleSheet = QStringLiteral( "QLabel { background-color: yellow; color: blue;}" );
    mTextFound = [ = ]( QString searchText ) {return qobject_cast<QLabel *>( mWidget )->text().contains( searchText, Qt::CaseInsensitive );};
  }
  else if ( qobject_cast<QCheckBox *>( widget ) )
  {
    styleSheet = QStringLiteral( "QCheckBox { background-color: yellow; color: blue;}" );
    mTextFound = [ = ]( QString searchText ) {return qobject_cast<QCheckBox *>( mWidget )->text().contains( searchText, Qt::CaseInsensitive );};
  }
  else if ( qobject_cast<QAbstractButton *>( widget ) )
  {
    styleSheet = QStringLiteral( "QAbstractButton { background-color: yellow; color: blue;}" );
    mTextFound = [ = ]( QString searchText ) {return qobject_cast<QAbstractButton *>( mWidget )->text().contains( searchText, Qt::CaseInsensitive );};
  }
  else if ( qobject_cast<QGroupBox *>( widget ) )
  {
    styleSheet = QStringLiteral( "QGroupBox::title { background-color: yellow; color: blue;}" );
    mTextFound = [ = ]( QString searchText ) {return qobject_cast<QGroupBox *>( mWidget )->title().contains( searchText, Qt::CaseInsensitive );};
  }
  if ( !styleSheet.isEmpty() )
  {
    styleSheet.prepend( "/*!search!*/" ).append( "/*!search!*/" );

    mHighlight = [ = ]( QString searchText )
    {
      Q_UNUSED( searchText );
      mWidget->setStyleSheet( mWidget->styleSheet() + styleSheet );
    };

    mReset = [ = ]()
    {
      if ( mWidget )
      {
        QString ss = mWidget->styleSheet();
        ss.remove( styleSheet );
        mWidget->setStyleSheet( ss );
      }
    };
  }
  else if ( qobject_cast<QTreeView *>( widget ) )
  {
    mTextFound = [ = ]( QString searchText )
    {
      QTreeView *treeView = qobject_cast<QTreeView *>( mWidget );
      if ( !treeView )
        return false;
      QModelIndexList hits = treeView->model()->match( treeView->model()->index( 0, 0 ), Qt::DisplayRole, searchText, 1, Qt::MatchContains | Qt::MatchRecursive );
      return !hits.isEmpty();
    };

    if ( qobject_cast<QTreeWidget *>( widget ) )
    {
      mHighlight = [ = ]( QString searchText )
      {
        QTreeWidget *treeWidget = qobject_cast<QTreeWidget *>( widget );
        if ( treeWidget )
        {
          QList<QTreeWidgetItem *> items = treeWidget->findItems( searchText, Qt::MatchContains | Qt::MatchRecursive, 0 );
          mChangedStyle = items.count() ? true : false;
          mTreeInitialBackground.clear();
          mTreeInitialExpand.clear();
          for ( QTreeWidgetItem *item : items )
          {
            mTreeInitialBackground.insert( item, item->background( 0 ) );
            item->setBackground( 0, QBrush( QColor( Qt::yellow ) ) );

            QTreeWidgetItem *parent = item;
            while ( ( parent = parent->parent() ) )
            {
              if ( mTreeInitialExpand.contains( parent ) )
                break;
              mTreeInitialExpand.insert( parent, parent->isExpanded() );
              parent->setExpanded( true );
            }
          }

        }
      };

      mReset = [ = ]()
      {
        for ( QTreeWidgetItem *item : mTreeInitialExpand.keys() )
        {
          if ( item )
          {
            item->setExpanded( mTreeInitialExpand.value( item ) );
          }
        }
        for ( QTreeWidgetItem *item : mTreeInitialBackground.keys() )
        {
          if ( item )
          {
            item->setBackground( 0, mTreeInitialBackground.value( item ) );
          }
        }
        mTreeInitialBackground.clear();
        mTreeInitialExpand.clear();
      };
    }
  }
  else
  {
    mValid = false;
  }

  if ( mValid )
  {
    connect( mWidget, &QWidget::destroyed, this, &QgsSearchHighlightOptionWidget::widgetDestroyed );
  }
}

bool QgsSearchHighlightOptionWidget::searchHighlight( const QString &searchText )
{
  mSearchText = searchText;
  bool found = false;
  if ( !mWidget )
    return found;

  if ( !searchText.isEmpty() )
  {
    found = mTextFound( searchText );
  }

  if ( found )
  {
    if ( mChangedStyle )
    {
      mReset();
      mChangedStyle = false;
    }
    if ( !mWidget->isVisible() )
    {
      // show the widget to get initial stylesheet in case it's modified
      QgsDebugMsg( QString( "installing event filter on: %1 (%2)" )
                   .arg( mWidget->objectName() )
                   .arg( qobject_cast<QLabel *>( mWidget ) ? qobject_cast<QLabel *>( mWidget )->text() : QString() ) );
      mWidget->installEventFilter( this );
      mInstalledFilter = true;
    }
    else
    {
      mHighlight( searchText );
      mChangedStyle = true;
    }
  }

  return found;
}

bool QgsSearchHighlightOptionWidget::eventFilter( QObject *obj, QEvent *event )
{
  if ( mInstalledFilter && event->type() == QEvent::Show && obj == mWidget )
  {
    mWidget->removeEventFilter( this );
    mInstalledFilter = false;
    // instead of catching the event and calling show again
    // it might be better to use a timer to change the style
    // after the widget is shown
#if 1
    mWidget->show();
    mHighlight( mSearchText );
    mChangedStyle = true;
    return true;
#else
    QTimer::singleShot( 500, this, [ = ]
    {
      mWidget->setStyleSheet( mWidget->styleSheet() + mStyleSheet );
      mChangedStyle = true;
    } );
#endif
  }
  return QObject::eventFilter( obj, event );
}

void QgsSearchHighlightOptionWidget::reset()
{
  if ( mWidget && mValid )
  {
    if ( mChangedStyle )
    {
      mReset();
      mChangedStyle = false;
    }
    if ( mInstalledFilter )
    {
      mWidget->removeEventFilter( this );
      mInstalledFilter = false;
    }
  }
}

void QgsSearchHighlightOptionWidget::widgetDestroyed()
{
  mWidget = nullptr;
  mValid = false;
}
