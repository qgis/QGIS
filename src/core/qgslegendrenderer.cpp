#include "qgslegendrenderer.h"

#include "qgscomposerlegenditem.h"
#include "qgslegendmodel.h"
#include "qgsmaplayerregistry.h"
#include "qgssymbolv2.h"
#include "qgsvectorlayer.h"

#include <QPainter>

#define FONT_WORKAROUND_SCALE 10 //scale factor for upscaling fontsize and downscaling painter


QgsLegendRenderer::QgsLegendRenderer( QgsLegendModel* legendModel, const QgsLegendSettings& settings )
    : mLegendModel( legendModel )
    , mSettings( settings )
{
}

QSizeF QgsLegendRenderer::minimumSize()
{
  return paintAndDetermineSize( 0 );
}

void QgsLegendRenderer::drawLegend( QPainter* painter )
{
  paintAndDetermineSize( painter );
}


QSizeF QgsLegendRenderer::paintAndDetermineSize( QPainter* painter )
{
  QSizeF size( 0, 0 );
  QStandardItem* rootItem = mLegendModel->invisibleRootItem();
  if ( !rootItem ) return size;

  QList<Atom> atomList = createAtomList( rootItem, mSettings.splitLayer() );

  setColumns( atomList );

  qreal maxColumnWidth = 0;
  if ( mSettings.equalColumnWidth() )
  {
    foreach ( Atom atom, atomList )
    {
      maxColumnWidth = qMax( atom.size.width(), maxColumnWidth );
    }
  }

  //calculate size of title
  QSizeF titleSize = drawTitle();
  //add title margin to size of title text
  titleSize.rwidth() += mSettings.boxSpace() * 2.0;
  double columnTop = mSettings.boxSpace() + titleSize.height() + mSettings.style( QgsComposerLegendStyle::Title ).margin( QgsComposerLegendStyle::Bottom );

  QPointF point( mSettings.boxSpace(), columnTop );
  bool firstInColumn = true;
  double columnMaxHeight = 0;
  qreal columnWidth = 0;
  int column = 0;
  foreach ( Atom atom, atomList )
  {
    if ( atom.column > column )
    {
      // Switch to next column
      if ( mSettings.equalColumnWidth() )
      {
        point.rx() += mSettings.columnSpace() + maxColumnWidth;
      }
      else
      {
        point.rx() += mSettings.columnSpace() + columnWidth;
      }
      point.ry() = columnTop;
      columnWidth = 0;
      column++;
      firstInColumn = true;
    }
    if ( !firstInColumn )
    {
      point.ry() += spaceAboveAtom( atom );
    }

    QSizeF atomSize = drawAtom( atom, painter, point );
    columnWidth = qMax( atomSize.width(), columnWidth );

    point.ry() += atom.size.height();
    columnMaxHeight = qMax( point.y() - columnTop, columnMaxHeight );

    firstInColumn = false;
  }
  point.rx() += columnWidth + mSettings.boxSpace();

  size.rheight() = columnTop + columnMaxHeight + mSettings.boxSpace();
  size.rwidth() = point.x();
  if ( !mSettings.title().isEmpty() )
  {
    size.rwidth() = qMax( titleSize.width(), size.width() );
  }

  // override the size if it was set by the user
  if ( mLegendSize.isValid() )
  {
    qreal w = qMax( size.width(), mLegendSize.width() );
    qreal h = qMax( size.height(), mLegendSize.height() );
    size = QSizeF( w, h );
  }

  // Now we have set the correct total item width and can draw the title centered
  if ( !mSettings.title().isEmpty() )
  {
    if ( mSettings.titleAlignment() == Qt::AlignLeft )
    {
      point.rx() = mSettings.boxSpace();
    }
    else if ( mSettings.titleAlignment() == Qt::AlignHCenter )
    {
      point.rx() = size.width() / 2;
    }
    else
    {
      point.rx() = size.width() - mSettings.boxSpace();
    }
    point.ry() = mSettings.boxSpace();
    drawTitle( painter, point, mSettings.titleAlignment(), size.width() );
  }

  return size;
}





