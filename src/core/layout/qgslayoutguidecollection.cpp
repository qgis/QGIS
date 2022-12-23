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
#include "qgsproject.h"
#include "qgsreadwritecontext.h"
#include "qgslayoutpagecollection.h"
#include "qgslayoutundostack.h"
#include <QGraphicsLineItem>


//
// QgsLayoutGuide
//

QgsLayoutGuide::QgsLayoutGuide( Qt::Orientation orientation, QgsLayoutMeasurement position, QgsLayoutItemPage *page )
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

void QgsLayoutGuide::setPosition( QgsLayoutMeasurement position )
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

  double layoutPos = mLayout->convertToLayoutUnits( mPosition );
  bool showGuide = mLayout->guides().visible();
  switch ( mOrientation )
  {
    case Qt::Horizontal:
      if ( layoutPos > mPage->rect().height() )
      {
        mLineItem->hide();
      }
      else
      {
        mLineItem->setLine( 0, layoutPos + mPage->y(), mPage->rect().width(), layoutPos + mPage->y() );
        mLineItem->setVisible( showGuide );
      }

      break;

    case Qt::Vertical:
      if ( layoutPos > mPage->rect().width() )
      {
        mLineItem->hide();
      }
      else
      {
        mLineItem->setLine( layoutPos, mPage->y(), layoutPos, mPage->y() + mPage->rect().height() );
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
    case Qt::Horizontal:
      return mLineItem->mapToScene( mLineItem->line().p1() ).y();

    case Qt::Vertical:
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
    case Qt::Horizontal:
      p = mPage->mapFromScene( QPointF( 0, position ) ).y();
      break;

    case Qt::Vertical:
      p = mPage->mapFromScene( QPointF( position, 0 ) ).x();
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
    // which doesn't scale with the layout and keeps a constant size
    linePen.setWidthF( 0 );
    mLineItem->setPen( linePen );
  }

  mLayout->addItem( mLineItem );
  update();
}

Qt::Orientation QgsLayoutGuide::orientation() const
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
  mHeaderSize = QFontMetrics( f ).boundingRect( QStringLiteral( "XX" ) ).width();

  connect( mPageCollection, &QgsLayoutPageCollection::pageAboutToBeRemoved, this, &QgsLayoutGuideCollection::pageAboutToBeRemoved );
}

QgsLayoutGuideCollection::~QgsLayoutGuideCollection()
{
  qDeleteAll( mGuides );
}

QgsLayout *QgsLayoutGuideCollection::layout()
{
  return mLayout;
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
      mLayout->undoStack()->beginCommand( mPageCollection, tr( "Move Guide" ), Move + index.row() );
      whileBlocking( guide )->setPosition( m );
      guide->update();
      mLayout->undoStack()->endCommand();
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
      if ( qgsDoubleNear( m.length(), newPos ) )
        return true;

      m.setLength( newPos );
      mLayout->undoStack()->beginCommand( mPageCollection, tr( "Move Guide" ), Move + index.row() );
      whileBlocking( guide )->setPosition( m );
      guide->update();
      mLayout->undoStack()->endCommand();
      emit dataChanged( index, index, QVector<int>() << role );
      return true;
    }

    case LayoutPositionRole:
    {
      bool ok = false;
      double newPos = value.toDouble( &ok );
      if ( !ok )
        return false;

      mLayout->undoStack()->beginCommand( mPageCollection, tr( "Move Guide" ), Move + index.row() );
      whileBlocking( guide )->setLayoutPosition( newPos );
      mLayout->undoStack()->endCommand();
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
      mLayout->undoStack()->beginCommand( mPageCollection, tr( "Move Guide" ), Move + index.row() );
      whileBlocking( guide )->setPosition( m );
      guide->update();
      mLayout->undoStack()->endCommand();
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

  if ( !mBlockUndoCommands )
    mLayout->undoStack()->beginCommand( mPageCollection, tr( "Remove Guide(s)" ), Remove + row );
  beginRemoveRows( parent, row, row + count - 1 );
  for ( int i = 0; i < count; ++ i )
  {
    delete mGuides.takeAt( row );
  }
  endRemoveRows();
  if ( !mBlockUndoCommands )
    mLayout->undoStack()->endCommand();
  return true;
}

