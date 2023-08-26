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
#include <QListWidgetItem>
#include <QMessageBox>
#include <QPainter>
#include <QScrollBar>
#include <QSplitter>
#include <QStackedWidget>
#include <QTimer>
#include <QStandardItem>
#include <QTreeView>
#include <QHeaderView>
#if QT_VERSION < QT_VERSION_CHECK(6, 5, 0)
#include <QScreen>
#endif
#include <functional>

#include "qgsfilterlineedit.h"
#include "qgslogger.h"
#include "qgsoptionsdialoghighlightwidget.h"
#include "qgsoptionswidgetfactory.h"
#include "qgsguiutils.h"
#include "qgsapplication.h"
#include "qgsvariantutils.h"



#if QT_VERSION < QT_VERSION_CHECK(6, 5, 0)
static void checkRestoredGeometry( const QRect &availableGeometry, QRect *restoredGeometry,
                                   int frameHeight )
{
  // compare with restored geometry's height increased by frameHeight
  const int height = restoredGeometry->height() + frameHeight;

  // Step 1: Resize if necessary:
  // make height / width 2px smaller than screen, because an exact match would be fullscreen
  if ( availableGeometry.height() <= height )
    restoredGeometry->setHeight( availableGeometry.height() - 2 - frameHeight );
  if ( availableGeometry.width() <= restoredGeometry->width() )
    restoredGeometry->setWidth( availableGeometry.width() - 2 );

  // Step 2: Move if necessary:
  // Construct a rectangle from restored Geometry adjusted by frameHeight
  const QRect restored = restoredGeometry->adjusted( 0, -frameHeight, 0, 0 );

  // Return if restoredGeometry (including frame) fits into screen
  if ( availableGeometry.contains( restored ) )
    return;

  // (size is correct, but at least one edge is off screen)

  // Top out of bounds => move down
  if ( restored.top() <= availableGeometry.top() )
  {
    restoredGeometry->moveTop( availableGeometry.top() + 1 + frameHeight );
  }
  else if ( restored.bottom() >= availableGeometry.bottom() )
  {
    // Bottom out of bounds => move up
    restoredGeometry->moveBottom( availableGeometry.bottom() - 1 );
  }

  // Left edge out of bounds => move right
  if ( restored.left() <= availableGeometry.left() )
  {
    restoredGeometry->moveLeft( availableGeometry.left() + 1 );
  }
  else if ( restored.right() >= availableGeometry.right() )
  {
    // Right edge out of bounds => move left
    restoredGeometry->moveRight( availableGeometry.right() - 1 );
  }
}


QSize QgsOptionsDialogBase::adjustedSize() const
{
  QSize s = sizeHint();

  if ( isWindow() )
  {
    Qt::Orientations exp;
    if ( layout() )
    {
      if ( layout()->hasHeightForWidth() )
        s.setHeight( layout()->totalHeightForWidth( s.width() ) );
      exp = layout()->expandingDirections();
    }
    else
    {
      if ( sizePolicy().hasHeightForWidth() )
        s.setHeight( heightForWidth( s.width() ) );
      exp = sizePolicy().expandingDirections();
    }
    if ( exp & Qt::Horizontal )
      s.setWidth( qMax( s.width(), 200 ) );
    if ( exp & Qt::Vertical )
      s.setHeight( qMax( s.height(), 100 ) );

    QRect screen;
    if ( const QScreen *screenAtPoint = QGuiApplication::screenAt( pos() ) )
      screen = screenAtPoint->geometry();
    else
      screen = QGuiApplication::primaryScreen()->geometry();

    s.setWidth( qMin( s.width(), screen.width() * 2 / 3 ) );
    s.setHeight( qMin( s.height(), screen.height() * 2 / 3 ) );

  }

  if ( !s.isValid() )
  {
    QRect r = childrenRect(); // get children rectangle
    if ( r.isNull() )
      return s;
    s = r.size() + QSize( 2 * r.x(), 2 * r.y() );
  }

  return s;
}