QList<QgsLegendRenderer::Atom> QgsLegendRenderer::createAtomList( QStandardItem* rootItem, bool splitLayer )
{
  QList<Atom> atoms;

  if ( !rootItem ) return atoms;

  Atom atom;

  for ( int i = 0; i < rootItem->rowCount(); i++ )
  {
    QStandardItem* currentLayerItem = rootItem->child( i );
    QgsComposerLegendItem* currentLegendItem = dynamic_cast<QgsComposerLegendItem*>( currentLayerItem );
    if ( !currentLegendItem ) continue;

    QgsComposerLegendItem::ItemType type = currentLegendItem->itemType();
    if ( type == QgsComposerLegendItem::GroupItem )
    {
      // Group subitems
      QList<Atom> groupAtoms = createAtomList( currentLayerItem, splitLayer );

      Nucleon nucleon;
      nucleon.item = currentLegendItem;
      nucleon.size = drawGroupItemTitle( dynamic_cast<QgsComposerGroupItem*>( currentLegendItem ) );

      if ( groupAtoms.size() > 0 )
      {
        // Add internal space between this group title and the next nucleon
        groupAtoms[0].size.rheight() += spaceAboveAtom( groupAtoms[0] );
        // Prepend this group title to the first atom
        groupAtoms[0].nucleons.prepend( nucleon );
        groupAtoms[0].size.rheight() += nucleon.size.height();
        groupAtoms[0].size.rwidth() = qMax( nucleon.size.width(), groupAtoms[0].size.width() );
      }
      else
      {
        // no subitems, append new atom
        Atom atom;
        atom.nucleons.append( nucleon );
        atom.size.rwidth() += nucleon.size.width();
        atom.size.rheight() += nucleon.size.height();
        atom.size.rwidth() = qMax( nucleon.size.width(), atom.size.width() );
        groupAtoms.append( atom );
      }
      atoms.append( groupAtoms );
    }
    else if ( type == QgsComposerLegendItem::LayerItem )
    {
      Atom atom;

      if ( currentLegendItem->style() != QgsComposerLegendStyle::Hidden )
      {
        Nucleon nucleon;
        nucleon.item = currentLegendItem;
        nucleon.size = drawLayerItemTitle( dynamic_cast<QgsComposerLayerItem*>( currentLegendItem ) );
        atom.nucleons.append( nucleon );
        atom.size.rwidth() = nucleon.size.width();
        atom.size.rheight() = nucleon.size.height();
      }

      QList<Atom> layerAtoms;

      for ( int j = 0; j < currentLegendItem->rowCount(); j++ )
      {
        QgsComposerLegendItem * symbolItem = dynamic_cast<QgsComposerLegendItem*>( currentLegendItem->child( j, 0 ) );
        if ( !symbolItem ) continue;

        Nucleon symbolNucleon = drawSymbolItem( symbolItem );

        if ( !mSettings.splitLayer() || j == 0 )
        {
          // append to layer atom
          // the width is not correct at this moment, we must align all symbol labels
          atom.size.rwidth() = qMax( symbolNucleon.size.width(), atom.size.width() );
          // Add symbol space only if there is already title or another item above
          if ( atom.nucleons.size() > 0 )
          {
            // TODO: for now we keep Symbol and SymbolLabel Top margin in sync
            atom.size.rheight() += mSettings.style( QgsComposerLegendStyle::Symbol ).margin( QgsComposerLegendStyle::Top );
          }
          atom.size.rheight() += symbolNucleon.size.height();
          atom.nucleons.append( symbolNucleon );
        }
        else
        {
          Atom symbolAtom;
          symbolAtom.nucleons.append( symbolNucleon );
          symbolAtom.size.rwidth() = symbolNucleon.size.width();
          symbolAtom.size.rheight() = symbolNucleon.size.height();
          layerAtoms.append( symbolAtom );
        }
      }
      layerAtoms.prepend( atom );
      atoms.append( layerAtoms );
    }
  }

  return atoms;
}