void QgsLayoutGuideCollection::addGuide( QgsLayoutGuide *guide )
{
  if ( guide->layout() != mLayout )
    guide->setLayout( mLayout );

  if ( !mBlockUndoCommands )
    mLayout->undoStack()->beginCommand( mPageCollection, tr( "Create Guide" ) );
  beginInsertRows( QModelIndex(), mGuides.count(), mGuides.count() );
  mGuides.append( guide );
  endInsertRows();
  if ( !mBlockUndoCommands )
    mLayout->undoStack()->endCommand();

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

void QgsLayoutGuideCollection::setGuideLayoutPosition( QgsLayoutGuide *guide, double position )
{
  int row = mGuides.indexOf( guide );
  if ( row < 0 )
    return;

  setData( index( row, 0 ), position, LayoutPositionRole );
}

void QgsLayoutGuideCollection::clear()
{
  mLayout->undoStack()->beginCommand( mPageCollection, tr( "Clear Guides" ) );
  beginResetModel();
  qDeleteAll( mGuides );
  mGuides.clear();
  endResetModel();
  mLayout->undoStack()->endCommand();
}

void QgsLayoutGuideCollection::applyGuidesToAllOtherPages( int sourcePage )
{
  mLayout->undoStack()->beginCommand( mPageCollection, tr( "Apply Guides" ) );
  mBlockUndoCommands = true;
  QgsLayoutItemPage *page = mPageCollection->page( sourcePage );
  // remove other page's guides
  const auto constMGuides = mGuides;
  for ( QgsLayoutGuide *guide : constMGuides )
  {
    if ( guide->page() != page )
      removeGuide( guide );
  }

  // remaining guides belong to source page - clone them to other pages
  for ( QgsLayoutGuide *guide : std::as_const( mGuides ) )
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
  mLayout->undoStack()->endCommand();
  mBlockUndoCommands = false;
}

void QgsLayoutGuideCollection::update()
{
  const auto constMGuides = mGuides;
  for ( QgsLayoutGuide *guide : constMGuides )
  {
    guide->update();
  }
}

QList<QgsLayoutGuide *> QgsLayoutGuideCollection::guides()
{
  return mGuides;
}

QList<QgsLayoutGuide *> QgsLayoutGuideCollection::guides( Qt::Orientation orientation, int page )
{
  QList<QgsLayoutGuide *> res;
  const auto constMGuides = mGuides;
  for ( QgsLayoutGuide *guide : constMGuides )
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
  const auto constMGuides = mGuides;
  for ( QgsLayoutGuide *guide : constMGuides )
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
  mLayout->undoStack()->beginCommand( mPageCollection, tr( "Change Guide Visibility" ) );
  mGuidesVisible = visible;
  mLayout->undoStack()->endCommand();
  update();
}

void QgsLayoutGuideCollection::pageAboutToBeRemoved( int pageNumber )
{
  mBlockUndoCommands = true;
  const auto constGuidesOnPage = guidesOnPage( pageNumber );
  for ( QgsLayoutGuide *guide : constGuidesOnPage )
  {
    removeGuide( guide );
  }
  mBlockUndoCommands = false;
}

bool QgsLayoutGuideCollection::writeXml( QDomElement &parentElement, QDomDocument &document, const QgsReadWriteContext & ) const
{
  QDomElement element = document.createElement( QStringLiteral( "GuideCollection" ) );
  element.setAttribute( QStringLiteral( "visible" ), mGuidesVisible );
  const auto constMGuides = mGuides;
  for ( QgsLayoutGuide *guide : constMGuides )
  {
    QDomElement guideElement = document.createElement( QStringLiteral( "Guide" ) );
    guideElement.setAttribute( QStringLiteral( "orientation" ), guide->orientation() );
    guideElement.setAttribute( QStringLiteral( "page" ), mPageCollection->pageNumber( guide->page() ) );
    guideElement.setAttribute( QStringLiteral( "position" ), guide->position().length() );
    guideElement.setAttribute( QStringLiteral( "units" ), QgsUnitTypes::encodeUnit( guide->position().units() ) );
    element.appendChild( guideElement );
  }

  parentElement.appendChild( element );
  return true;
}

bool QgsLayoutGuideCollection::readXml( const QDomElement &e, const QDomDocument &, const QgsReadWriteContext & )
{
  QDomElement element = e;
  if ( element.nodeName() != QLatin1String( "GuideCollection" ) )
  {
    element = element.firstChildElement( QStringLiteral( "GuideCollection" ) );
  }

  if ( element.nodeName() != QLatin1String( "GuideCollection" ) )
  {
    return false;
  }

  mBlockUndoCommands = true;
  beginResetModel();
  qDeleteAll( mGuides );
  mGuides.clear();

  mGuidesVisible = element.attribute( QStringLiteral( "visible" ), QStringLiteral( "0" ) ) != QLatin1String( "0" );
  QDomNodeList guideNodeList = element.elementsByTagName( QStringLiteral( "Guide" ) );
  for ( int i = 0; i < guideNodeList.size(); ++i )
  {
    QDomElement element = guideNodeList.at( i ).toElement();
    Qt::Orientation orientation = static_cast< Qt::Orientation >( element.attribute( QStringLiteral( "orientation" ), QStringLiteral( "1" ) ).toInt() );
    double pos = element.attribute( QStringLiteral( "position" ), QStringLiteral( "0" ) ).toDouble();
    QgsUnitTypes::LayoutUnit unit = QgsUnitTypes::decodeLayoutUnit( element.attribute( QStringLiteral( "units" ) ) );
    int page = element.attribute( QStringLiteral( "page" ), QStringLiteral( "0" ) ).toInt();
    std::unique_ptr< QgsLayoutGuide > guide( new QgsLayoutGuide( orientation, QgsLayoutMeasurement( pos, unit ), mPageCollection->page( page ) ) );
    guide->update();
    addGuide( guide.release() );
  }

  endResetModel();
  mBlockUndoCommands = false;
  return true;
}

//
// QgsLayoutGuideProxyModel
//

QgsLayoutGuideProxyModel::QgsLayoutGuideProxyModel( QObject *parent, Qt::Orientation orientation, int page )
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
  Qt::Orientation orientation = static_cast< Qt::Orientation>( sourceModel()->data( index, QgsLayoutGuideCollection::OrientationRole ).toInt() );
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
