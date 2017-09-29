/***************************************************************************
                              qgslayout.cpp
                             -------------------
    begin                : June 2017
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

#include "qgslayout.h"
#include "qgslayoutitem.h"
#include "qgslayoutpagecollection.h"
#include "qgslayoutguidecollection.h"
#include "qgsreadwritecontext.h"
#include "qgsproject.h"

QgsLayout::QgsLayout( QgsProject *project )
  : mProject( project )
  , mSnapper( QgsLayoutSnapper( this ) )
  , mGridSettings( this )
  , mPageCollection( new QgsLayoutPageCollection( this ) )
  , mUndoStack( new QgsLayoutUndoStack( this ) )
{
  // just to make sure - this should be the default, but maybe it'll change in some future Qt version...
  setBackgroundBrush( Qt::NoBrush );
}

void QgsLayout::initializeDefaults()
{
  // default to a A4 landscape page
  QgsLayoutItemPage *page = new QgsLayoutItemPage( this );
  page->setPageSize( QgsLayoutSize( 297, 210, QgsUnitTypes::LayoutMillimeters ) );
  mPageCollection->addPage( page );
  mUndoStack->stack()->clear();
}

QgsProject *QgsLayout::project() const
{
  return mProject;
}

QList<QgsLayoutItem *> QgsLayout::selectedLayoutItems( const bool includeLockedItems )
{
  QList<QgsLayoutItem *> layoutItemList;

  const QList<QGraphicsItem *> graphicsItemList = selectedItems();
  for ( QGraphicsItem *item : graphicsItemList )
  {
    QgsLayoutItem *layoutItem = dynamic_cast<QgsLayoutItem *>( item );
    if ( layoutItem && ( includeLockedItems || !layoutItem->isLocked() ) )
    {
      layoutItemList.push_back( layoutItem );
    }
  }

  return layoutItemList;
}

QgsLayoutItem *QgsLayout::itemByUuid( const QString &uuid )
{
  QList<QgsLayoutItem *> itemList;
  layoutItems( itemList );
  Q_FOREACH ( QgsLayoutItem *item, itemList )
  {
    if ( item->uuid() == uuid )
    {
      return item;
    }
  }

  return nullptr;
}

double QgsLayout::convertToLayoutUnits( const QgsLayoutMeasurement &measurement ) const
{
  return mContext.measurementConverter().convert( measurement, mUnits ).length();
}

QSizeF QgsLayout::convertToLayoutUnits( const QgsLayoutSize &size ) const
{
  return mContext.measurementConverter().convert( size, mUnits ).toQSizeF();
}

QPointF QgsLayout::convertToLayoutUnits( const QgsLayoutPoint &point ) const
{
  return mContext.measurementConverter().convert( point, mUnits ).toQPointF();
}

QgsLayoutMeasurement QgsLayout::convertFromLayoutUnits( const double length, const QgsUnitTypes::LayoutUnit unit ) const
{
  return mContext.measurementConverter().convert( QgsLayoutMeasurement( length, mUnits ), unit );
}

QgsLayoutSize QgsLayout::convertFromLayoutUnits( const QSizeF &size, const QgsUnitTypes::LayoutUnit unit ) const
{
  return mContext.measurementConverter().convert( QgsLayoutSize( size.width(), size.height(), mUnits ), unit );
}

QgsLayoutPoint QgsLayout::convertFromLayoutUnits( const QPointF &point, const QgsUnitTypes::LayoutUnit unit ) const
{
  return mContext.measurementConverter().convert( QgsLayoutPoint( point.x(), point.y(), mUnits ), unit );
}

QgsLayoutGuideCollection &QgsLayout::guides()
{
  return mPageCollection->guides();
}

const QgsLayoutGuideCollection &QgsLayout::guides() const
{
  return mPageCollection->guides();
}

QgsExpressionContext QgsLayout::createExpressionContext() const
{
  QgsExpressionContext context = QgsExpressionContext();
  context.appendScope( QgsExpressionContextUtils::globalScope() );
  context.appendScope( QgsExpressionContextUtils::projectScope( mProject ) );
  context.appendScope( QgsExpressionContextUtils::layoutScope( this ) );
#if 0 //TODO
  if ( mAtlasComposition.enabled() )
  {
    context.appendScope( QgsExpressionContextUtils::atlasScope( &mAtlasComposition ) );
  }
#endif
  return context;
}

void QgsLayout::setCustomProperty( const QString &key, const QVariant &value )
{
  mCustomProperties.setValue( key, value );

  if ( key.startsWith( QLatin1String( "variable" ) ) )
    emit variablesChanged();
}

QVariant QgsLayout::customProperty( const QString &key, const QVariant &defaultValue ) const
{
  return mCustomProperties.value( key, defaultValue );
}

void QgsLayout::removeCustomProperty( const QString &key )
{
  mCustomProperties.remove( key );
}

QStringList QgsLayout::customProperties() const
{
  return mCustomProperties.keys();
}

QgsLayoutItemMap *QgsLayout::referenceMap() const
{
  return nullptr;
}

void QgsLayout::setReferenceMap( QgsLayoutItemMap *map )
{
  Q_UNUSED( map );
}

QgsLayoutPageCollection *QgsLayout::pageCollection()
{
  return mPageCollection.get();
}

const QgsLayoutPageCollection *QgsLayout::pageCollection() const
{
  return mPageCollection.get();
}

QRectF QgsLayout::layoutBounds( bool ignorePages, double margin ) const
{
  //start with an empty rectangle
  QRectF bounds;

  //add all QgsComposerItems and QgsPaperItems which are in the composition
  Q_FOREACH ( const QGraphicsItem *item, items() )
  {
    const QgsLayoutItem *layoutItem = dynamic_cast<const QgsLayoutItem *>( item );
    if ( !layoutItem )
      continue;

    bool isPage = layoutItem->type() == QgsLayoutItemRegistry::LayoutPage;
    if ( !isPage || !ignorePages )
    {
      //expand bounds with current item's bounds
      QRectF itemBounds;
      if ( isPage )
      {
        // for pages we only consider the item's rect - not the bounding rect
        // as the bounding rect contains extra padding
        itemBounds = layoutItem->mapToScene( layoutItem->rect() ).boundingRect();
      }
      else
        itemBounds = item->sceneBoundingRect();

      if ( bounds.isValid() )
        bounds = bounds.united( itemBounds );
      else
        bounds = itemBounds;
    }
  }

  if ( bounds.isValid() && margin > 0.0 )
  {
    //finally, expand bounds out by specified margin of page size
    double maxWidth = mPageCollection->maximumPageWidth();
    bounds.adjust( -maxWidth * margin, -maxWidth * margin, maxWidth * margin, maxWidth * margin );
  }

  return bounds;

}

void QgsLayout::addLayoutItem( QgsLayoutItem *item )
{
  addItem( item );
  updateBounds();
}

QgsLayoutUndoStack *QgsLayout::undoStack()
{
  return mUndoStack.get();
}

const QgsLayoutUndoStack *QgsLayout::undoStack() const
{
  return mUndoStack.get();
}

///@cond PRIVATE
class QgsLayoutUndoCommand: public QgsAbstractLayoutUndoCommand
{
  public:

    QgsLayoutUndoCommand( QgsLayout *layout, const QString &text, int id, QUndoCommand *parent SIP_TRANSFERTHIS = nullptr )
      : QgsAbstractLayoutUndoCommand( text, id, parent )
      , mLayout( layout )
    {}

  protected:

    void saveState( QDomDocument &stateDoc ) const override
    {
      stateDoc.clear();
      QDomElement documentElement = stateDoc.createElement( QStringLiteral( "UndoState" ) );
      mLayout->writeXmlLayoutSettings( documentElement, stateDoc, QgsReadWriteContext() );
      stateDoc.appendChild( documentElement );
    }

    void restoreState( QDomDocument &stateDoc ) override
    {
      if ( !mLayout )
      {
        return;
      }

      mLayout->readXmlLayoutSettings( stateDoc.documentElement().firstChild().toElement(), stateDoc, QgsReadWriteContext() );
      mLayout->project()->setDirty( true );
    }

  private:

    QgsLayout *mLayout = nullptr;
};
///@endcond

QgsAbstractLayoutUndoCommand *QgsLayout::createCommand( const QString &text, int id, QUndoCommand *parent )
{
  return new QgsLayoutUndoCommand( this, text, id, parent );
}

void QgsLayout::writeXmlLayoutSettings( QDomElement &element, QDomDocument &document, const QgsReadWriteContext & ) const
{
  mCustomProperties.writeXml( element, document );
  element.setAttribute( QStringLiteral( "name" ), mName );
  element.setAttribute( QStringLiteral( "units" ), QgsUnitTypes::encodeUnit( mUnits ) );
}

QDomElement QgsLayout::writeXml( QDomDocument &document, const QgsReadWriteContext &context ) const
{
  QDomElement element = document.createElement( QStringLiteral( "Layout" ) );
  auto save = [&]( const QgsLayoutSerializableObject * object )->bool
  {
    return object->writeXml( element, document, context );
  };
  save( &mSnapper );
  save( &mGridSettings );
  save( mPageCollection.get() );

  writeXmlLayoutSettings( element, document, context );
  return element;
}

bool QgsLayout::readXmlLayoutSettings( const QDomElement &layoutElement, const QDomDocument &, const QgsReadWriteContext & )
{
  mCustomProperties.readXml( layoutElement );
  setName( layoutElement.attribute( QStringLiteral( "name" ) ) );
  setUnits( QgsUnitTypes::decodeLayoutUnit( layoutElement.attribute( QStringLiteral( "units" ) ) ) );
  return true;
}

bool QgsLayout::readXml( const QDomElement &layoutElement, const QDomDocument &document, const QgsReadWriteContext &context )
{
  if ( layoutElement.nodeName() != QStringLiteral( "Layout" ) )
  {
    return false;
  }

  auto restore = [&]( QgsLayoutSerializableObject * object )->bool
  {
    return object->readXml( layoutElement, document, context );
  };

  readXmlLayoutSettings( layoutElement, document, context );

  restore( mPageCollection.get() );
  restore( &mSnapper );
  restore( &mGridSettings );

  return true;
}

void QgsLayout::updateBounds()
{
  setSceneRect( layoutBounds( false, 0.05 ) );
}