void QgsLegendRenderer::setColumns( QList<Atom>& atomList )
{
  if ( mSettings.columnCount() == 0 ) return;

  // Divide atoms to columns
  double totalHeight = 0;
  // bool first = true;
  qreal maxAtomHeight = 0;
  foreach ( Atom atom, atomList )
  {
    //if ( !first )
    //{
    totalHeight += spaceAboveAtom( atom );
    //}
    totalHeight += atom.size.height();
    maxAtomHeight = qMax( atom.size.height(), maxAtomHeight );
    // first  = false;
  }

  // We know height of each atom and we have to split them into columns
  // minimizing max column height. It is sort of bin packing problem, NP-hard.
  // We are using simple heuristic, brute fore appeared to be to slow,
  // the number of combinations is N = n!/(k!*(n-k)!) where n = atomsCount-1
  // and k = columnsCount-1

  double avgColumnHeight = totalHeight / mSettings.columnCount();
  int currentColumn = 0;
  int currentColumnAtomCount = 0; // number of atoms in current column
  double currentColumnHeight = 0;
  double maxColumnHeight = 0;
  double closedColumnsHeight = 0;
  // first = true; // first in column
  for ( int i = 0; i < atomList.size(); i++ )
  {
    Atom atom = atomList[i];
    double currentHeight = currentColumnHeight;
    //if ( !first )
    //{
    currentHeight += spaceAboveAtom( atom );
    //}
    currentHeight += atom.size.height();

    // Recalc average height for remaining columns including current
    avgColumnHeight = ( totalHeight - closedColumnsHeight ) / ( mSettings.columnCount() - currentColumn );
    if (( currentHeight - avgColumnHeight ) > atom.size.height() / 2 // center of current atom is over average height
        && currentColumnAtomCount > 0 // do not leave empty column
        && currentHeight > maxAtomHeight  // no sense to make smaller columns than max atom height
        && currentHeight > maxColumnHeight  // no sense to make smaller columns than max column already created
        && currentColumn < mSettings.columnCount() - 1 ) // must not exceed max number of columns
    {
      // New column
      currentColumn++;
      currentColumnAtomCount = 0;
      closedColumnsHeight += currentColumnHeight;
      currentColumnHeight = atom.size.height();
    }
    else
    {
      currentColumnHeight = currentHeight;
    }
    atomList[i].column = currentColumn;
    currentColumnAtomCount++;
    maxColumnHeight = qMax( currentColumnHeight, maxColumnHeight );

    // first  = false;
  }

  // Alling labels of symbols for each layr/column to the same labelXOffset
  QMap<QString, qreal> maxSymbolWidth;
  for ( int i = 0; i < atomList.size(); i++ )
  {
    for ( int j = 0; j < atomList[i].nucleons.size(); j++ )
    {
      QgsComposerLegendItem* item = atomList[i].nucleons[j].item;
      if ( !item ) continue;
      QgsComposerLegendItem::ItemType type = item->itemType();
      if ( type == QgsComposerLegendItem::SymbologyV2Item ||
           type == QgsComposerLegendItem::RasterSymbolItem )
      {
        QString key = QString( "%1-%2" ).arg(( qulonglong )item->parent() ).arg( atomList[i].column );
        maxSymbolWidth[key] = qMax( atomList[i].nucleons[j].symbolSize.width(), maxSymbolWidth[key] );
      }
    }
  }
  for ( int i = 0; i < atomList.size(); i++ )
  {
    for ( int j = 0; j < atomList[i].nucleons.size(); j++ )
    {
      QgsComposerLegendItem* item = atomList[i].nucleons[j].item;
      if ( !item ) continue;
      QgsComposerLegendItem::ItemType type = item->itemType();
      if ( type == QgsComposerLegendItem::SymbologyV2Item ||
           type == QgsComposerLegendItem::RasterSymbolItem )
      {
        QString key = QString( "%1-%2" ).arg(( qulonglong )item->parent() ).arg( atomList[i].column );
        double space = mSettings.style( QgsComposerLegendStyle::Symbol ).margin( QgsComposerLegendStyle::Right ) +
                       mSettings.style( QgsComposerLegendStyle::SymbolLabel ).margin( QgsComposerLegendStyle::Left );
        atomList[i].nucleons[j].labelXOffset =  maxSymbolWidth[key] + space;
        atomList[i].nucleons[j].size.rwidth() =  maxSymbolWidth[key] + space + atomList[i].nucleons[j].labelSize.width();
      }
    }
  }
}