bool QgsOptionsDialogBase::restoreGeometry2( const QByteArray &geometry )
{
  if ( geometry.size() < 4 )
    return false;
  QDataStream stream( geometry );
  stream.setVersion( QDataStream::Qt_4_0 );

  const quint32 magicNumber = 0x1D9D0CB;
  quint32 storedMagicNumber;
  stream >> storedMagicNumber;
  if ( storedMagicNumber != magicNumber )
    return false;

  const quint16 currentMajorVersion = 3;
  quint16 majorVersion = 0;
  quint16 minorVersion = 0;

  stream >> majorVersion >> minorVersion;

  if ( majorVersion > currentMajorVersion )
    return false;
  // (Allow all minor versions.)

  QRect restoredFrameGeometry;
  QRect restoredGeometry;
  QRect restoredNormalGeometry;
  qint32 restoredScreenNumber;
  quint8 maximized;
  quint8 fullScreen;
  qint32 restoredScreenWidth = 0;

  stream >> restoredFrameGeometry // Only used for sanity checks in version 0
         >> restoredNormalGeometry
         >> restoredScreenNumber
         >> maximized
         >> fullScreen;

  if ( maximized || fullScreen )
  {
    return QWidget::restoreGeometry( geometry );
  }

  if ( majorVersion > 1 )
    stream >> restoredScreenWidth;
  if ( majorVersion > 2 )
    stream >> restoredGeometry;

  // ### Qt 6 - Perhaps it makes sense to dumb down the restoreGeometry() logic, see QTBUG-69104

  if ( restoredScreenNumber >= qMax( QGuiApplication::screens().size(), 1 ) )
    restoredScreenNumber = 0;
  const QScreen *restoredScreen = QGuiApplication::screens().value( restoredScreenNumber, nullptr );
  const qreal screenWidthF = restoredScreen ? qreal( restoredScreen->geometry().width() ) : 0;
  // Sanity check bailing out when large variations of screen sizes occur due to
  // high DPI scaling or different levels of DPI awareness.
  if ( restoredScreenWidth )
  {
    const qreal factor = qreal( restoredScreenWidth ) / screenWidthF;
    if ( factor < 0.8 || factor > 1.25 )
      return false;
  }
  else
  {
    // Saved by Qt 5.3 and earlier, try to prevent too large windows
    // unless the size will be adapted by maximized or fullscreen.
    if ( !maximized && !fullScreen && qreal( restoredFrameGeometry.width() ) / screenWidthF > 1.5 )
      return false;
  }

  const int frameHeight = QApplication::style()
                          ? QApplication::style()->pixelMetric( QStyle::PM_TitleBarHeight )
                          : 20;

  if ( !restoredNormalGeometry.isValid() )
    restoredNormalGeometry = QRect( QPoint( 0, frameHeight ), sizeHint() );
  if ( !restoredNormalGeometry.isValid() )
  {
    // use the widget's adjustedSize if the sizeHint() doesn't help
    restoredNormalGeometry.setSize( restoredNormalGeometry
                                    .size()
                                    .expandedTo( adjustedSize() ) );
  }

  const QRect availableGeometry = restoredScreen ? restoredScreen->availableGeometry()
                                  : QRect();

  // Modify the restored geometry if we are about to restore to coordinates
  // that would make the window "lost". This happens if:
  // - The restored geometry is completely or partly oustside the available geometry
  // - The title bar is outside the available geometry.

  checkRestoredGeometry( availableGeometry, &restoredGeometry, frameHeight );
  checkRestoredGeometry( availableGeometry, &restoredNormalGeometry, frameHeight );

  setWindowState( windowState() & ~( Qt::WindowMaximized | Qt::WindowFullScreen ) );

  // FIXME: Why fall back to restoredNormalGeometry if majorVersion <= 2?
  if ( majorVersion > 2 )
    setGeometry( restoredGeometry );
  else
    setGeometry( restoredNormalGeometry );

  return true;
}
#endif

