/***************************************************************************
                              qgslayoutitempage.cpp
                             ---------------------
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

#include "qgslayoutitempage.h"
#include "qgslayout.h"
#include "qgslayoututils.h"
#include "qgspagesizeregistry.h"
#include "qgssymbollayerutils.h"
#include "qgslayoutitemundocommand.h"
#include "qgslayoutpagecollection.h"
#include "qgslayoutundostack.h"
#include "qgsstyle.h"
#include "qgsstyleentityvisitor.h"
#include "qgsfillsymbol.h"

#include <QPainter>
#include <QStyleOptionGraphicsItem>

QgsLayoutItemPage::QgsLayoutItemPage( QgsLayout *layout )
  : QgsLayoutItem( layout, false )
{
  setFlag( QGraphicsItem::ItemIsSelectable, false );
  setFlag( QGraphicsItem::ItemIsMovable, false );
  setZValue( QgsLayout::ZPage );

  connect( this, &QgsLayoutItem::sizePositionChanged, this, [ = ]
  {
    mBoundingRect = QRectF();
    prepareGeometryChange();
  } );

  const QFont font;
  const QFontMetrics fm( font );
  mMaximumShadowWidth = fm.boundingRect( QStringLiteral( "X" ) ).width();

  mGrid.reset( new QgsLayoutItemPageGrid( pos().x(), pos().y(), rect().width(), rect().height(), mLayout ) );
  mGrid->setParentItem( this );

  createDefaultPageStyleSymbol();
}

QgsLayoutItemPage::~QgsLayoutItemPage() = default;

QgsLayoutItemPage *QgsLayoutItemPage::create( QgsLayout *layout )
{
  return new QgsLayoutItemPage( layout );
}

int QgsLayoutItemPage::type() const
{
  return QgsLayoutItemRegistry::LayoutPage;
}

QString QgsLayoutItemPage::displayName() const
{
  return QObject::tr( "Page" );
}

void QgsLayoutItemPage::setPageSize( const QgsLayoutSize &size )
{
  attemptResize( size );
}

bool QgsLayoutItemPage::setPageSize( const QString &size, Orientation orientation )
{
  QgsPageSize newSize;
  if ( QgsApplication::pageSizeRegistry()->decodePageSize( size, newSize ) )
  {
    switch ( orientation )
    {
      case Portrait:
        break; // nothing to do

      case Landscape:
      {
        // flip height and width
        const double x = newSize.size.width();
        newSize.size.setWidth( newSize.size.height() );
        newSize.size.setHeight( x );
        break;
      }
    }

    setPageSize( newSize.size );
    return true;
  }
  else
  {
    return false;
  }
}

QPageLayout QgsLayoutItemPage::pageLayout() const
{
  QPageLayout pageLayout;
  pageLayout.setMargins( {0, 0, 0, 0} );
  pageLayout.setMode( QPageLayout::FullPageMode );
  const QSizeF size = layout()->renderContext().measurementConverter().convert( pageSize(), QgsUnitTypes::LayoutMillimeters ).toQSizeF();

  if ( pageSize().width() > pageSize().height() )
  {
    pageLayout.setOrientation( QPageLayout::Landscape );
    pageLayout.setPageSize( QPageSize( QSizeF( size.height(), size.width() ), QPageSize::Millimeter ) );
  }
  else
  {
    pageLayout.setOrientation( QPageLayout::Portrait );
    pageLayout.setPageSize( QPageSize( size, QPageSize::Millimeter ) );
  }
  pageLayout.setUnits( QPageLayout::Millimeter );
  return pageLayout;
}

QgsLayoutSize QgsLayoutItemPage::pageSize() const
{
  return sizeWithUnits();
}

QgsLayoutItemPage::Orientation QgsLayoutItemPage::orientation() const
{
  if ( sizeWithUnits().width() >= sizeWithUnits().height() )
    return Landscape;
  else
    return Portrait;
}

void QgsLayoutItemPage::setPageStyleSymbol( QgsFillSymbol *symbol )
{
  mPageStyleSymbol.reset( symbol );
  update();
}

QgsLayoutItemPage::Orientation QgsLayoutItemPage::decodePageOrientation( const QString &string, bool *ok )
{
  if ( ok )
    *ok = false;

  const QString trimmedString = string.trimmed();
  if ( trimmedString.compare( QLatin1String( "portrait" ), Qt::CaseInsensitive ) == 0 )
  {
    if ( ok )
      *ok = true;
    return Portrait;
  }
  else if ( trimmedString.compare( QLatin1String( "landscape" ), Qt::CaseInsensitive ) == 0 )
  {
    if ( ok )
      *ok = true;
    return Landscape;
  }
  return Landscape;
}

QRectF QgsLayoutItemPage::boundingRect() const
{
  if ( mBoundingRect.isNull() )
  {
    const double shadowWidth = mLayout->pageCollection()->pageShadowWidth();
    mBoundingRect = rect();
    mBoundingRect.adjust( 0, 0, shadowWidth, shadowWidth );
  }
  return mBoundingRect;
}

void QgsLayoutItemPage::attemptResize( const QgsLayoutSize &size, bool includesFrame )
{
  QgsLayoutItem::attemptResize( size, includesFrame );
  //update size of attached grid to reflect new page size and position
  mGrid->setRect( 0, 0, rect().width(), rect().height() );

  mLayout->guides().update();
}

void QgsLayoutItemPage::createDefaultPageStyleSymbol()
{
  QVariantMap properties;
  properties.insert( QStringLiteral( "color" ), QStringLiteral( "white" ) );
  properties.insert( QStringLiteral( "style" ), QStringLiteral( "solid" ) );
  properties.insert( QStringLiteral( "style_border" ), QStringLiteral( "no" ) );
  properties.insert( QStringLiteral( "joinstyle" ), QStringLiteral( "miter" ) );
  mPageStyleSymbol.reset( QgsFillSymbol::createSimple( properties ) );
}



///@cond PRIVATE
class QgsLayoutItemPageUndoCommand: public QgsLayoutItemUndoCommand
{
  public:

    QgsLayoutItemPageUndoCommand( QgsLayoutItemPage *page, const QString &text, int id = 0, QUndoCommand *parent SIP_TRANSFERTHIS = nullptr )
      : QgsLayoutItemUndoCommand( page, text, id, parent )
    {}

    void restoreState( QDomDocument &stateDoc ) override
    {
      QgsLayoutItemUndoCommand::restoreState( stateDoc );
      layout()->pageCollection()->reflow();
    }

  protected:

    QgsLayoutItem *recreateItem( int, QgsLayout *layout ) override
    {
      QgsLayoutItemPage *page = new QgsLayoutItemPage( layout );
      layout->pageCollection()->addPage( page );
      return page;
    }
};
///@endcond

QgsAbstractLayoutUndoCommand *QgsLayoutItemPage::createCommand( const QString &text, int id, QUndoCommand *parent )
{
  return new QgsLayoutItemPageUndoCommand( this, text, id, parent );
}

QgsLayoutItem::ExportLayerBehavior QgsLayoutItemPage::exportLayerBehavior() const
{
  return CanGroupWithItemsOfSameType;
}

bool QgsLayoutItemPage::accept( QgsStyleEntityVisitorInterface *visitor ) const
{
  QgsStyleSymbolEntity entity( mPageStyleSymbol.get() );
  if ( !visitor->visit( QgsStyleEntityVisitorInterface::StyleLeaf( &entity, QStringLiteral( "page" ), QObject::tr( "Page" ) ) ) )
    return false;
  return true;
}

void QgsLayoutItemPage::redraw()
{
  QgsLayoutItem::redraw();
  mGrid->update();
}

void QgsLayoutItemPage::draw( QgsLayoutItemRenderContext &context )
{
  if ( !context.renderContext().painter() || !mLayout || !mLayout->renderContext().pagesVisible() )
  {
    return;
  }

  const double scale = context.renderContext().convertToPainterUnits( 1, QgsUnitTypes::RenderMillimeters );

  const QgsExpressionContext expressionContext = createExpressionContext();
  context.renderContext().setExpressionContext( expressionContext );

  QPainter *painter = context.renderContext().painter();
  const QgsScopedQPainterState painterState( painter );

  if ( mLayout->renderContext().isPreviewRender() )
  {
    //if in preview mode, draw page border and shadow so that it's
    //still possible to tell where pages with a transparent style begin and end
    painter->setRenderHint( QPainter::Antialiasing, false );

    const QRectF pageRect = QRectF( 0, 0, scale * rect().width(), scale * rect().height() );

    //shadow
    painter->setBrush( QBrush( QColor( 150, 150, 150 ) ) );
    painter->setPen( Qt::NoPen );
    painter->drawRect( pageRect.translated( std::min( scale * mLayout->pageCollection()->pageShadowWidth(), mMaximumShadowWidth ),
                                            std::min( scale * mLayout->pageCollection()->pageShadowWidth(), mMaximumShadowWidth ) ) );

    //page area
    painter->setBrush( QColor( 215, 215, 215 ) );
    QPen pagePen = QPen( QColor( 100, 100, 100 ), 0 );
    pagePen.setJoinStyle( Qt::MiterJoin );
    pagePen.setCosmetic( true );
    painter->setPen( pagePen );
    painter->drawRect( pageRect );
  }

  if ( mPageStyleSymbol )
  {
    std::unique_ptr< QgsFillSymbol > symbol( mPageStyleSymbol->clone() );
    symbol->startRender( context.renderContext() );

    //get max bleed from symbol
    double maxBleedPixels = QgsSymbolLayerUtils::estimateMaxSymbolBleed( symbol.get(), context.renderContext() );

    //Now subtract 1 pixel to prevent semi-transparent borders at edge of solid page caused by
    //anti-aliased painting. This may cause a pixel to be cropped from certain edge lines/symbols,
    //but that can be counteracted by adding a dummy transparent line symbol layer with a wider line width
    if ( !mLayout->renderContext().isPreviewRender() || !qgsDoubleNear( maxBleedPixels, 0.0 ) )
    {
      maxBleedPixels = std::floor( maxBleedPixels - 2 );
    }

    // round up
    const QPolygonF pagePolygon = QPolygonF( QRectF( maxBleedPixels, maxBleedPixels,
                                  std::ceil( rect().width() * scale ) - 2 * maxBleedPixels, std::ceil( rect().height() * scale ) - 2 * maxBleedPixels ) );
    const QVector<QPolygonF> rings; //empty list

    symbol->renderPolygon( pagePolygon, &rings, nullptr, context.renderContext() );
    symbol->stopRender( context.renderContext() );
  }
}

void QgsLayoutItemPage::drawFrame( QgsRenderContext & )
{}

void QgsLayoutItemPage::drawBackground( QgsRenderContext & )
{}

bool QgsLayoutItemPage::writePropertiesToElement( QDomElement &element, QDomDocument &document, const QgsReadWriteContext &context ) const
{
  const QDomElement styleElem = QgsSymbolLayerUtils::saveSymbol( QString(), mPageStyleSymbol.get(), document, context );
  element.appendChild( styleElem );
  return true;
}

bool QgsLayoutItemPage::readPropertiesFromElement( const QDomElement &element, const QDomDocument &, const QgsReadWriteContext &context )
{
  const QDomElement symbolElem = element.firstChildElement( QStringLiteral( "symbol" ) );
  if ( !symbolElem.isNull() )
  {
    mPageStyleSymbol.reset( QgsSymbolLayerUtils::loadSymbol<QgsFillSymbol>( symbolElem, context ) );
  }
  else
  {
    createDefaultPageStyleSymbol();
  }

  return true;
}

//
// QgsLayoutItemPageGrid
//
///@cond PRIVATE

QgsLayoutItemPageGrid::QgsLayoutItemPageGrid( double x, double y, double width, double height, QgsLayout *layout )
  : QGraphicsRectItem( 0, 0, width, height )
  , mLayout( layout )
{
  // needed to access current view transform during paint operations
  setFlags( flags() | QGraphicsItem::ItemUsesExtendedStyleOption );
  setCacheMode( QGraphicsItem::DeviceCoordinateCache );
  setFlag( QGraphicsItem::ItemIsSelectable, false );
  setFlag( QGraphicsItem::ItemIsMovable, false );
  setZValue( QgsLayout::ZGrid );
  setPos( x, y );
}

void QgsLayoutItemPageGrid::paint( QPainter *painter, const QStyleOptionGraphicsItem *itemStyle, QWidget *pWidget )
{
  Q_UNUSED( pWidget )

  //draw grid
  if ( !mLayout )
    return;

  if ( !mLayout->renderContext().isPreviewRender() )
    return;

  const QgsLayoutRenderContext &context = mLayout->renderContext();
  const QgsLayoutGridSettings &grid = mLayout->gridSettings();

  if ( !context.gridVisible() || grid.resolution().length() <= 0 )
    return;

  const QPointF gridOffset = mLayout->convertToLayoutUnits( grid.offset() );
  const double gridResolution = mLayout->convertToLayoutUnits( grid.resolution() );
  const int gridMultiplyX = static_cast< int >( gridOffset.x() / gridResolution );
  const int gridMultiplyY = static_cast< int >( gridOffset.y() / gridResolution );
  double currentXCoord = gridOffset.x() - gridMultiplyX * gridResolution;
  double currentYCoord;
  const double minYCoord = gridOffset.y() - gridMultiplyY * gridResolution;

  const QgsScopedQPainterState painterState( painter );
  //turn of antialiasing so grid is nice and sharp
  painter->setRenderHint( QPainter::Antialiasing, false );

  switch ( grid.style() )
  {
    case QgsLayoutGridSettings::StyleLines:
    {
      painter->setPen( grid.pen() );

      //draw vertical lines
      for ( ; currentXCoord <= rect().width(); currentXCoord += gridResolution )
      {
        painter->drawLine( QPointF( currentXCoord, 0 ), QPointF( currentXCoord, rect().height() ) );
      }

      //draw horizontal lines
      currentYCoord = minYCoord;
      for ( ; currentYCoord <= rect().height(); currentYCoord += gridResolution )
      {
        painter->drawLine( QPointF( 0, currentYCoord ), QPointF( rect().width(), currentYCoord ) );
      }
      break;
    }

    case QgsLayoutGridSettings::StyleDots:
    case QgsLayoutGridSettings::StyleCrosses:
    {
      const QPen gridPen = grid.pen();
      painter->setPen( gridPen );
      painter->setBrush( QBrush( gridPen.color() ) );
      double halfCrossLength = 1;
      if ( grid.style() == QgsLayoutGridSettings::StyleDots )
      {
        //dots are actually drawn as tiny crosses a few pixels across
        //set halfCrossLength to equivalent of 1 pixel
        halfCrossLength = 1 / QgsLayoutUtils::scaleFactorFromItemStyle( itemStyle, painter );
      }
      else
      {
        halfCrossLength = gridResolution / 6;
      }

      for ( ; currentXCoord <= rect().width(); currentXCoord += gridResolution )
      {
        currentYCoord = minYCoord;
        for ( ; currentYCoord <= rect().height(); currentYCoord += gridResolution )
        {
          painter->drawLine( QPointF( currentXCoord - halfCrossLength, currentYCoord ), QPointF( currentXCoord + halfCrossLength, currentYCoord ) );
          painter->drawLine( QPointF( currentXCoord, currentYCoord - halfCrossLength ), QPointF( currentXCoord, currentYCoord + halfCrossLength ) );
        }
      }
      break;
    }
  }
}

///@endcond