QSizeF QgsLegendRenderer::drawTitle( QPainter* painter, QPointF point, Qt::AlignmentFlag halignment, double legendWidth )
{
  QSizeF size( 0, 0 );
  if ( mSettings.title().isEmpty() )
  {
    return size;
  }

  QStringList lines = splitStringForWrapping( mSettings.title() );
  double y = point.y();

  if ( painter )
  {
    painter->setPen( mSettings.fontColor() );
  }

  //calculate width and left pos of rectangle to draw text into
  double textBoxWidth;
  double textBoxLeft;
  switch ( halignment )
  {
    case Qt::AlignHCenter:
      textBoxWidth = ( qMin( point.x(), legendWidth - point.x() ) - mSettings.boxSpace() ) * 2.0;
      textBoxLeft = point.x() - textBoxWidth / 2.;
      break;
    case Qt::AlignRight:
      textBoxLeft = mSettings.boxSpace();
      textBoxWidth = point.x() - mSettings.boxSpace();
      break;
    case Qt::AlignLeft:
    default:
      textBoxLeft = point.x();
      textBoxWidth = legendWidth - point.x() - mSettings.boxSpace();
      break;
  }

  QFont titleFont = mSettings.style( QgsComposerLegendStyle::Title ).font();

  for ( QStringList::Iterator titlePart = lines.begin(); titlePart != lines.end(); ++titlePart )
  {
    //last word is not drawn if rectangle width is exactly text width, so add 1
    //TODO - correctly calculate size of italicized text, since QFontMetrics does not
    qreal width = textWidthMillimeters( titleFont, *titlePart ) + 1;
    qreal height = fontAscentMillimeters( titleFont ) + fontDescentMillimeters( titleFont );

    QRectF r( textBoxLeft, y, textBoxWidth, height );

    if ( painter )
    {
      drawText( painter, r, *titlePart, titleFont, halignment, Qt::AlignVCenter, Qt::TextDontClip );
    }

    //update max width of title
    size.rwidth() = qMax( width, size.rwidth() );

    y += height;
    if ( titlePart != lines.end() )
    {
      y += mSettings.lineSpacing();
    }
  }
  size.rheight() = y - point.y();

  return size;
}


double QgsLegendRenderer::spaceAboveAtom( Atom atom )
{
  if ( atom.nucleons.size() == 0 ) return 0;

  Nucleon nucleon = atom.nucleons.first();

  QgsComposerLegendItem* item = nucleon.item;
  if ( !item ) return 0;

  QgsComposerLegendItem::ItemType type = item->itemType();
  switch ( type )
  {
    case QgsComposerLegendItem::GroupItem:
      return mSettings.style( item->style() ).margin( QgsComposerLegendStyle::Top );
      break;
    case QgsComposerLegendItem::LayerItem:
      return mSettings.style( item->style() ).margin( QgsComposerLegendStyle::Top );
      break;
    case QgsComposerLegendItem::SymbologyV2Item:
    case QgsComposerLegendItem::RasterSymbolItem:
      // TODO: use Symbol or SymbolLabel Top margin
      return mSettings.style( QgsComposerLegendStyle::Symbol ).margin( QgsComposerLegendStyle::Top );
      break;
    default:
      break;
  }
  return 0;
}