QgsOptionsDialogBase::QgsOptionsDialogBase( const QString &settingsKey, QWidget *parent, Qt::WindowFlags fl, QgsSettings *settings )
  : QDialog( parent, fl )
  , mOptsKey( settingsKey )
  , mSettings( settings )
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
  if ( auto *lLayout = layout() )
  {
    lLayout->setContentsMargins( 0, 0, 0, 0 ); // Qt default spacing
  }

  // start with copy of qgsoptionsdialog_template.ui to ensure existence of these objects
  mOptListWidget = findChild<QListWidget *>( QStringLiteral( "mOptionsListWidget" ) );
  mOptTreeView = findChild<QTreeView *>( QStringLiteral( "mOptionsTreeView" ) );
  if ( mOptTreeView )
  {
    mOptTreeModel = qobject_cast< QStandardItemModel * >( mOptTreeView->model() );
    mTreeProxyModel = new QgsOptionsProxyModel( this );
    mTreeProxyModel->setSourceModel( mOptTreeModel );
    mOptTreeView->setModel( mTreeProxyModel );
    mOptTreeView->expandAll();
  }

  QFrame *optionsFrame = findChild<QFrame *>( QStringLiteral( "mOptionsFrame" ) );
  mOptStackedWidget = findChild<QStackedWidget *>( QStringLiteral( "mOptionsStackedWidget" ) );
  mOptSplitter = findChild<QSplitter *>( QStringLiteral( "mOptionsSplitter" ) );
  mOptButtonBox = findChild<QDialogButtonBox *>( QStringLiteral( "buttonBox" ) );
  QFrame *buttonBoxFrame = findChild<QFrame *>( QStringLiteral( "mButtonBoxFrame" ) );
  mSearchLineEdit = findChild<QgsFilterLineEdit *>( QStringLiteral( "mSearchLineEdit" ) );

  if ( ( !mOptListWidget && !mOptTreeView ) || !mOptStackedWidget || !mOptSplitter || !optionsFrame )
  {
    return;
  }

  QAbstractItemView *optView = mOptListWidget ? static_cast< QAbstractItemView * >( mOptListWidget ) : static_cast< QAbstractItemView * >( mOptTreeView );
  int iconSize = 16;
  if ( mOptListWidget )
  {
    int size = QgsGuiUtils::scaleIconSize( mSettings->value( QStringLiteral( "/IconSize" ), 24 ).toInt() );
    // buffer size to match displayed icon size in toolbars, and expected geometry restore
    // newWidth (above) may need adjusted if you adjust iconBuffer here
    const int iconBuffer = QgsGuiUtils::scaleIconSize( 4 );
    iconSize = size + iconBuffer;
  }
  else if ( mOptTreeView )
  {
    iconSize = QgsGuiUtils::scaleIconSize( mSettings->value( QStringLiteral( "/IconSize" ), 16 ).toInt() );
    mOptTreeView->header()->setVisible( false );
  }
  optView->setIconSize( QSize( iconSize, iconSize ) );
  optView->setFrameStyle( QFrame::NoFrame );

  const int frameMargin = QgsGuiUtils::scaleIconSize( 3 );
  optionsFrame->layout()->setContentsMargins( 0, frameMargin, frameMargin, frameMargin );
  QVBoxLayout *layout = static_cast<QVBoxLayout *>( optionsFrame->layout() );

  if ( buttonBoxFrame )
  {
    buttonBoxFrame->layout()->setContentsMargins( 0, 0, 0, 0 );
    layout->insertWidget( layout->count(), buttonBoxFrame );
  }
  else if ( mOptButtonBox )
  {
    layout->insertWidget( layout->count(), mOptButtonBox );
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

  if ( mOptTreeView )
  {
    // sync selection in tree view with current stacked widget index
    connect( mOptTreeView->selectionModel(), &QItemSelectionModel::selectionChanged, mOptStackedWidget, [ = ]( const QItemSelection &, const QItemSelection & )
    {
      const QModelIndexList selected = mOptTreeView->selectionModel()->selectedIndexes();
      if ( selected.isEmpty() )
        return;

      const QModelIndex index = mTreeProxyModel->mapToSource( selected.at( 0 ) );

      if ( !mOptTreeModel || !mOptTreeModel->itemFromIndex( index )->isSelectable() )
        return;

      mOptStackedWidget->setCurrentIndex( mTreeProxyModel->sourceIndexToPageNumber( index ) );
    } );
  }

  if ( mSearchLineEdit )
  {
    mSearchLineEdit->setShowSearchIcon( true );
    connect( mSearchLineEdit, &QgsFilterLineEdit::textChanged, this, &QgsOptionsDialogBase::searchText );
    if ( mOptTreeView )
    {
      connect( mSearchLineEdit, &QgsFilterLineEdit::cleared, mOptTreeView, &QTreeView::expandAll );
    }
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
  }
  else
  {
    // re-save original dialog title in case it was changed after dialog initialization
    mDialogTitle = windowTitle();
  }
  updateWindowTitle();

#if QT_VERSION < QT_VERSION_CHECK(6, 5, 0)
  restoreGeometry2( mSettings->value( QStringLiteral( "/Windows/%1/geometry" ).arg( mOptsKey ) ).toByteArray() );
#else
  restoreGeometry( mSettings->value( QStringLiteral( "/Windows/%1/geometry" ).arg( mOptsKey ) ).toByteArray() );
#endif
  // mOptListWidget width is fixed to take up less space in QtDesigner
  // revert it now unless the splitter's state hasn't been saved yet
  QAbstractItemView *optView = mOptListWidget ? static_cast< QAbstractItemView * >( mOptListWidget ) : static_cast< QAbstractItemView * >( mOptTreeView );
  if ( optView )
  {
    optView->setMaximumWidth(
      QgsVariantUtils::isNull( mSettings->value( QStringLiteral( "/Windows/%1/splitState" ).arg( mOptsKey ) ) ) ? 150 : 16777215 );
    // get rid of annoying outer focus rect on Mac
    optView->setAttribute( Qt::WA_MacShowFocusRect, false );
  }

  mOptSplitter->restoreState( mSettings->value( QStringLiteral( "/Windows/%1/splitState" ).arg( mOptsKey ) ).toByteArray() );

  restoreLastPage();

  // brute force approach to try to standardize page margins!
  for ( int i = 0; i < mOptStackedWidget->count(); ++i )
  {
    if ( QLayout *l = mOptStackedWidget->widget( i )->layout() )
    {
      l->setContentsMargins( 0, 0, 0, 0 );
    }
  }
}

