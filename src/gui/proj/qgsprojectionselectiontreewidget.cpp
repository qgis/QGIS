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

#ifdef ENABLE_MODELTEST
#include "modeltest.h"
#endif

QgsProjectionSelectionTreeWidget::QgsProjectionSelectionTreeWidget( QWidget *parent, QgsCoordinateReferenceSystemProxyModel::Filters filters )
  : QWidget( parent )
{
  setupUi( this );

  mCrsModel = new QgsCoordinateReferenceSystemProxyModel( this );
  mCrsModel->setFilters( filters );

  mRecentCrsModel = new QgsRecentCoordinateReferenceSystemTableModel( this );
  mRecentCrsModel->setFilters( filters );

  lstCoordinateSystems->setModel( mCrsModel );
  lstCoordinateSystems->setSelectionBehavior( QAbstractItemView::SelectRows );

  lstRecent->setModel( mRecentCrsModel );
  lstRecent->viewport()->setAttribute( Qt::WA_Hover );
  lstRecent->setSelectionBehavior( QAbstractItemView::SelectRows );
  lstRecent->setRootIsDecorated( false );

  RemoveRecentCrsDelegate *removeDelegate = new RemoveRecentCrsDelegate( lstRecent );
  lstRecent->setItemDelegateForColumn( 2, removeDelegate );
  lstRecent->viewport()->installEventFilter( removeDelegate );

  if ( mCrsModel->rowCount() == 1 )
  {
    // if only one group, expand it by default
    lstCoordinateSystems->expand( mCrsModel->index( 0, 0, QModelIndex() ) );
  }

  QFont f = teProjection->font();
  f.setPointSize( f.pointSize() - 2 );
  teProjection->setFont( f );

  leSearch->setShowSearchIcon( true );

  connect( lstCoordinateSystems, &QTreeView::doubleClicked, this, &QgsProjectionSelectionTreeWidget::lstCoordinateSystemsDoubleClicked );
  connect( lstRecent, &QTreeView::doubleClicked, this, &QgsProjectionSelectionTreeWidget::lstRecentDoubleClicked );
  connect( lstRecent, &QTreeView::clicked, this, &QgsProjectionSelectionTreeWidget::lstRecentClicked );
  connect( lstCoordinateSystems->selectionModel(), &QItemSelectionModel::selectionChanged, this, &QgsProjectionSelectionTreeWidget::lstCoordinateSystemsSelectionChanged );
  connect( lstRecent->selectionModel(), &QItemSelectionModel::selectionChanged, this, &QgsProjectionSelectionTreeWidget::lstRecentSelectionChanged );
  connect( cbxHideDeprecated, &QCheckBox::toggled, this, [ = ]( bool selected )
  {
    mCrsModel->setFilterDeprecated( selected );
    mRecentCrsModel->setFilterDeprecated( selected );
  } );
  connect( leSearch, &QgsFilterLineEdit::textChanged, this, [ = ]( const QString & filter )
  {
    mCrsModel->setFilterString( filter );
    mRecentCrsModel->setFilterString( filter );
  } );

  mAreaCanvas->setVisible( mShowMap );

  lstCoordinateSystems->header()->setSectionResizeMode( AuthidColumn, QHeaderView::Stretch );
  lstRecent->header()->setSectionResizeMode( AuthidColumn, QHeaderView::Stretch );

  // Clear Crs Column
  lstRecent->header()->setMinimumSectionSize( 10 );
  lstRecent->header()->setStretchLastSection( false );
  lstRecent->header()->resizeSection( ClearColumn, 20 );

  // Clear recent crs context menu
  lstRecent->setContextMenuPolicy( Qt::CustomContextMenu );
  connect( lstRecent, &QTreeView::customContextMenuRequested, this, [this]( const QPoint & pos )
  {
    // If list is empty, do nothing
    if ( lstRecent->model()->rowCount() == 0 )
      return;
    QMenu menu;
    // Clear selected
    const QModelIndex currentIndex = lstRecent->indexAt( pos );
    if ( currentIndex.isValid() )
    {
      QAction *clearSelected = menu.addAction( QgsApplication::getThemeIcon( "/mIconClearItem.svg" ),  tr( "Remove Selected CRS from Recently Used CRS" ) );
      connect( clearSelected, &QAction::triggered, this, [this, currentIndex ] { removeRecentCrsItem( currentIndex ); } );
      menu.addSeparator();
    }
    // Clear all
    QAction *clearAll = menu.addAction( QgsApplication::getThemeIcon( "/console/iconClearConsole.svg" ), tr( "Clear All Recently Used CRS" ) );
    connect( clearAll, &QAction::triggered, this, &QgsProjectionSelectionTreeWidget::clearRecentCrs );
    menu.exec( lstRecent->viewport()->mapToGlobal( pos ) );
  } );

  // Install event filter to catch delete key press on the recent crs list
  lstRecent->installEventFilter( this );

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
    QgsApplication::coordinateReferenceSystemRegistry()->pushRecent( selectedCrs );
}