// Draw atom and expand its size (using actual nucleons labelXOffset)
QSizeF QgsLegendRenderer::drawAtom( Atom atom, QPainter* painter, QPointF point )
{
  bool first = true;
  QSizeF size = QSizeF( atom.size );
  foreach ( Nucleon nucleon, atom.nucleons )
  {
    QgsComposerLegendItem* item = nucleon.item;
    //QgsDebugMsg( "text: " + item->text() );
    if ( !item ) continue;
    QgsComposerLegendItem::ItemType type = item->itemType();
    if ( type == QgsComposerLegendItem::GroupItem )
    {
      QgsComposerGroupItem* groupItem = dynamic_cast<QgsComposerGroupItem*>( item );
      if ( !groupItem ) continue;
      if ( groupItem->style() != QgsComposerLegendStyle::Hidden )
      {
        if ( !first )
        {
          point.ry() += mSettings.style( groupItem->style() ).margin( QgsComposerLegendStyle::Top );
        }
        drawGroupItemTitle( groupItem, painter, point );
      }
    }
    else if ( type == QgsComposerLegendItem::LayerItem )
    {
      QgsComposerLayerItem* layerItem = dynamic_cast<QgsComposerLayerItem*>( item );
      if ( !layerItem ) continue;
      if ( layerItem->style() != QgsComposerLegendStyle::Hidden )
      {
        if ( !first )
        {
          point.ry() += mSettings.style( layerItem->style() ).margin( QgsComposerLegendStyle::Top );
        }
        drawLayerItemTitle( layerItem, painter, point );
      }
    }
    else if ( type == QgsComposerLegendItem::SymbologyV2Item ||
              type == QgsComposerLegendItem::RasterSymbolItem )
    {
      if ( !first )
      {
        point.ry() += mSettings.style( QgsComposerLegendStyle::Symbol ).margin( QgsComposerLegendStyle::Top );
      }
      double labelXOffset = nucleon.labelXOffset;
      Nucleon symbolNucleon = drawSymbolItem( item, painter, point, labelXOffset );
      // expand width, it may be wider because of labelXOffset
      size.rwidth() = qMax( symbolNucleon.size.width(), size.width() );
    }
    point.ry() += nucleon.size.height();
    first = false;
  }
  return size;
}


QStringList QgsLegendRenderer::splitStringForWrapping( QString stringToSplt )
{
  QStringList list;
  // If the string contains nothing then just return the string without spliting.
  if ( mSettings.wrapChar().count() == 0 )
    list << stringToSplt;
  else
    list = stringToSplt.split( mSettings.wrapChar() );
  return list;
}



void QgsLegendRenderer::drawText( QPainter* p, double x, double y, const QString& text, const QFont& font ) const
{
  QFont textFont = scaledFontPixelSize( font );

  p->save();
  p->setFont( textFont );
  double scaleFactor = 1.0 / FONT_WORKAROUND_SCALE;
  p->scale( scaleFactor, scaleFactor );
  p->drawText( QPointF( x * FONT_WORKAROUND_SCALE, y * FONT_WORKAROUND_SCALE ), text );
  p->restore();
}


void QgsLegendRenderer::drawText( QPainter* p, const QRectF& rect, const QString& text, const QFont& font, Qt::AlignmentFlag halignment, Qt::AlignmentFlag valignment, int flags ) const
{
  QFont textFont = scaledFontPixelSize( font );

  QRectF scaledRect( rect.x() * FONT_WORKAROUND_SCALE, rect.y() * FONT_WORKAROUND_SCALE,
                     rect.width() * FONT_WORKAROUND_SCALE, rect.height() * FONT_WORKAROUND_SCALE );

  p->save();
  p->setFont( textFont );
  double scaleFactor = 1.0 / FONT_WORKAROUND_SCALE;
  p->scale( scaleFactor, scaleFactor );
  p->drawText( scaledRect, halignment | valignment | flags, text );
  p->restore();
}


QFont QgsLegendRenderer::scaledFontPixelSize( const QFont& font ) const
{
  QFont scaledFont = font;
  double pixelSize = pixelFontSize( font.pointSizeF() ) * FONT_WORKAROUND_SCALE + 0.5;
  scaledFont.setPixelSize( pixelSize );
  return scaledFont;
}

double QgsLegendRenderer::pixelFontSize( double pointSize ) const
{
  return ( pointSize * 0.3527 );
}

double QgsLegendRenderer::textWidthMillimeters( const QFont& font, const QString& text ) const
{
  QFont metricsFont = scaledFontPixelSize( font );
  QFontMetricsF fontMetrics( metricsFont );
  return ( fontMetrics.width( text ) / FONT_WORKAROUND_SCALE );
}

double QgsLegendRenderer::fontHeightCharacterMM( const QFont& font, const QChar& c ) const
{
  QFont metricsFont = scaledFontPixelSize( font );
  QFontMetricsF fontMetrics( metricsFont );
  return ( fontMetrics.boundingRect( c ).height() / FONT_WORKAROUND_SCALE );
}

double QgsLegendRenderer::fontAscentMillimeters( const QFont& font ) const
{
  QFont metricsFont = scaledFontPixelSize( font );
  QFontMetricsF fontMetrics( metricsFont );
  return ( fontMetrics.ascent() / FONT_WORKAROUND_SCALE );
}