void QgsOptionsDialogBase::restoreLastPage()
{
  int curIndx = mSettings->value( QStringLiteral( "/Windows/%1/tab" ).arg( mOptsKey ), 0 ).toInt();

  // if the last used tab is out of range or not enabled display the first enabled one
  if ( mOptStackedWidget->count() < curIndx + 1
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

  if ( mOptStackedWidget->count() == 0 )
    return;

  mOptStackedWidget->setCurrentIndex( curIndx );
  setListToItemAtIndex( curIndx );
}

void QgsOptionsDialogBase::setListToItemAtIndex( int index )
{
  if ( mOptListWidget && mOptListWidget->count() > index )
  {
    mOptListWidget->setCurrentRow( index );
  }
  else if ( mOptTreeView && mOptTreeModel )
  {
    mOptTreeView->setCurrentIndex( mTreeProxyModel->mapFromSource( mTreeProxyModel->pageNumberToSourceIndex( index ) ) );
  }
}

void QgsOptionsDialogBase::resizeAlltabs( int index )
{
  // Adjust size (GH issue #31449 and #32615)
  // make the stacked widget size to the current page only
  for ( int i = 0; i < mOptStackedWidget->count(); ++i )
  {
    // Set the size policy
    QSizePolicy::Policy policy = QSizePolicy::Ignored;
    if ( i == index )
    {
      policy = QSizePolicy::MinimumExpanding;
    }

    // update the size policy
    mOptStackedWidget->widget( i )->setSizePolicy( policy, policy );

    if ( i == index )
    {
      mOptStackedWidget->layout()->update();
    }
  }
  mOptStackedWidget->adjustSize();
}

void QgsOptionsDialogBase::setCurrentPage( const QString &page )
{
  //find the page with a matching widget name
  for ( int idx = 0; idx < mOptStackedWidget->count(); ++idx )
  {
    QWidget *currentPage = mOptStackedWidget->widget( idx );
    if ( currentPage->objectName() == page )
    {
      //found the page, set it as current
      mOptStackedWidget->setCurrentIndex( idx );
      return;
    }
  }
}

void QgsOptionsDialogBase::addPage( const QString &title, const QString &tooltip, const QIcon &icon, QWidget *widget, const QStringList &path, const QString &key )
{
  int newPage = -1;

  if ( mOptListWidget )
  {
    QListWidgetItem *item = new QListWidgetItem();
    item->setIcon( icon );
    item->setText( title );
    item->setToolTip( tooltip );
    mOptListWidget->addItem( item );
  }
  else if ( mOptTreeModel )
  {
    QStandardItem *item = new QStandardItem( icon, title );
    item->setToolTip( tooltip );
    if ( !key.isEmpty() )
    {
      item->setData( key );
    }

    QModelIndex parent;
    QStandardItem *parentItem = nullptr;
    if ( !path.empty() )
    {
      QStringList parents = path;
      while ( !parents.empty() )
      {
        const QString parentPath = parents.takeFirst();

        QModelIndex thisParent;
        for ( int row = 0; row < mOptTreeModel->rowCount( parent ); ++row )
        {
          const QModelIndex index = mOptTreeModel->index( row, 0, parent );
          if ( index.data().toString().compare( parentPath, Qt::CaseInsensitive ) == 0
               || index.data( Qt::UserRole + 1 ).toString().compare( parentPath, Qt::CaseInsensitive ) == 0 )
          {
            thisParent = index;
            break;
          }
        }

        // add new child if required
        if ( !thisParent.isValid() )
        {
          QStandardItem *newParentItem = new QStandardItem( parentPath );
          newParentItem->setToolTip( parentPath );
          newParentItem->setSelectable( false );
          if ( parentItem )
            parentItem->appendRow( newParentItem );
          else
            mOptTreeModel->appendRow( newParentItem );
          parentItem = newParentItem;
        }
        else
        {
          parentItem = mOptTreeModel->itemFromIndex( thisParent );
        }
        parent = mOptTreeModel->indexFromItem( parentItem );
      }
    }

    if ( parentItem )
    {
      parentItem->appendRow( item );
      const QModelIndex newIndex = mOptTreeModel->indexFromItem( item );
      newPage = mTreeProxyModel->sourceIndexToPageNumber( newIndex );
    }
    else
      mOptTreeModel->appendRow( item );
  }

  if ( newPage < 0 )
    mOptStackedWidget->addWidget( widget );
  else
    mOptStackedWidget->insertWidget( newPage, widget );
}

void QgsOptionsDialogBase::insertPage( const QString &title, const QString &tooltip, const QIcon &icon, QWidget *widget, const QString &before, const QStringList &path, const QString &key )
{
  //find the page with a matching widget name
  for ( int page = 0; page < mOptStackedWidget->count(); ++page )
  {
    QWidget *currentPage = mOptStackedWidget->widget( page );
    if ( currentPage->objectName() == before )
    {
      //found the "before" page

      if ( mOptListWidget )
      {
        QListWidgetItem *item = new QListWidgetItem();
        item->setIcon( icon );
        item->setText( title );
        item->setToolTip( tooltip );
        mOptListWidget->insertItem( page, item );
      }
      else if ( mOptTreeModel )
      {
        QModelIndex sourceIndexBefore = mTreeProxyModel->pageNumberToSourceIndex( page );
        QList< QModelIndex > sourceBeforeIndices;
        while ( sourceIndexBefore.parent().isValid() )
        {
          sourceBeforeIndices.insert( 0, sourceIndexBefore );
          sourceIndexBefore = sourceIndexBefore.parent();
        }
        sourceBeforeIndices.insert( 0, sourceIndexBefore );

        QStringList parentPaths = path;

        QModelIndex parentIndex;
        QStandardItem *parentItem = nullptr;
        while ( !parentPaths.empty() )
        {
          QString thisPath = parentPaths.takeFirst();
          QModelIndex sourceIndex = !sourceBeforeIndices.isEmpty() ? sourceBeforeIndices.takeFirst() : QModelIndex();

          if ( sourceIndex.data().toString().compare( thisPath, Qt::CaseInsensitive ) == 0
               || sourceIndex.data( Qt::UserRole + 1 ).toString().compare( thisPath, Qt::CaseInsensitive ) == 0 )
          {
            parentIndex = sourceIndex;
            parentItem = mOptTreeModel->itemFromIndex( parentIndex );
          }
          else
          {
            QStandardItem *newParentItem = new QStandardItem( thisPath );
            newParentItem->setToolTip( thisPath );
            newParentItem->setSelectable( false );
            if ( sourceIndex.isValid() )
            {
              // insert in model before sourceIndex
              if ( parentItem )
                parentItem->insertRow( sourceIndex.row(), newParentItem );
              else
                mOptTreeModel->insertRow( sourceIndex.row(), newParentItem );
            }
            else
            {
              // append to end
              if ( parentItem )
                parentItem->appendRow( newParentItem );
              else
                mOptTreeModel->appendRow( newParentItem );
            }
            parentItem = newParentItem;
          }
        }

        QStandardItem *item = new QStandardItem( icon, title );
        item->setToolTip( tooltip );
        if ( !key.isEmpty() )
        {
          item->setData( key );
        }
        if ( parentItem )
        {
          if ( sourceBeforeIndices.empty() )
            parentItem->appendRow( item );
          else
          {
            parentItem->insertRow( sourceBeforeIndices.at( 0 ).row(), item );
          }
        }
        else
        {
          mOptTreeModel->insertRow( sourceIndexBefore.row(), item );
        }
      }

      mOptStackedWidget->insertWidget( page, widget );
      return;
    }
  }

  // no matching pages, so just add the page
  addPage( title, tooltip, icon, widget, path );
}

void QgsOptionsDialogBase::searchText( const QString &text )
{
  const int minimumTextLength = 3;

  mSearchLineEdit->setMinimumWidth( text.isEmpty() ? 0 : 70 );

  if ( !mOptStackedWidget )
    return;

  if ( mOptStackedWidget->isHidden() )
    mOptStackedWidget->show();
  if ( mOptButtonBox && mOptButtonBox->isHidden() )
    mOptButtonBox->show();

  // hide all pages if text has to be search, show them all otherwise
  if ( mOptListWidget )
  {
    for ( int r = 0; r < mOptStackedWidget->count(); ++r )
    {
      if ( mOptListWidget->item( r )->text().contains( text, Qt::CaseInsensitive ) )
      {
        mOptListWidget->setRowHidden( r, false );
      }
      else
      {
        mOptListWidget->setRowHidden( r, text.length() >= minimumTextLength );
      }
    }

    for ( const QPair< QgsOptionsDialogHighlightWidget *, int > &rsw : std::as_const( mRegisteredSearchWidgets ) )
    {
      if ( rsw.first->searchHighlight( text.length() >= minimumTextLength ? text : QString() ) )
      {
        mOptListWidget->setRowHidden( rsw.second, false );
      }
    }
  }
  else if ( mTreeProxyModel )
  {
    QMap< int, bool > hiddenPages;
    for ( int r = 0; r < mOptStackedWidget->count(); ++r )
    {
      hiddenPages.insert( r, text.length() >= minimumTextLength );
    }

    std::function<void( const QModelIndex & )> traverseModel;
    // traverse through the model, showing pages which match by page name
    traverseModel = [&]( const QModelIndex & parent )
    {
      for ( int row = 0; row < mOptTreeModel->rowCount( parent ); ++row )
      {
        const QModelIndex currentIndex = mOptTreeModel->index( row, 0, parent );
        if ( currentIndex.data().toString().contains( text, Qt::CaseInsensitive ) )
        {
          hiddenPages.insert( mTreeProxyModel->sourceIndexToPageNumber( currentIndex ), false );
        }
        traverseModel( currentIndex );
      }
    };
    traverseModel( QModelIndex() );

    for ( const QPair< QgsOptionsDialogHighlightWidget *, int > &rsw : std::as_const( mRegisteredSearchWidgets ) )
    {
      if ( rsw.first->searchHighlight( text.length() >= minimumTextLength ? text : QString() ) )
      {
        hiddenPages.insert( rsw.second, false );
      }
    }
    for ( auto it = hiddenPages.constBegin(); it != hiddenPages.constEnd(); ++it )
    {
      mTreeProxyModel->setPageHidden( it.key(), it.value() );
    }
  }
  if ( mOptTreeView && text.length() >= minimumTextLength )
  {
    // auto expand out any group with children matching the search term
    mOptTreeView->expandAll();
  }

  // if current item is hidden, move to first available...
  if ( mOptListWidget && mOptListWidget->isRowHidden( mOptStackedWidget->currentIndex() ) )
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
  else if ( mOptTreeView )
  {
    const QModelIndex currentSourceIndex = mTreeProxyModel->pageNumberToSourceIndex( mOptStackedWidget->currentIndex() );
    if ( !mTreeProxyModel->filterAcceptsRow( currentSourceIndex.row(), currentSourceIndex.parent() ) )
    {
      std::function<QModelIndex( const QModelIndex & )> traverseModel;
      traverseModel = [&]( const QModelIndex & parent ) -> QModelIndex
      {
        for ( int row = 0; row < mTreeProxyModel->rowCount(); ++row )
        {
          const QModelIndex proxyIndex = mTreeProxyModel->index( row, 0, parent );
          const QModelIndex sourceIndex = mTreeProxyModel->mapToSource( proxyIndex );
          if ( mOptTreeModel->itemFromIndex( sourceIndex )->isSelectable() )
          {
            return sourceIndex;
          }
          else
          {
            QModelIndex res = traverseModel( proxyIndex );
            if ( res.isValid() )
              return res;
          }
        }
        return QModelIndex();
      };

      const QModelIndex firstVisibleSourceIndex = traverseModel( QModelIndex() );

      if ( firstVisibleSourceIndex.isValid() )
      {
        mOptTreeView->setCurrentIndex( mTreeProxyModel->mapFromSource( firstVisibleSourceIndex ) );
      }
      else
      {
        // if no page can be shown, hide stack widget
        mOptStackedWidget->hide();
        if ( mOptButtonBox )
          mOptButtonBox->hide();
      }
    }
    else
    {
      // make sure item stays current
      mOptTreeView->setCurrentIndex( mTreeProxyModel->mapFromSource( currentSourceIndex ) );
    }
  }
}

void QgsOptionsDialogBase::registerTextSearchWidgets()
{
  mRegisteredSearchWidgets.clear();

  for ( int i = 0; i < mOptStackedWidget->count(); i++ )
  {
    const QList< QWidget * > widgets = mOptStackedWidget->widget( i )->findChildren<QWidget *>();
    for ( QWidget *widget : widgets )
    {
      // see if the widget also inherits QgsOptionsDialogHighlightWidget
      QgsOptionsDialogHighlightWidget *shw = dynamic_cast<QgsOptionsDialogHighlightWidget *>( widget );
      if ( !shw )
      {
        // get custom highlight widget in user added pages
        QHash<QWidget *, QgsOptionsDialogHighlightWidget *> customHighlightWidgets;
        QgsOptionsPageWidget *opw = qobject_cast<QgsOptionsPageWidget *>( mOptStackedWidget->widget( i ) );
        if ( opw )
        {
          customHighlightWidgets = opw->registeredHighlightWidgets();
        }
        // take custom if exists
        if ( customHighlightWidgets.contains( widget ) )
        {
          shw = customHighlightWidgets.value( widget );
        }
      }
      // try to construct one otherwise
      if ( !shw || !shw->isValid() )
      {
        shw = QgsOptionsDialogHighlightWidget::createWidget( widget );
      }
      if ( shw && shw->isValid() )
      {
        QgsDebugMsgLevel( QStringLiteral( "Registering: %1" ).arg( widget->objectName() ), 4 );
        mRegisteredSearchWidgets.append( qMakePair( shw, i ) );
      }
      else
      {
        delete shw;
      }
    }
  }
}

QStandardItem *QgsOptionsDialogBase::createItem( const QString &name, const QString &tooltip, const QString &icon )
{
  QStandardItem *res = new QStandardItem( QgsApplication::getThemeIcon( icon ), name );
  res->setToolTip( tooltip );
  return res;
}

void QgsOptionsDialogBase::showEvent( QShowEvent *e )
{
  if ( mInit )
  {
    updateOptionsListVerticalTabs();
    if ( mOptListWidget )
    {
      optionsStackedWidget_CurrentChanged( mOptListWidget->currentRow() );
    }
    else if ( mOptTreeView )
    {
      optionsStackedWidget_CurrentChanged( mTreeProxyModel->sourceIndexToPageNumber( mTreeProxyModel->mapToSource( mOptTreeView->currentIndex() ) ) );
    }
  }
  else
  {
    QTimer::singleShot( 0, this, &QgsOptionsDialogBase::warnAboutMissingObjects );
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
    QTimer::singleShot( 0, this, &QgsOptionsDialogBase::updateOptionsListVerticalTabs );

  QDialog::paintEvent( e );
}

void QgsOptionsDialogBase::updateWindowTitle()
{
  const QString itemText = mOptListWidget && mOptListWidget->currentItem() ? mOptListWidget->currentItem()->text()
                           : mOptTreeView && mOptTreeView->currentIndex().isValid() ? mOptTreeView->currentIndex().data( Qt::DisplayRole ).toString() : QString();
  if ( !itemText.isEmpty() )
  {
    setWindowTitle( QStringLiteral( "%1 %2 %3" )
                    .arg( mDialogTitle )
                    .arg( QChar( 0x2014 ) ) // em-dash unicode
                    .arg( itemText ) );
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

  QAbstractItemView *optView = mOptListWidget ? static_cast< QAbstractItemView * >( mOptListWidget ) : static_cast< QAbstractItemView * >( mOptTreeView );
  if ( optView )
  {
    if ( optView->maximumWidth() != 16777215 )
      optView->setMaximumWidth( 16777215 );
    // auto-resize splitter for vert scrollbar without covering icons in icon-only mode
    // TODO: mOptListWidget has fixed 32px wide icons for now, allow user-defined
    // Note: called on splitter resize and dialog paint event, so only update when necessary
    int iconWidth = optView->iconSize().width();
    int snapToIconWidth = iconWidth + 32;

    QList<int> splitSizes = mOptSplitter->sizes();
    mIconOnly = ( splitSizes.at( 0 ) <= snapToIconWidth );

    // iconBuffer (above) may need adjusted if you adjust iconWidth here
    int newWidth = optView->verticalScrollBar()->isVisible() ? iconWidth + 22 : iconWidth + 9;
    bool diffWidth = optView->minimumWidth() != newWidth;

    if ( diffWidth )
      optView->setMinimumWidth( newWidth );

    if ( mIconOnly && ( diffWidth || optView->width() != newWidth ) )
    {
      splitSizes[1] = splitSizes.at( 1 ) - ( splitSizes.at( 0 ) - newWidth );
      splitSizes[0] = newWidth;
      mOptSplitter->setSizes( splitSizes );
    }

    if ( mOptListWidget )
    {
      if ( mOptListWidget->wordWrap() && mIconOnly )
        mOptListWidget->setWordWrap( false );
      if ( !mOptListWidget->wordWrap() && !mIconOnly )
        mOptListWidget->setWordWrap( true );
    }
  }
}

void QgsOptionsDialogBase::optionsStackedWidget_CurrentChanged( int index )
{
  if ( mOptListWidget )
  {
    mOptListWidget->blockSignals( true );
    mOptListWidget->setCurrentRow( index );
    mOptListWidget->blockSignals( false );
  }
  else if ( mOptTreeView )
  {
    mOptTreeView->blockSignals( true );
    mOptTreeView->setCurrentIndex( mTreeProxyModel->mapFromSource( mTreeProxyModel->pageNumberToSourceIndex( index ) ) );
    mOptTreeView->blockSignals( false );
  }

  updateWindowTitle();
}

void QgsOptionsDialogBase::optionsStackedWidget_WidgetRemoved( int index )
{
  // will need to take item first, if widgets are set for item in future
  if ( mOptListWidget )
  {
    delete mOptListWidget->item( index );
  }
  else if ( mOptTreeModel )
  {
    mOptTreeModel->removeRow( index );
  }

  QList<QPair< QgsOptionsDialogHighlightWidget *, int > >::iterator it = mRegisteredSearchWidgets.begin();
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
  QMessageBox::warning( nullptr, tr( "Missing Objects" ),
                        tr( "Base options dialog could not be initialized.\n\n"
                            "Missing some of the .ui template objects:\n" )
                        + " mOptionsListWidget,\n mOptionsStackedWidget,\n mOptionsSplitter,\n mOptionsListFrame",
                        QMessageBox::Ok,
                        QMessageBox::Ok );
}


///@cond PRIVATE
QgsOptionsProxyModel::QgsOptionsProxyModel( QObject *parent )
  : QSortFilterProxyModel( parent )
{
  setDynamicSortFilter( true );
}

void QgsOptionsProxyModel::setPageHidden( int page, bool hidden )
{
  mHiddenPages[ page ] = hidden;
  invalidateFilter();
}

QModelIndex QgsOptionsProxyModel::pageNumberToSourceIndex( int page ) const
{
  QStandardItemModel *itemModel = qobject_cast< QStandardItemModel * >( sourceModel() );
  if ( !itemModel )
    return QModelIndex();

  int pagesRemaining = page;
  std::function<QModelIndex( const QModelIndex & )> traversePages;

  // traverse through the model, counting all selectable items until we hit the desired page number
  traversePages = [&]( const QModelIndex & parent ) -> QModelIndex
  {
    for ( int row = 0; row < itemModel->rowCount( parent ); ++row )
    {
      const QModelIndex currentIndex = itemModel->index( row, 0, parent );
      if ( itemModel->itemFromIndex( currentIndex )->isSelectable() )
      {
        if ( pagesRemaining == 0 )
          return currentIndex;

        else pagesRemaining--;
      }

      const QModelIndex res = traversePages( currentIndex );
      if ( res.isValid() )
        return res;
    }
    return QModelIndex();
  };

  return traversePages( QModelIndex() );
}

int QgsOptionsProxyModel::sourceIndexToPageNumber( const QModelIndex &index ) const
{
  QStandardItemModel *itemModel = qobject_cast< QStandardItemModel * >( sourceModel() );
  if ( !itemModel )
    return 0;

  int page = 0;

  std::function<int( const QModelIndex & )> traverseModel;

  // traverse through the model, counting all which correspond to pages till we hit the desired index
  traverseModel = [&]( const QModelIndex & parent ) -> int
  {
    for ( int row = 0; row < itemModel->rowCount( parent ); ++row )
    {
      const QModelIndex currentIndex = itemModel->index( row, 0, parent );
      if ( currentIndex == index )
        return page;

      if ( itemModel->itemFromIndex( currentIndex )->isSelectable() )
        page++;

      const int res = traverseModel( currentIndex );
      if ( res >= 0 )
        return res;
    }
    return -1;
  };

  return traverseModel( QModelIndex() );
}

bool QgsOptionsProxyModel::filterAcceptsRow( int source_row, const QModelIndex &source_parent ) const
{
  QStandardItemModel *itemModel = qobject_cast< QStandardItemModel * >( sourceModel() );
  if ( !itemModel )
    return true;

  const QModelIndex sourceIndex = sourceModel()->index( source_row, 0, source_parent );

  const int pageNumber = sourceIndexToPageNumber( sourceIndex );
  if ( !mHiddenPages.value( pageNumber, false ) )
    return true;

  if ( sourceModel()->hasChildren( sourceIndex ) )
  {
    // this is a group -- show if any children are visible
    for ( int row = 0; row < sourceModel()->rowCount( sourceIndex ); ++row )
    {
      if ( filterAcceptsRow( row, sourceIndex ) )
        return true;
    }
  }
  return false;
}
///@endcond