void QgsProjectionSelectionTreeWidget::resizeEvent( QResizeEvent *event )
{
  lstCoordinateSystems->header()->resizeSection( NameColumn, event->size().width() - 240 );
  lstCoordinateSystems->header()->resizeSection( AuthidColumn, 240 );

  lstRecent->header()->resizeSection( NameColumn, event->size().width() - 260 );
  lstRecent->header()->resizeSection( AuthidColumn, 240 );
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
    const QModelIndex currentIndex = lstRecent->selectionModel()->selectedRows( 0 ).value( 0 );
    if ( currentIndex.isValid() )
      removeRecentCrsItem( currentIndex );
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
  mRecentCrsModel->setFilters( filters );
  if ( mCrsModel->rowCount() == 1 )
  {
    // if only one group, expand it by default
    lstCoordinateSystems->expand( mCrsModel->index( 0, 0, QModelIndex() ) );
  }
}

QgsCoordinateReferenceSystem QgsProjectionSelectionTreeWidget::crs() const
{
  if ( mCheckBoxNoProjection->isEnabled() && mCheckBoxNoProjection->isChecked() )
    return QgsCoordinateReferenceSystem();

  const QModelIndex currentIndex = lstCoordinateSystems->selectionModel()->selectedRows( 0 ).value( 0 );
  const QString authid = currentIndex.data( static_cast< int >( QgsCoordinateReferenceSystemModel::CustomRole::AuthId ) ).toString();
  if ( !authid.isEmpty() )
  {
    return QgsCoordinateReferenceSystem::fromOgcWmsCrs( authid );
  }
  else
  {
    // custom CRS
    const QString wkt = currentIndex.data( static_cast< int >( QgsCoordinateReferenceSystemModel::CustomRole::Wkt ) ).toString();
    const QString proj = currentIndex.data( static_cast< int >( QgsCoordinateReferenceSystemModel::CustomRole::Proj ) ).toString();

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
    const QString authid = currentIndex.data( static_cast< int >( QgsCoordinateReferenceSystemModel::CustomRole::AuthId ) ).toString();
    const QString wkt = currentIndex.data( static_cast< int >( QgsCoordinateReferenceSystemModel::CustomRole::Wkt ) ).toString();
    const QString proj = currentIndex.data( static_cast< int >( QgsCoordinateReferenceSystemModel::CustomRole::Proj ) ).toString();
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

    const QString crsAuthId = mCrsModel->coordinateReferenceSystemModel()->data( sourceIndex, static_cast< int >( QgsCoordinateReferenceSystemModel::CustomRole::AuthId ) ).toString();
    if ( !crsAuthId.isEmpty() )
    {
      const QModelIndexList recentMatches = mRecentCrsModel->match( mRecentCrsModel->index( 0, 0 ),
                                            static_cast< int >( QgsRecentCoordinateReferenceSystemsModel::CustomRole::AuthId ),
                                            crsAuthId );
      if ( !recentMatches.isEmpty() )
      {
        QgsDebugMsgLevel( QStringLiteral( "found srs %1 in recent" ).arg( crsAuthId ), 4 );

        lstRecent->selectionModel()->select( recentMatches.at( 0 ), QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows );
        lstRecent->scrollTo( recentMatches.at( 0 ) );
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

void QgsProjectionSelectionTreeWidget::lstRecentSelectionChanged( const QItemSelection &selected, const QItemSelection & )
{
  if ( selected.isEmpty() )
  {
    QgsDebugMsgLevel( QStringLiteral( "no current item" ), 4 );
    return;
  }

  const QModelIndex selectedIndex = lstRecent->selectionModel()->selectedRows( 0 ).value( 0 );
  if ( !selectedIndex.isValid() )
    return;

  lstRecent->scrollTo( selectedIndex );

  const QString selectedAuthId = mRecentCrsModel->crs( selectedIndex ).authid();
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

void QgsProjectionSelectionTreeWidget::lstRecentDoubleClicked( const QModelIndex &index )
{
  QgsDebugMsgLevel( QStringLiteral( "Entered." ), 4 );
  if ( !index.isValid() )
  {
    QgsDebugMsgLevel( QStringLiteral( "no current item" ), 4 );
    return;
  }

  emit projectionDoubleClicked();
}

void QgsProjectionSelectionTreeWidget::lstRecentClicked( const QModelIndex &index )
{
  if ( index.column() == ClearColumn )
  {
    removeRecentCrsItem( index );
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
  const QString wktString = QStringLiteral( "<dt><b>%1</b></dt><dd><code>%2</code></dd>" ).arg( tr( "WKT" ), currentCrs.toWkt( Qgis::CrsWktVariant::Preferred, true ).replace( '\n', QLatin1String( "<br>" ) ).replace( ' ', QLatin1String( "&nbsp;" ) ) );
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
    selectedName = currentIndex.data( static_cast< int >( QgsCoordinateReferenceSystemModel::CustomRole::Name ) ).toString();
  }
  teProjection->setText( QStringLiteral( "<div style=\"font-size: %1pt\"><h3>%2</h3><dl>" ).arg( smallerPointSize ).arg( selectedName ) + propertiesString + wktString + proj4String + extentHtml + QStringLiteral( "</dl></div>" ) );
}

void QgsProjectionSelectionTreeWidget::clearRecentCrs()
{
  // If the list is empty, there is nothing to do
  if ( QgsApplication::coordinateReferenceSystemRegistry()->recentCrs().isEmpty() )
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
  QgsApplication::coordinateReferenceSystemRegistry()->clearRecent();
}

void QgsProjectionSelectionTreeWidget::removeRecentCrsItem( const QModelIndex &index )
{
  const QgsCoordinateReferenceSystem selectedRecentCrs = mRecentCrsModel->crs( index );
  QgsApplication::coordinateReferenceSystemRegistry()->removeRecent( selectedRecentCrs );
}


///@cond PRIVATE
QgsRecentCoordinateReferenceSystemTableModel::QgsRecentCoordinateReferenceSystemTableModel( QObject *parent )
  : QgsRecentCoordinateReferenceSystemsProxyModel( parent, 3 )
{
#ifdef ENABLE_MODELTEST
  new ModelTest( this, this );
#endif
}

QVariant QgsRecentCoordinateReferenceSystemTableModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
  if ( orientation == Qt::Horizontal )
  {
    switch ( role )
    {
      case Qt::DisplayRole:
        switch ( section )
        {
          case 0:
            return tr( "Coordinate Reference System" );
          case 1:
            return tr( "Authority ID" );
          case 2:
            return QString();
          default:
            break;
        }
        break;

      default:
        break;
    }
  }
  return QVariant();
}

QVariant QgsRecentCoordinateReferenceSystemTableModel::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() )
    return QVariant();

  const int column = index.column();
  switch ( column )
  {
    case 1:
    {
      const QgsCoordinateReferenceSystem lCrs = crs( index );
      switch ( role )
      {
        case Qt::DisplayRole:
        case Qt::ToolTipRole:
          return lCrs.authid();

        default:
          break;
      }
      break;
    }

    case 2:
    {
      switch ( role )
      {
        case Qt::ToolTipRole:
          return tr( "Remove from recently used CRS" );

        default:
          break;
      }
      break;
    }

    default:
      break;
  }
  return QgsRecentCoordinateReferenceSystemsProxyModel::data( index, role );
}


//
// RemoveRecentCrsDelegate
//

RemoveRecentCrsDelegate::RemoveRecentCrsDelegate( QObject *parent )
  : QStyledItemDelegate( parent )
{

}

bool RemoveRecentCrsDelegate::eventFilter( QObject *obj, QEvent *event )
{
  if ( event->type() == QEvent::HoverEnter || event->type() == QEvent::HoverMove )
  {
    QHoverEvent *hoverEvent = static_cast<QHoverEvent *>( event );
    if ( QAbstractItemView *view = qobject_cast<QAbstractItemView *>( obj->parent() ) )
    {
      const QModelIndex indexUnderMouse = view->indexAt( hoverEvent->pos() );
      setHoveredIndex( indexUnderMouse );
      view->viewport()->update();
    }
  }
  else if ( event->type() == QEvent::HoverLeave )
  {
    setHoveredIndex( QModelIndex() );
    qobject_cast< QWidget * >( obj )->update();
  }
  return QStyledItemDelegate::eventFilter( obj, event );
}

void RemoveRecentCrsDelegate::paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  QStyledItemDelegate::paint( painter, option, index );

  if ( index == mHoveredIndex )
  {
    QStyleOptionButton buttonOption;
    buttonOption.initFrom( option.widget );
    buttonOption.rect = option.rect;

    option.widget->style()->drawControl( QStyle::CE_PushButton, &buttonOption, painter );
  }

  const QIcon icon = QgsApplication::getThemeIcon( "/mIconClearItem.svg" );
  const QRect iconRect( option.rect.left() + ( option.rect.width() - 16 ) / 2,
                        option.rect.top() + ( option.rect.height() - 16 ) / 2,
                        16, 16 );

  icon.paint( painter, iconRect );
}

void RemoveRecentCrsDelegate::setHoveredIndex( const QModelIndex &index )
{
  mHoveredIndex = index;
}

///@endcond PRIVATE