double QgsLegendRenderer::fontDescentMillimeters( const QFont& font ) const
{
  QFont metricsFont = scaledFontPixelSize( font );
  QFontMetricsF fontMetrics( metricsFont );
  return ( fontMetrics.descent() / FONT_WORKAROUND_SCALE );
}


QSizeF QgsLegendRenderer::drawGroupItemTitle( QgsComposerGroupItem* groupItem, QPainter* painter, QPointF point )
{
  QSizeF size( 0, 0 );
  if ( !groupItem ) return size;

  double y = point.y();

  if ( painter ) painter->setPen( mSettings.fontColor() );

  QFont groupFont = mSettings.style( groupItem->style() ).font();

  QStringList lines = splitStringForWrapping( groupItem->text() );
  for ( QStringList::Iterator groupPart = lines.begin(); groupPart != lines.end(); ++groupPart )
  {
    y += fontAscentMillimeters( groupFont );
    if ( painter ) drawText( painter, point.x(), y, *groupPart, groupFont );
    qreal width = textWidthMillimeters( groupFont, *groupPart );
    size.rwidth() = qMax( width, size.width() );
    if ( groupPart != lines.end() )
    {
      y += mSettings.lineSpacing();
    }
  }
  size.rheight() = y - point.y();
  return size;
}




QSizeF QgsLegendRenderer::drawLayerItemTitle( QgsComposerLayerItem* layerItem, QPainter* painter, QPointF point )
{
  QSizeF size( 0, 0 );
  if ( !layerItem ) return size;

  //Let the user omit the layer title item by having an empty layer title string
  if ( layerItem->text().isEmpty() ) return size;

  double y = point.y();

  if ( painter ) painter->setPen( mSettings.fontColor() );

  QFont layerFont = mSettings.style( layerItem->style() ).font();

  QStringList lines = splitStringForWrapping( layerItem->text() );
  for ( QStringList::Iterator layerItemPart = lines.begin(); layerItemPart != lines.end(); ++layerItemPart )
  {
    y += fontAscentMillimeters( layerFont );
    if ( painter ) drawText( painter, point.x(), y, *layerItemPart , layerFont );
    qreal width = textWidthMillimeters( layerFont, *layerItemPart );
    size.rwidth() = qMax( width, size.width() );
    if ( layerItemPart != lines.end() )
    {
      y += mSettings.lineSpacing();
    }
  }
  size.rheight() = y - point.y();

  return size;
}



