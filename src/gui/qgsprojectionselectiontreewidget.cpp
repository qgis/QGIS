/***************************************************************************
 *   qgsprojectionselector.cpp                                             *
 *   Copyright (C) 2005 by Tim Sutton                                      *
 *   tim@linfiniti.com                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#include "qgsprojectionselectiontreewidget.h"

//standard includes
#include <sqlite3.h>

//qgis includes
#include "qgis.h" //magic numbers here
#include "qgsapplication.h"
#include "qgslogger.h"
#include "qgscoordinatereferencesystem.h"
#include "qgssettings.h"
#include "qgsrectangle.h"
#include "qgsdatums.h"
#include "qgsprojoperation.h"
#include "qgsstringutils.h"
#include "qgsunittypes.h"
#include "qgscoordinatereferencesystemmodel.h"

//qt includes
#include <QAction>
#include <QToolButton>
#include <QMenu>
#include <QFileInfo>
#include <QHeaderView>
#include <QResizeEvent>
#include <QMessageBox>
#include <QRegularExpression>

QgsProjectionSelectionTreeWidget::QgsProjectionSelectionTreeWidget( QWidget *parent, QgsCoordinateReferenceSystemProxyModel::Filters filters )
  : QWidget( parent )
{
  setupUi( this );

  mCrsModel = new QgsCoordinateReferenceSystemProxyModel( this );
  mCrsModel->setFilters( filters );

  lstCoordinateSystems->setModel( mCrsModel );
  lstCoordinateSystems->setSelectionBehavior( QAbstractItemView::SelectRows );

  QFont f = teProjection->font();
  f.setPointSize( f.pointSize() - 2 );
  teProjection->setFont( f );

  leSearch->setShowSearchIcon( true );

  connect( lstCoordinateSystems, &QTreeView::doubleClicked, this, &QgsProjectionSelectionTreeWidget::lstCoordinateSystemsDoubleClicked );
  connect( lstRecent, &QTreeWidget::itemDoubleClicked, this, &QgsProjectionSelectionTreeWidget::lstRecent_itemDoubleClicked );
  connect( lstCoordinateSystems->selectionModel(), &QItemSelectionModel::selectionChanged, this, &QgsProjectionSelectionTreeWidget::lstCoordinateSystemsSelectionChanged );
  connect( lstRecent, &QTreeWidget::currentItemChanged, this, &QgsProjectionSelectionTreeWidget::lstRecent_currentItemChanged );
  connect( cbxHideDeprecated, &QCheckBox::toggled, this, [ = ]( bool selected )
  {
    mCrsModel->setFilterDeprecated( selected );
    filterRecentCrsList();
  } );
  connect( leSearch, &QgsFilterLineEdit::textChanged, this, [ = ]( const QString & filter )
  {
    mCrsModel->setFilterString( filter );
    filterRecentCrsList();
  } );

  mAreaCanvas->setVisible( mShowMap );

  lstCoordinateSystems->header()->setSectionResizeMode( AuthidColumn, QHeaderView::Stretch );

  // Hide (internal) ID column
  lstRecent->header()->setSectionResizeMode( AuthidColumn, QHeaderView::Stretch );
  lstRecent->header()->resizeSection( QgisCrsIdColumn, 0 );
  lstRecent->header()->setSectionResizeMode( QgisCrsIdColumn, QHeaderView::Fixed );
  lstRecent->setColumnHidden( QgisCrsIdColumn, true );

  // Clear Crs Column
  lstRecent->header()->setMinimumSectionSize( 10 );
  lstRecent->header()->setStretchLastSection( false );
  lstRecent->header()->resizeSection( ClearColumn, 20 );

  // Clear recent crs context menu
  lstRecent->setContextMenuPolicy( Qt::CustomContextMenu );
  connect( lstRecent, &QTreeWidget::customContextMenuRequested, this, [this]( const QPoint & pos )
  {
    // If list is empty, do nothing
    if ( lstRecent->topLevelItemCount() == 0 )
      return;
    QMenu menu;
    // Clear selected
    QTreeWidgetItem *currentItem = lstRecent->itemAt( pos );
    if ( currentItem )
    {
      QAction *clearSelected = menu.addAction( QgsApplication::getThemeIcon( "/mIconClearItem.svg" ),  tr( "Remove Selected CRS from Recently Used CRS" ) );
      connect( clearSelected, &QAction::triggered, this, [this, currentItem ] { removeRecentCrsItem( currentItem ); } );
      menu.addSeparator();
    }
    // Clear all
    QAction *clearAll = menu.addAction( QgsApplication::getThemeIcon( "/console/iconClearConsole.svg" ), tr( "Clear All Recently Used CRS" ) );
    connect( clearAll, &QAction::triggered, this, &QgsProjectionSelectionTreeWidget::clearRecentCrs );
    menu.exec( lstRecent->viewport()->mapToGlobal( pos ) );
  } );

  // Install event fiter to catch delete key press on the recent crs list
  lstRecent->installEventFilter( this );

  mRecentProjections = QgsCoordinateReferenceSystem::recentCoordinateReferenceSystems();
  for ( const QgsCoordinateReferenceSystem &crs : std::as_const( mRecentProjections ) )
  {
    insertRecent( crs );
  }

  mCheckBoxNoProjection->setHidden( true );
  mCheckBoxNoProjection->setEnabled( false );
  connect( mCheckBoxNoProjection, &QCheckBox::toggled, this, [ = ]
  {
    if ( !mBlockSignals )
    {
      emit crsSelected();
      emit hasValidSelectionChanged( hasValidSelection() );
    }
  } );
  connect( mCheckBoxNoProjection, &QCheckBox::toggled, this, [ = ]( bool checked )
  {
    if ( mCheckBoxNoProjection->isEnabled() )
    {
      mFrameProjections->setDisabled( checked );
    }
  } );

  QgsSettings settings;
  mSplitter->restoreState( settings.value( QStringLiteral( "Windows/ProjectionSelector/splitterState" ) ).toByteArray() );
}

QgsProjectionSelectionTreeWidget::~QgsProjectionSelectionTreeWidget()
{
  QgsSettings settings;
  settings.setValue( QStringLiteral( "Windows/ProjectionSelector/splitterState" ), mSplitter->saveState() );

  // Push current projection to front, only if set
  const QgsCoordinateReferenceSystem selectedCrs = crs();
  if ( selectedCrs.isValid() )
    QgsCoordinateReferenceSystem::pushRecentCoordinateReferenceSystem( selectedCrs );
}

void QgsProjectionSelectionTreeWidget::resizeEvent( QResizeEvent *event )
{
  lstCoordinateSystems->header()->resizeSection( NameColumn, event->size().width() - 240 );
  lstCoordinateSystems->header()->resizeSection( AuthidColumn, 240 );
  lstCoordinateSystems->header()->resizeSection( QgisCrsIdColumn, 0 );

  lstRecent->header()->resizeSection( NameColumn, event->size().width() - 260 );
  lstRecent->header()->resizeSection( AuthidColumn, 240 );
  lstRecent->header()->resizeSection( QgisCrsIdColumn, 0 );
  lstRecent->header()->resizeSection( ClearColumn, 20 );
}

bool QgsProjectionSelectionTreeWidget::eventFilter( QObject *obj, QEvent *ev )
{
  if ( obj != lstRecent )
    return false;

  if ( ev->type() != QEvent::KeyPress )
    return false;

  QKeyEvent *keyEvent = static_cast<QKeyEvent *>( ev );
  if ( keyEvent->matches( QKeySequence::Delete ) )
  {
    removeRecentCrsItem( lstRecent->currentItem() );
    return true;
  }

  return false;
}

void QgsProjectionSelectionTreeWidget::selectCrsByAuthId( const QString &authid )
{
  const QModelIndex sourceIndex = mCrsModel->coordinateReferenceSystemModel()->authIdToIndex( authid );
  if ( !sourceIndex.isValid() )
    return;

  const QModelIndex proxyIndex = mCrsModel->mapFromSource( sourceIndex );
  if ( proxyIndex.isValid() )
  {
    lstCoordinateSystems->selectionModel()->select( proxyIndex, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows );
    lstCoordinateSystems->scrollTo( proxyIndex );
  }
  else
  {
    // deselect the selected item to avoid confusing the user
    lstCoordinateSystems->clearSelection();
    lstRecent->clearSelection();
    teProjection->clear();
  }
}

void QgsProjectionSelectionTreeWidget::insertRecent( const QgsCoordinateReferenceSystem &crs )
{
  const QModelIndex sourceIndex = mCrsModel->coordinateReferenceSystemModel()->authIdToIndex( crs.authid() );
  if ( !sourceIndex.isValid() )
    return;

  QTreeWidgetItem *item = new QTreeWidgetItem( lstRecent, QStringList()
      << sourceIndex.data( QgsCoordinateReferenceSystemModel::RoleName ).toString()
      << sourceIndex.data( QgsCoordinateReferenceSystemModel::RoleAuthId ).toString() );

  // Insert clear button in the last column
  QToolButton *clearButton = new QToolButton();
  clearButton->setIcon( QgsApplication::getThemeIcon( "/mIconClearItem.svg" ) );
  clearButton->setAutoRaise( true );
  clearButton->setToolTip( tr( "Remove from recently used CRS" ) );
  connect( clearButton, &QToolButton::clicked, this, [this, item] { removeRecentCrsItem( item ); } );
  lstRecent->setItemWidget( item, ClearColumn, clearButton );

  lstRecent->insertTopLevelItem( 0,  item );
}

void QgsProjectionSelectionTreeWidget::setCrs( const QgsCoordinateReferenceSystem &crs )
{
  if ( !crs.isValid() )
  {
    mCheckBoxNoProjection->setChecked( true );
  }
  else
  {
    mBlockSignals = true;
    mCheckBoxNoProjection->setChecked( false );
    mBlockSignals = false;

    if ( !crs.authid().isEmpty() )
      selectCrsByAuthId( crs.authid() );
    else
      loadUnknownCrs( crs );

    const bool changed = crs != QgsProjectionSelectionTreeWidget::crs();
    if ( changed )
    {
      emit crsSelected();
      emit hasValidSelectionChanged( hasValidSelection() );
    }
  }
}

void QgsProjectionSelectionTreeWidget::setPreviewRect( const QgsRectangle &rect )
{
  mAreaCanvas->setCanvasRect( rect );
}

QgsRectangle QgsProjectionSelectionTreeWidget::previewRect() const
{
  return mAreaCanvas->canvasRect();
}

QgsCoordinateReferenceSystemProxyModel::Filters QgsProjectionSelectionTreeWidget::filters() const
{
  return mCrsModel->filters();
}

void QgsProjectionSelectionTreeWidget::setFilters( QgsCoordinateReferenceSystemProxyModel::Filters filters )
{
  mCrsModel->setFilters( filters );
}

QgsCoordinateReferenceSystem QgsProjectionSelectionTreeWidget::crs() const
{
  if ( mCheckBoxNoProjection->isEnabled() && mCheckBoxNoProjection->isChecked() )
    return QgsCoordinateReferenceSystem();

  const QModelIndex currentIndex = lstCoordinateSystems->selectionModel()->selectedRows( 0 ).value( 0 );
  const QString authid = currentIndex.data( QgsCoordinateReferenceSystemModel::RoleAuthId ).toString();
  if ( !authid.isEmpty() )
  {
    return QgsCoordinateReferenceSystem::fromOgcWmsCrs( authid );
  }
  else
  {
    // custom CRS
    const QString wkt = currentIndex.data( QgsCoordinateReferenceSystemModel::RoleWkt ).toString();
    const QString proj = currentIndex.data( QgsCoordinateReferenceSystemModel::RoleProj ).toString();

    if ( !wkt.isEmpty() )
      return QgsCoordinateReferenceSystem::fromWkt( wkt );
    else if ( !proj.isEmpty() )
      return QgsCoordinateReferenceSystem::fromProj( proj );
    else
      return QgsCoordinateReferenceSystem();
  }
}

void QgsProjectionSelectionTreeWidget::setShowNoProjection( bool show )
{
  mCheckBoxNoProjection->setVisible( show );
  mCheckBoxNoProjection->setEnabled( show );
  if ( show )
  {
    mFrameProjections->setDisabled( mCheckBoxNoProjection->isChecked() );
  }
}

void QgsProjectionSelectionTreeWidget::setShowBoundsMap( bool show )
{
  mShowMap = show;
  mAreaCanvas->setVisible( show );
}

bool QgsProjectionSelectionTreeWidget::showNoProjection() const
{
  return !mCheckBoxNoProjection->isHidden();
}

void QgsProjectionSelectionTreeWidget::setNotSetText( const QString &text )
{
  mCheckBoxNoProjection->setText( text );
}

bool QgsProjectionSelectionTreeWidget::showBoundsMap() const
{
  return mShowMap;
}

bool QgsProjectionSelectionTreeWidget::hasValidSelection() const
{
  if ( mCheckBoxNoProjection->isChecked() )
  {
    return true;
  }
  else
  {
    const QModelIndex currentIndex = lstCoordinateSystems->selectionModel()->selectedRows( 0 ).value( 0 );
    const QString authid = currentIndex.data( QgsCoordinateReferenceSystemModel::RoleAuthId ).toString();
    const QString wkt = currentIndex.data( QgsCoordinateReferenceSystemModel::RoleWkt ).toString();
    const QString proj = currentIndex.data( QgsCoordinateReferenceSystemModel::RoleProj ).toString();
    return !authid.isEmpty() || !wkt.isEmpty() || !proj.isEmpty();
  }
}

void QgsProjectionSelectionTreeWidget::setOgcWmsCrsFilter( const QSet<QString> &crsFilter )
{
  mCrsModel->setFilterAuthIds( crsFilter );
}

void QgsProjectionSelectionTreeWidget::loadUnknownCrs( const QgsCoordinateReferenceSystem &crs )
{
  const QModelIndex sourceIndex = mCrsModel->coordinateReferenceSystemModel()->addCustomCrs( crs );
  lstCoordinateSystems->selectionModel()->select( mCrsModel->mapFromSource( sourceIndex ), QItemSelectionModel::ClearAndSelect  | QItemSelectionModel::Rows );
  lstCoordinateSystems->scrollTo( mCrsModel->mapFromSource( sourceIndex ) );
}

// New coordinate system selected from the list
void QgsProjectionSelectionTreeWidget::lstCoordinateSystemsSelectionChanged( const QItemSelection &selected, const QItemSelection & )
{
  if ( selected.isEmpty() )
  {
    QgsDebugMsgLevel( QStringLiteral( "no current item" ), 4 );
    return;
  }

  const QModelIndex selectedProxyIndex = lstCoordinateSystems->selectionModel()->selectedRows( 0 ).value( 0 );
  if ( !selectedProxyIndex.isValid() )
    return;

  lstCoordinateSystems->scrollTo( selectedProxyIndex );
  const QModelIndex sourceIndex = mCrsModel->mapToSource( selectedProxyIndex );

  // If the item has children, it's not an end node in the tree, and
  // hence is just a grouping thingy, not an actual CRS.
  if ( mCrsModel->coordinateReferenceSystemModel()->rowCount( sourceIndex ) == 0 )
  {
    // Found a real CRS
    if ( !mBlockSignals )
    {
      emit crsSelected();
      emit hasValidSelectionChanged( true );
    }

    updateBoundsPreview();

    const QString crsAuthId = mCrsModel->coordinateReferenceSystemModel()->data( sourceIndex, QgsCoordinateReferenceSystemModel::RoleAuthId ).toString();
    if ( !crsAuthId.isEmpty() )
    {
      QList<QTreeWidgetItem *> nodes = lstRecent->findItems( crsAuthId, Qt::MatchExactly, AuthidColumn );
      if ( !nodes.isEmpty() )
      {
        QgsDebugMsgLevel( QStringLiteral( "found srs %1 in recent" ).arg( crsAuthId ), 4 );
        lstRecent->setCurrentItem( nodes.first() );
      }
      else
      {
        QgsDebugMsgLevel( QStringLiteral( "srs %1 not recent" ).arg( crsAuthId ), 4 );
        lstRecent->clearSelection();
        lstCoordinateSystems->setFocus( Qt::OtherFocusReason );
      }
    }
    else
    {
      lstRecent->clearSelection();
      lstCoordinateSystems->setFocus( Qt::OtherFocusReason );
    }
  }
  else
  {
    // Not a CRS
    teProjection->clear();
    lstRecent->clearSelection();
    emit hasValidSelectionChanged( false );
  }
}

void QgsProjectionSelectionTreeWidget::lstCoordinateSystemsDoubleClicked( const QModelIndex &index )
{
  if ( !index.isValid() )
  {
    QgsDebugMsgLevel( QStringLiteral( "no current item" ), 4 );
    return;
  }

  // If the item has children, it's not an end node in the tree, and
  // hence is just a grouping thingy, not an actual CRS.
  if ( !mCrsModel->coordinateReferenceSystemModel()->hasChildren( mCrsModel->mapToSource( index ) ) )
    emit projectionDoubleClicked();
}

void QgsProjectionSelectionTreeWidget::lstRecent_currentItemChanged( QTreeWidgetItem *current, QTreeWidgetItem * )
{
  QgsDebugMsgLevel( QStringLiteral( "Entered." ), 4 );

  if ( !current )
  {
    QgsDebugMsgLevel( QStringLiteral( "no current item" ), 4 );
    return;
  }

  lstRecent->scrollToItem( current );

  const QString selectedAuthId = current->text( AuthidColumn );
  const QModelIndex sourceIndex = mCrsModel->coordinateReferenceSystemModel()->authIdToIndex( selectedAuthId );
  if ( sourceIndex.isValid() )
  {
    const QModelIndex proxyIndex = mCrsModel->mapFromSource( sourceIndex );
    if ( proxyIndex.isValid() )
    {
      lstCoordinateSystems->selectionModel()->select( proxyIndex, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows );
      lstCoordinateSystems->scrollTo( proxyIndex );
    }
  }
}

void QgsProjectionSelectionTreeWidget::lstRecent_itemDoubleClicked( QTreeWidgetItem *current, int column )
{
  Q_UNUSED( column )

  QgsDebugMsgLevel( QStringLiteral( "Entered." ), 4 );

  if ( !current )
  {
    QgsDebugMsgLevel( QStringLiteral( "no current item" ), 4 );
    return;
  }

  const QString selectedAuthId = current->text( AuthidColumn );
  const QModelIndex sourceIndex = mCrsModel->coordinateReferenceSystemModel()->authIdToIndex( selectedAuthId );
  if ( sourceIndex.isValid() )
  {
    const QModelIndex proxyIndex = mCrsModel->mapFromSource( sourceIndex );
    if ( proxyIndex.isValid() )
    {
      emit projectionDoubleClicked();
    }
  }
}

void QgsProjectionSelectionTreeWidget::filterRecentCrsList()
{
  QString filterTxtCopy = QgsStringUtils::qRegExpEscape( leSearch->text() );
  const thread_local QRegularExpression filterRx( QStringLiteral( "\\s+" ) );
  filterTxtCopy.replace( filterRx, QStringLiteral( ".*" ) );
  const QRegularExpression re( filterTxtCopy, QRegularExpression::PatternOption::CaseInsensitiveOption );

  const bool hideDeprecated = cbxHideDeprecated->isChecked();

  QTreeWidgetItemIterator itr( lstRecent );
  while ( *itr )
  {
    if ( ( *itr )->childCount() == 0 ) // it's an end node aka a projection
    {
      if ( hideDeprecated && ( *itr )->data( 0, RoleDeprecated ).toBool() )
      {
        ( *itr )->setHidden( true );
        if ( ( *itr )->isSelected() )
        {
          ( *itr )->setSelected( false );
          teProjection->clear();
        }
      }
      else if ( ( *itr )->text( NameColumn ).contains( re )
                || ( *itr )->text( AuthidColumn ).contains( re )
              )
      {
        ( *itr )->setHidden( false );
        QTreeWidgetItem *parent = ( *itr )->parent();
        while ( parent )
        {
          parent->setExpanded( true );
          parent->setHidden( false );
          parent = parent->parent();
        }
      }
      else
      {
        ( *itr )->setHidden( true );
      }
    }
    else
    {
      ( *itr )->setHidden( true );
    }
    ++itr;
  }
}

void QgsProjectionSelectionTreeWidget::pushProjectionToFront()
{
}

void QgsProjectionSelectionTreeWidget::updateBoundsPreview()
{
  const QgsCoordinateReferenceSystem currentCrs = crs();
  if ( !currentCrs.isValid() )
    return;

  QgsRectangle rect = currentCrs.bounds();
  QString extentString = tr( "Extent not known" );
  mAreaCanvas->setPreviewRect( rect );
  if ( !qgsDoubleNear( rect.area(), 0.0 ) )
  {
    extentString = QStringLiteral( "%1, %2, %3, %4" )
                   .arg( rect.xMinimum(), 0, 'f', 2 )
                   .arg( rect.yMinimum(), 0, 'f', 2 )
                   .arg( rect.xMaximum(), 0, 'f', 2 )
                   .arg( rect.yMaximum(), 0, 'f', 2 );
  }

  QStringList properties;
  if ( currentCrs.isGeographic() )
    properties << tr( "Geographic (uses latitude and longitude for coordinates)" );
  else
  {
    properties << tr( "Units: %1" ).arg( QgsUnitTypes::toString( currentCrs.mapUnits() ) );
  }
  properties << ( currentCrs.isDynamic() ? tr( "Dynamic (relies on a datum which is not plate-fixed)" ) : tr( "Static (relies on a datum which is plate-fixed)" ) );

  try
  {
    const QString celestialBody = currentCrs.celestialBodyName();
    if ( !celestialBody.isEmpty() )
    {
      properties << tr( "Celestial body: %1" ).arg( celestialBody );
    }
  }
  catch ( QgsNotSupportedException & )
  {

  }

  try
  {
    const QgsDatumEnsemble ensemble = currentCrs.datumEnsemble();
    if ( ensemble.isValid() )
    {
      QString id;
      if ( !ensemble.code().isEmpty() )
        id = QStringLiteral( "<i>%1</i> (%2:%3)" ).arg( ensemble.name(), ensemble.authority(), ensemble.code() );
      else
        id = QStringLiteral( "<i>%</i>”" ).arg( ensemble.name() );
      if ( ensemble.accuracy() > 0 )
      {
        properties << tr( "Based on %1, which has a limited accuracy of <b>at best %2 meters</b>." ).arg( id ).arg( ensemble.accuracy() );
      }
      else
      {
        properties << tr( "Based on %1, which has a limited accuracy." ).arg( id );
      }
    }
  }
  catch ( QgsNotSupportedException & )
  {

  }

  const QgsProjOperation operation = currentCrs.operation();
  properties << tr( "Method: %1" ).arg( operation.description() );

  const QString propertiesString = QStringLiteral( "<dt><b>%1</b></dt><dd><ul><li>%2</li></ul></dd>" ).arg( tr( "Properties" ),
                                   properties.join( QLatin1String( "</li><li>" ) ) );

  const QString extentHtml = QStringLiteral( "<dt><b>%1</b></dt><dd>%2</dd>" ).arg( tr( "Extent" ), extentString );
  const QString wktString = QStringLiteral( "<dt><b>%1</b></dt><dd><code>%2</code></dd>" ).arg( tr( "WKT" ), currentCrs.toWkt( QgsCoordinateReferenceSystem::WKT_PREFERRED, true ).replace( '\n', QLatin1String( "<br>" ) ).replace( ' ', QLatin1String( "&nbsp;" ) ) );
  const QString proj4String = QStringLiteral( "<dt><b>%1</b></dt><dd><code>%2</code></dd>" ).arg( tr( "Proj4" ), currentCrs.toProj() );

#ifdef Q_OS_WIN
  const int smallerPointSize = std::max( font().pointSize() - 1, 8 ); // bit less on windows, due to poor rendering of small point sizes
#else
  const int smallerPointSize = std::max( font().pointSize() - 2, 6 );
#endif

  const QModelIndex currentIndex = lstCoordinateSystems->selectionModel()->selectedRows( 0 ).value( 0 );
  QString selectedName;
  if ( currentIndex.isValid() )
  {
    selectedName = currentIndex.data( QgsCoordinateReferenceSystemModel::RoleName ).toString();
  }
  teProjection->setText( QStringLiteral( "<div style=\"font-size: %1pt\"><h3>%2</h3><dl>" ).arg( smallerPointSize ).arg( selectedName ) + propertiesString + wktString + proj4String + extentHtml + QStringLiteral( "</dl></div>" ) );
}

void QgsProjectionSelectionTreeWidget::clearRecentCrs()
{
  // If the list is empty, there is nothing to do
  if ( lstRecent->topLevelItemCount() == 0 )
  {
    return;
  }

  // Ask for confirmation
  if ( QMessageBox::question( this, tr( "Clear Recent CRS" ),
                              tr( "Are you sure you want to clear the list of recently used coordinate reference system?" ),
                              QMessageBox::Yes | QMessageBox::No ) != QMessageBox::Yes )
  {
    return;
  }
  QgsCoordinateReferenceSystem::clearRecentCoordinateReferenceSystems();
  lstRecent->clear();
}

void QgsProjectionSelectionTreeWidget::removeRecentCrsItem( QTreeWidgetItem *item )
{
  if ( !item )
    return;

  int index = lstRecent->indexOfTopLevelItem( item );
  if ( index == -1 )
    return;

  const QString selectedAuthId = item->text( AuthidColumn );
  if ( !selectedAuthId.isEmpty() )
  {
    const QgsCoordinateReferenceSystem crs( selectedAuthId );
    QgsCoordinateReferenceSystem::removeRecentCoordinateReferenceSystem( crs );
  }
  lstRecent->takeTopLevelItem( index );
  delete item;
}
