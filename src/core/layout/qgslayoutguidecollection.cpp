/***************************************************************************
                             qgslayoutguidecollection.cpp
                             ----------------------------
    begin                : July 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayoutguidecollection.h"
#include "qgslayout.h"
#include <QGraphicsLineItem>


//
// QgsLayoutGuide
//

QgsLayoutGuide::QgsLayoutGuide( Orientation orientation, const QgsLayoutMeasurement &position, QgsLayoutItemPage *page )
  : QObject( nullptr )
  , mOrientation( orientation )
  , mPosition( position )
  , mPage( page )
{}

QgsLayoutGuide::~QgsLayoutGuide()
{
  if ( mLayout && mLineItem )
  {
    mLayout->removeItem( mLineItem );
    delete mLineItem;
  }
}

QgsLayoutMeasurement QgsLayoutGuide::position() const
{
  return mPosition;
}

void QgsLayoutGuide::setPosition( const QgsLayoutMeasurement &position )
{
  mPosition = position;
  update();
  emit positionChanged();
}

QgsLayoutItemPage *QgsLayoutGuide::page()
{
  return mPage;
}

void QgsLayoutGuide::setPage( QgsLayoutItemPage *page )
{
  mPage = page;
  update();
}

void QgsLayoutGuide::update()
{
  if ( !mLayout || !mLineItem )
    return;

  // first find matching page
  if ( !mPage )
  {
    mLineItem->hide();
    return;
  }

  if ( mLineItem->parentItem() != mPage )
  {
    mLineItem->setParentItem( mPage );
  }
  double layoutPos = mLayout->convertToLayoutUnits( mPosition );
  bool showGuide = mLayout->guides().visible();
  switch ( mOrientation )
  {
    case Horizontal:
      if ( layoutPos > mPage->rect().height() )
      {
        mLineItem->hide();
      }
      else
      {
        mLineItem->setLine( 0, layoutPos, mPage->rect().width(), layoutPos );
        mLineItem->setVisible( showGuide );
      }

      break;

    case Vertical:
      if ( layoutPos > mPage->rect().width() )
      {
        mLineItem->hide();
      }
      else
      {
        mLineItem->setLine( layoutPos, 0, layoutPos, mPage->rect().height() );
        mLineItem->setVisible( showGuide );
      }

      break;
  }
}

QGraphicsLineItem *QgsLayoutGuide::item()
{
  return mLineItem;
}

double QgsLayoutGuide::layoutPosition() const
{
  if ( !mLineItem )
    return -999;

  switch ( mOrientation )
  {
    case Horizontal:
      return mLineItem->mapToScene( mLineItem->line().p1() ).y();

    case Vertical:
      return mLineItem->mapToScene( mLineItem->line().p1() ).x();
  }
  return -999; // avoid warning
}

void QgsLayoutGuide::setLayoutPosition( double position )
{
  if ( !mLayout )
    return;

  double p = 0;
  switch ( mOrientation )
  {
    case Horizontal:
      p = mLineItem->mapFromScene( QPointF( 0, position ) ).y();
      break;

    case Vertical:
      p = mLineItem->mapFromScene( QPointF( position, 0 ) ).x();
      break;
  }
  mPosition = mLayout->convertFromLayoutUnits( p, mPosition.units() );
  update();
  emit positionChanged();
}

QgsLayout *QgsLayoutGuide::layout() const
{
  return mLayout;
}

void QgsLayoutGuide::setLayout( QgsLayout *layout )
{
  mLayout = layout;

  if ( !mLineItem )
  {
    mLineItem = new QGraphicsLineItem();
    mLineItem->hide();
    mLineItem->setZValue( QgsLayout::ZGuide );
    QPen linePen( Qt::DotLine );
    linePen.setColor( Qt::red );
    // use a pen width of 0, since this activates a cosmetic pen
    // which doesn't scale with the composer and keeps a constant size
    linePen.setWidthF( 0 );
    mLineItem->setPen( linePen );
  }

  mLayout->addItem( mLineItem );
  update();
}

QgsLayoutGuide::Orientation QgsLayoutGuide::orientation() const
{
  return mOrientation;
}



//
// QgsLayoutGuideCollection
//

QgsLayoutGuideCollection::QgsLayoutGuideCollection( QgsLayout *layout, QgsLayoutPageCollection *pageCollection )
  : QAbstractTableModel( layout )
  , mLayout( layout )
  , mPageCollection( pageCollection )
{
  QFont f;
  mHeaderSize = QFontMetrics( f ).width( "XX" );

  connect( mPageCollection, &QgsLayoutPageCollection::pageAboutToBeRemoved, this, &QgsLayoutGuideCollection::pageAboutToBeRemoved );
}

QgsLayoutGuideCollection::~QgsLayoutGuideCollection()
{
  qDeleteAll( mGuides );
}

int QgsLayoutGuideCollection::rowCount( const QModelIndex & ) const
{
  return mGuides.count();
}

int QgsLayoutGuideCollection::columnCount( const QModelIndex &parent ) const
{
  if ( parent.isValid() )
    return 0;

  return 2;
}

QVariant QgsLayoutGuideCollection::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() )
    return QVariant();

  if ( index.row() >= mGuides.count() || index.row() < 0 )
    return QVariant();

  QgsLayoutGuide *guide = mGuides.at( index.row() );
  switch ( role )
  {
    case Qt::DisplayRole:
    case Qt::EditRole:
    {
      if ( index.column() == 0 )
        return guide->position().length();
      else
        return QgsUnitTypes::toAbbreviatedString( guide->position().units() );
    }

    case OrientationRole:
      return guide->orientation();

    case PositionRole:
      return guide->position().length();

    case UnitsRole:
      return guide->position().units();

    case PageRole:
      return mPageCollection->pageNumber( guide->page() );

    case LayoutPositionRole:
      return guide->layoutPosition();

    default:
      return QVariant();
  }
}

bool QgsLayoutGuideCollection::setData( const QModelIndex &index, const QVariant &value, int role )
{
  if ( !index.isValid() )
    return false;

  if ( index.row() >= mGuides.count() || index.row() < 0 )
    return false;

  QgsLayoutGuide *guide = mGuides.at( index.row() );

  switch ( role )
  {
    case  Qt::EditRole:
    {
      bool ok = false;
      double newPos = value.toDouble( &ok );
      if ( !ok )
        return false;

      QgsLayoutMeasurement m = guide->position();
      m.setLength( newPos );
      whileBlocking( guide )->setPosition( m );
      guide->update();
      emit dataChanged( index, index, QVector<int>() << role );
      return true;
    }
    case PositionRole:
    {
      bool ok = false;
      double newPos = value.toDouble( &ok );
      if ( !ok )
        return false;

      QgsLayoutMeasurement m = guide->position();
      m.setLength( newPos );
      whileBlocking( guide )->setPosition( m );
      guide->update();
      emit dataChanged( index, index, QVector<int>() << role );
      return true;
    }
    case UnitsRole:
    {
      bool ok = false;
      int units = value.toInt( &ok );
      if ( !ok )
        return false;

      QgsLayoutMeasurement m = guide->position();
      m.setUnits( static_cast< QgsUnitTypes::LayoutUnit >( units ) );
      whileBlocking( guide )->setPosition( m );
      guide->update();
      emit dataChanged( index, index, QVector<int>() << role );
      return true;
    }
  }

  return false;
}

Qt::ItemFlags QgsLayoutGuideCollection::flags( const QModelIndex &index ) const
{
  if ( !index.isValid() )
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
  return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
}

QVariant QgsLayoutGuideCollection::headerData( int section, Qt::Orientation orientation, int role ) const
{
  if ( role == Qt::DisplayRole )
    return QVariant();
  else if ( role == Qt::SizeHintRole )
  {
    return QSize( mHeaderSize, mHeaderSize );
  }
  return QAbstractTableModel::headerData( section, orientation, role );
}

bool QgsLayoutGuideCollection::removeRows( int row, int count, const QModelIndex &parent )
{
  if ( parent.isValid() )
    return false;

  beginRemoveRows( parent, row, row + count - 1 );
  for ( int i = 0; i < count; ++ i )
  {
    delete mGuides.takeAt( row );
  }
  endRemoveRows();
  return true;
}

void QgsLayoutGuideCollection::addGuide( QgsLayoutGuide *guide )
{
  guide->setLayout( mLayout );

  beginInsertRows( QModelIndex(), mGuides.count(), mGuides.count() );
  mGuides.append( guide );
  endInsertRows();

  QModelIndex index = createIndex( mGuides.length() - 1, 0 );
  connect( guide, &QgsLayoutGuide::positionChanged, this, [ this, index ]
  {
    emit dataChanged( index, index );
  } );
}

void QgsLayoutGuideCollection::removeGuide( QgsLayoutGuide *guide )
{
  int row = mGuides.indexOf( guide );
  if ( row < 0 )
    return;

  removeRow( row );
}

void QgsLayoutGuideCollection::clear()
{
  beginResetModel();
  qDeleteAll( mGuides );
  mGuides.clear();
  endResetModel();
}

void QgsLayoutGuideCollection::applyGuidesToAllOtherPages( int sourcePage )
{
  QgsLayoutItemPage *page = mPageCollection->page( sourcePage );
  // remove other page's guides
  Q_FOREACH ( QgsLayoutGuide *guide, mGuides )
  {
    if ( guide->page() != page )
      removeGuide( guide );
  }

  // remaining guides belong to source page - clone them to other pages
  Q_FOREACH ( QgsLayoutGuide *guide, mGuides )
  {
    for ( int p = 0; p < mPageCollection->pageCount(); ++p )
    {
      if ( p == sourcePage )
        continue;

      std::unique_ptr< QgsLayoutGuide> newGuide( new QgsLayoutGuide( guide->orientation(), guide->position(), mPageCollection->page( p ) ) );
      newGuide->setLayout( mLayout );
      if ( newGuide->item()->isVisible() )
      {
        // if invisible, new guide is outside of page bounds
        addGuide( newGuide.release() );
      }
    }
  }
}

void QgsLayoutGuideCollection::update()
{
  Q_FOREACH ( QgsLayoutGuide *guide, mGuides )
  {
    guide->update();
  }
}

QList<QgsLayoutGuide *> QgsLayoutGuideCollection::guides( QgsLayoutGuide::Orientation orientation, int page )
{
  QList<QgsLayoutGuide *> res;
  Q_FOREACH ( QgsLayoutGuide *guide, mGuides )
  {
    if ( guide->orientation() == orientation && guide->item()->isVisible() &&
         ( page < 0 || mPageCollection->page( page ) == guide->page() ) )
      res << guide;
  }
  return res;
}

QList<QgsLayoutGuide *> QgsLayoutGuideCollection::guidesOnPage( int page )
{
  QList<QgsLayoutGuide *> res;
  Q_FOREACH ( QgsLayoutGuide *guide, mGuides )
  {
    if ( mPageCollection->page( page ) == guide->page() )
      res << guide;
  }
  return res;
}

bool QgsLayoutGuideCollection::visible() const
{
  return mGuidesVisible;
}

void QgsLayoutGuideCollection::setVisible( bool visible )
{
  mGuidesVisible = visible;
  update();
}

void QgsLayoutGuideCollection::pageAboutToBeRemoved( int pageNumber )
{
  Q_FOREACH ( QgsLayoutGuide *guide, guidesOnPage( pageNumber ) )
  {
    removeGuide( guide );
  }
}



//
// QgsLayoutGuideProxyModel
//

QgsLayoutGuideProxyModel::QgsLayoutGuideProxyModel( QObject *parent, QgsLayoutGuide::Orientation orientation, int page )
  : QSortFilterProxyModel( parent )
  , mOrientation( orientation )
  , mPage( page )
{
  setDynamicSortFilter( true );
  sort( 0 );
}

void QgsLayoutGuideProxyModel::setPage( int page )
{
  mPage = page;
  invalidateFilter();
}

bool QgsLayoutGuideProxyModel::filterAcceptsRow( int source_row, const QModelIndex &source_parent ) const
{
  QModelIndex index = sourceModel()->index( source_row, 0, source_parent );
  QgsLayoutGuide::Orientation orientation = static_cast< QgsLayoutGuide::Orientation>( sourceModel()->data( index, QgsLayoutGuideCollection::OrientationRole ).toInt() );
  if ( orientation != mOrientation )
    return false;

  int page = sourceModel()->data( index, QgsLayoutGuideCollection::PageRole ).toInt();
  return page == mPage;
}

bool QgsLayoutGuideProxyModel::lessThan( const QModelIndex &left, const QModelIndex &right ) const
{
  double leftPos = sourceModel()->data( left, QgsLayoutGuideCollection::LayoutPositionRole ).toDouble();
  double rightPos = sourceModel()->data( right, QgsLayoutGuideCollection::LayoutPositionRole ).toDouble();
  return leftPos < rightPos;
}