QgsLegendRenderer::Nucleon QgsLegendRenderer::drawSymbolItem( QgsComposerLegendItem* symbolItem, QPainter* painter, QPointF point, double labelXOffset )
{
  QSizeF symbolSize( 0, 0 );
  QSizeF labelSize( 0, 0 );
  if ( !symbolItem ) return Nucleon();

  QFont symbolLabelFont = mSettings.style( QgsComposerLegendStyle::SymbolLabel ).font();

  double textHeight = fontHeightCharacterMM( symbolLabelFont, QChar( '0' ) );
  // itemHeight here is not realy item height, it is only for symbol
  // vertical alignment purpose, i.e. ok take single line height
  // if there are more lines, thos run under the symbol
  double itemHeight = qMax( mSettings.symbolSize().height(), textHeight );

  //real symbol height. Can be different from standard height in case of point symbols
  double realSymbolHeight;

  QgsComposerLayerItem* layerItem = dynamic_cast<QgsComposerLayerItem*>( symbolItem->parent() );

  int opacity = 255;
  if ( layerItem )
  {
    QgsMapLayer* currentLayer = QgsMapLayerRegistry::instance()->mapLayer( layerItem->layerID() );
    if ( currentLayer )
    {
      //vector layer
      QgsVectorLayer* vectorLayer = dynamic_cast<QgsVectorLayer*>( currentLayer );
      if ( vectorLayer )
      {
        opacity = 255 - ( 255 * vectorLayer->layerTransparency() / 100 );
      }
    }
  }

  QString text = symbolItem->text();

  QStringList lines = splitStringForWrapping( text );

  QgsSymbolV2* symbolNg = 0;
  QgsComposerSymbolV2Item* symbolV2Item = dynamic_cast<QgsComposerSymbolV2Item*>( symbolItem );
  if ( symbolV2Item )
  {
    symbolNg = symbolV2Item->symbolV2();
  }
  QgsComposerRasterSymbolItem* rasterItem = dynamic_cast<QgsComposerRasterSymbolItem*>( symbolItem );

  double x = point.x();
  if ( symbolNg ) //item with symbol NG?
  {
    // must be called also with painter=0 to get real size
    drawSymbolV2( painter, symbolNg, point.y() + ( itemHeight - mSettings.symbolSize().height() ) / 2, x, realSymbolHeight, opacity );
    symbolSize.rwidth() = qMax( x - point.x(), mSettings.symbolSize().width() );
    symbolSize.rheight() = qMax( realSymbolHeight, mSettings.symbolSize().height() );
  }
  else if ( rasterItem )
  {
    // manage WMS lengendGraphic
    // actual code recognise if it's a legend because it has an icon and it's text is empty => this is not good MV pattern implementation :(
    QIcon symbolIcon = symbolItem->icon();
    if ( !symbolIcon.isNull() && symbolItem->text().isEmpty() )
    {
      // find max size
      QList<QSize> sizes = symbolIcon.availableSizes();
      double maxWidth = 0;
      double maxHeight = 0;
      foreach ( QSize size, sizes )
      {
        if ( maxWidth < size.width() ) maxWidth = size.width();
        if ( maxHeight < size.height() ) maxHeight = size.height();
      }
      QSize maxSize( maxWidth, maxHeight );

      // get and print legend
      QImage legend = symbolIcon.pixmap( maxWidth, maxHeight ).toImage();
      if ( painter )
      {
        painter->drawImage( QRectF( point.x(), point.y(), mSettings.wmsLegendSize().width(), mSettings.wmsLegendSize().height() ), legend, QRectF( 0, 0, maxWidth, maxHeight ) );
      }
      symbolSize.rwidth() = mSettings.wmsLegendSize().width();
      symbolSize.rheight() = mSettings.wmsLegendSize().height();
    }
    else
    {
      if ( painter )
      {
        painter->setBrush( rasterItem->color() );
        painter->drawRect( QRectF( point.x(), point.y() + ( itemHeight - mSettings.symbolSize().height() ) / 2, mSettings.symbolSize().width(), mSettings.symbolSize().height() ) );
      }
      symbolSize.rwidth() = mSettings.symbolSize().width();
      symbolSize.rheight() = mSettings.symbolSize().height();
    }
  }
  else //item with icon?
  {
    QIcon symbolIcon = symbolItem->icon();
    if ( !symbolIcon.isNull() )
    {
      if ( painter ) symbolIcon.paint( painter, point.x(), point.y() + ( itemHeight - mSettings.symbolSize().height() ) / 2, mSettings.symbolSize().width(), mSettings.symbolSize().height() );
      symbolSize.rwidth() = mSettings.symbolSize().width();
      symbolSize.rheight() = mSettings.symbolSize().height();
    }
  }

  if ( painter ) painter->setPen( mSettings.fontColor() );

  //double labelX = point.x() + labelXOffset; // + mIconLabelSpace;
  double labelX = point.x() + qMax(( double ) symbolSize.width(), labelXOffset );

  // Vertical alignment of label with symbol:
  // a) label height < symbol height: label centerd with symbol
  // b) label height > symbol height: label starts at top and runs under symbol

  labelSize.rheight() = lines.count() * textHeight + ( lines.count() - 1 ) * mSettings.lineSpacing();

  double labelY;
  if ( labelSize.height() < symbolSize.height() )
  {
    labelY = point.y() +  symbolSize.height() / 2 + textHeight / 2;
  }
  else
  {
    labelY = point.y() + textHeight;
  }

  for ( QStringList::Iterator itemPart = lines.begin(); itemPart != lines.end(); ++itemPart )
  {
    if ( painter ) drawText( painter, labelX, labelY, *itemPart , symbolLabelFont );
    labelSize.rwidth() = qMax( textWidthMillimeters( symbolLabelFont,  *itemPart ), double( labelSize.width() ) );
    if ( itemPart != lines.end() )
    {
      labelY += mSettings.lineSpacing() + textHeight;
    }
  }

  Nucleon nucleon;
  nucleon.item = symbolItem;
  nucleon.symbolSize = symbolSize;
  nucleon.labelSize = labelSize;
  //QgsDebugMsg( QString( "symbol height = %1 label height = %2").arg( symbolSize.height()).arg( labelSize.height() ));
  double width = qMax(( double ) symbolSize.width(), labelXOffset ) + labelSize.width();
  double height = qMax( symbolSize.height(), labelSize.height() );
  nucleon.size = QSizeF( width, height );
  return nucleon;
}



