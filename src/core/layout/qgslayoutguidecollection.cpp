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

QgsLayoutGuide::QgsLayoutGuide( Orientation orientation, const QgsLayoutMeasurement &position )
  : QObject( nullptr )
  , mOrientation( orientation )
  , mPosition( position )
  , mLineItem( new QGraphicsLineItem() )
{
  mLineItem->hide();
  mLineItem->setZValue( QgsLayout::ZGuide );
  QPen linePen( Qt::DotLine );
  linePen.setColor( Qt::red );
  // use a pen width of 0, since this activates a cosmetic pen
  // which doesn't scale with the composer and keeps a constant size
  linePen.setWidthF( 0 );
  mLineItem->setPen( linePen );
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

int QgsLayoutGuide::page() const
{
  return mPage;
}

void QgsLayoutGuide::setPage( int page )
{
  mPage = page;
  update();
}

void QgsLayoutGuide::update()
{
  if ( !mLayout )
    return;

  // first find matching page
  if ( mPage >= mLayout->pageCollection()->pageCount() )
  {
    mLineItem->hide();
    return;
  }

  QgsLayoutItemPage *page = mLayout->pageCollection()->page( mPage );
  mLineItem->setParentItem( page );
  double layoutPos = mLayout->convertToLayoutUnits( mPosition );
  switch ( mOrientation )
  {
    case Horizontal:
      if ( layoutPos > page->rect().height() )
      {
        mLineItem->hide();
      }
      else
      {
        mLineItem->setLine( 0, layoutPos, page->rect().width(), layoutPos );
        mLineItem->show();
      }

      break;

    case Vertical:
      if ( layoutPos > page->rect().width() )
      {
        mLineItem->hide();
      }
      else
      {
        mLineItem->setLine( layoutPos, 0, layoutPos, page->rect().height() );
        mLineItem->show();
      }

      break;
  }
}

QGraphicsLineItem *QgsLayoutGuide::item()
{
  return mLineItem.get();
}

double QgsLayoutGuide::layoutPosition() const
{
  switch ( mOrientation )
  {
    case Horizontal:
      return mLineItem->line().y1();

    case Vertical:
      return mLineItem->line().x1();
  }
  return -999; // avoid warning
}

QgsLayout *QgsLayoutGuide::layout() const
{
  return mLayout;
}

void QgsLayoutGuide::setLayout( QgsLayout *layout )
{
  mLayout = layout;
  mLayout->addItem( mLineItem.get() );
  update();
}

QgsLayoutGuide::Orientation QgsLayoutGuide::orientation() const
{
  return mOrientation;
}



//
// QgsLayoutGuideCollection
//

QgsLayoutGuideCollection::QgsLayoutGuideCollection( QgsLayout *layout )
  : QAbstractTableModel( layout )
  , mLayout( layout )
{
  QFont f;
  mHeaderSize = QFontMetrics( f ).width( "XX" );
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
      return guide->page();

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

void QgsLayoutGuideCollection::update()
{
  Q_FOREACH ( QgsLayoutGuide *guide, mGuides )
  {
    guide->update();
  }
}

QList<QgsLayoutGuide *> QgsLayoutGuideCollection::guides( QgsLayoutGuide::Orientation orientation )
{
  QList<QgsLayoutGuide *> res;
  Q_FOREACH ( QgsLayoutGuide *guide, mGuides )
  {
    if ( guide->orientation() == orientation && guide->item()->isVisible() )
      res << guide;
  }
  return res;
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