void QgsLegendRenderer::drawSymbolV2( QPainter* p, QgsSymbolV2* s, double currentYCoord, double& currentXPosition, double& symbolHeight, int opacity ) const
{
  if ( !s )
  {
    return;
  }

  //setup painter scaling to dots so that raster symbology is drawn to scale
  double dotsPerMM = 1.0;
  if ( p )
  {
    QPaintDevice* paintDevice = p->device();
    if ( !paintDevice )
    {
      return;
    }
    dotsPerMM = paintDevice->logicalDpiX() / 25.4;
  }

  //consider relation to composer map for symbol sizes in mm
  bool sizeInMapUnits = s->outputUnit() == QgsSymbolV2::MapUnit;
  QgsMarkerSymbolV2* markerSymbol = dynamic_cast<QgsMarkerSymbolV2*>( s );

  //Consider symbol size for point markers
  double height = mSettings.symbolSize().height();
  double width = mSettings.symbolSize().width();
  double size = 0;
  //Center small marker symbols
  double widthOffset = 0;
  double heightOffset = 0;

  if ( markerSymbol )
  {
    size = markerSymbol->size();
    height = size;
    width = size;
    if ( sizeInMapUnits )
    {
      height *= mSettings.mmPerMapUnit();
      width *= mSettings.mmPerMapUnit();
      markerSymbol->setSize( width );
    }
    if ( width < mSettings.symbolSize().width() )
    {
      widthOffset = ( mSettings.symbolSize().width() - width ) / 2.0;
    }
    if ( height < mSettings.symbolSize().height() )
    {
      heightOffset = ( mSettings.symbolSize().height() - height ) / 2.0;
    }
  }

  if ( p )
  {
    if ( markerSymbol && sizeInMapUnits )
    {
      s->setOutputUnit( QgsSymbolV2::MM );
    }

    p->save();
    p->setRenderHint( QPainter::Antialiasing );
    if ( opacity != 255 && mSettings.useAdvancedEffects() )
    {
      //semi transparent layer, so need to draw symbol to an image (to flatten it first)
      //create image which is same size as legend rect, in case symbol bleeds outside its alloted space
      QImage tempImage = QImage( QSize( width * dotsPerMM, height * dotsPerMM ), QImage::Format_ARGB32 );
      QPainter imagePainter( &tempImage );
      tempImage.fill( Qt::transparent );
      imagePainter.translate( dotsPerMM * ( currentXPosition + widthOffset ),
                              dotsPerMM * ( currentYCoord + heightOffset ) );
      s->drawPreviewIcon( &imagePainter, QSize( width * dotsPerMM, height * dotsPerMM ) );
      //reduce opacity of image
      imagePainter.setCompositionMode( QPainter::CompositionMode_DestinationIn );
      imagePainter.fillRect( tempImage.rect(), QColor( 0, 0, 0, opacity ) );
      //draw rendered symbol image
      p->scale( 1.0 / dotsPerMM, 1.0 / dotsPerMM );
      p->drawImage( 0, 0, tempImage );
    }
    else
    {
      p->translate( currentXPosition + widthOffset, currentYCoord + heightOffset );
      p->scale( 1.0 / dotsPerMM, 1.0 / dotsPerMM );
      s->drawPreviewIcon( p, QSize( width * dotsPerMM, height * dotsPerMM ) );
    }
    p->restore();

    if ( markerSymbol && sizeInMapUnits )
    {
      s->setOutputUnit( QgsSymbolV2::MapUnit );
      markerSymbol->setSize( size );
    }
  }
  currentXPosition += width;
  currentXPosition += 2 * widthOffset;
  symbolHeight = height + 2 * heightOffset;
}

