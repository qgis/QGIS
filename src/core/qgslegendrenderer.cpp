/***************************************************************************
  qgslegendrenderer.cpp
  --------------------------------------
  Date                 : July 2014
  Copyright            : (C) 2014 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslegendrenderer.h"

#include "qgslayertree.h"
#include "qgslayertreemodel.h"
#include "qgslayertreemodellegendnode.h"
#include "qgslegendstyle.h"
#include "qgsmaplayerlegend.h"
#include "qgssymbol.h"
#include "qgsrendercontext.h"
#include "qgsvectorlayer.h"
#include "qgsexpressioncontextutils.h"

#include <QJsonObject>
#include <QPainter>



QgsLegendRenderer::QgsLegendRenderer( QgsLayerTreeModel *legendModel, const QgsLegendSettings &settings )
  : mLegendModel( legendModel )
  , mSettings( settings )
{
}

QSizeF QgsLegendRenderer::minimumSize( QgsRenderContext *renderContext )
{
  return paintAndDetermineSize( renderContext );
}

void QgsLegendRenderer::drawLegend( QPainter *painter )
{
  paintAndDetermineSize( painter );
}

void QgsLegendRenderer::exportLegendToJson( const QgsRenderContext &context, QJsonObject &json )
{
  QgsLayerTreeGroup *rootGroup = mLegendModel->rootGroup();
  if ( !rootGroup )
    return;

  json["title"] = mSettings.title();
  exportLegendToJson( context, rootGroup, json );
}

void QgsLegendRenderer::exportLegendToJson( const QgsRenderContext &context, QgsLayerTreeGroup *nodeGroup, QJsonObject &json )
{
  QJsonArray nodes;
  for ( auto node : nodeGroup->children() )
  {
    if ( QgsLayerTree::isGroup( node ) )
    {
      QgsLayerTreeGroup *nodeGroup = QgsLayerTree::toGroup( node );
      const QModelIndex idx = mLegendModel->node2index( nodeGroup );
      const QString text = mLegendModel->data( idx, Qt::DisplayRole ).toString();

      QJsonObject group;
      group[ "type" ] = "group";
      group[ "title" ] = text;
      exportLegendToJson( context, nodeGroup, group );
      nodes.append( group );
    }
    else if ( QgsLayerTree::isLayer( node ) )
    {
      QJsonObject group;
      group[ "type" ] = "layer";

      QgsLayerTreeLayer *nodeLayer = QgsLayerTree::toLayer( node );

      QString text;
      if ( nodeLegendStyle( nodeLayer ) != QgsLegendStyle::Hidden )
      {
        const QModelIndex idx = mLegendModel->node2index( nodeLayer );
        text = mLegendModel->data( idx, Qt::DisplayRole ).toString();
      }

      QList<QgsLayerTreeModelLegendNode *> legendNodes = mLegendModel->layerLegendNodes( nodeLayer );

      if ( legendNodes.isEmpty() && mLegendModel->legendFilterMapSettings() )
        continue;

      if ( legendNodes.count() == 1 )
      {
        legendNodes.at( 0 )->exportToJson( mSettings, context, group );
        nodes.append( group );
      }
      else if ( legendNodes.count() > 1 )
      {
        QJsonArray symbols;
        for ( int j = 0; j < legendNodes.count(); j++ )
        {
          QgsLayerTreeModelLegendNode *legendNode = legendNodes.at( j );
          QJsonObject symbol;
          legendNode->exportToJson( mSettings, context, symbol );
          symbols.append( symbol );
        }
        group[ "title" ] = text;
        group[ "symbols" ] = symbols;
        nodes.append( group );
      }
    }
  }

  json["nodes"] = nodes;
}

QSizeF QgsLegendRenderer::paintAndDetermineSize( QPainter *painter )
{
  return paintAndDetermineSizeInternal( nullptr, painter );
}

QSizeF QgsLegendRenderer::paintAndDetermineSizeInternal( QgsRenderContext *context, QPainter *painter )
{
  QSizeF size( 0, 0 );
  QgsLayerTreeGroup *rootGroup = mLegendModel->rootGroup();
  if ( !rootGroup )
    return size;

  QList<LegendComponentGroup> atomList = createComponentGroupList( rootGroup, mSettings.splitLayer() );

  setColumns( atomList );

  // another iteration -- this one is required to calculate the maximum item width for each
  // column. Unfortunately, we can't trust the atom widths at this stage, as they are minimal widths
  // only. When actually rendering a symbol node, the text is aligned according to the WIDEST
  // symbol in a column. So that means we can't possibly determine the exact size of legend components
  // until now. BUUUUUUUUUUUUT. Because everything sucks, we can't even start the actual render of items
  // at the same time we calculate this -- legend items REQUIRE the REAL width of the columns in order to
  // correctly align right or center-aligned symbols/text. Bah -- A triple iteration it is!
  QMap< int, double > maxColumnWidths;
  qreal maxEqualColumnWidth = 0;
  // temporarily remove painter from context -- we don't need to actually draw anything yet. But we DO need
  // to send the full render context so that an expression context is available during the size calculation
  QPainter *prevPainter = context ? context->painter() : nullptr;
  if ( context )
    context->setPainter( nullptr );
  for ( const LegendComponentGroup &atom : qgis::as_const( atomList ) )
  {
    const QSizeF actualSize = drawGroup( atom, context, ColumnContext() );
    if ( mSettings.equalColumnWidth() )
    {
      maxEqualColumnWidth = std::max( actualSize.width(), maxEqualColumnWidth );
    }
    else
    {
      maxColumnWidths[ atom.column ] = std::max( actualSize.width(), maxColumnWidths.value( atom.column, 0 ) );
    }
  }
  if ( context )
    context->setPainter( prevPainter );

  //calculate size of title
  QSizeF titleSize = drawTitle();
  //add title margin to size of title text
  titleSize.rwidth() += mSettings.boxSpace() * 2.0;
  double columnTop = mSettings.boxSpace() + titleSize.height() + mSettings.style( QgsLegendStyle::Title ).margin( QgsLegendStyle::Bottom );

  bool firstInColumn = true;
  double columnMaxHeight = 0;
  qreal columnWidth = 0;
  int column = -1;
  ColumnContext columnContext;
  columnContext.left = mSettings.boxSpace();
  columnContext.right = mLegendSize.width() - mSettings.boxSpace();
  double currentY = columnTop;

  for ( const LegendComponentGroup &atom : qgis::as_const( atomList ) )
  {
    if ( atom.column > column )
    {
      // Switch to next column
      columnContext.left = atom.column > 0 ? columnContext.right + mSettings.columnSpace() : mSettings.boxSpace();
      columnWidth = mSettings.equalColumnWidth() ? maxEqualColumnWidth : maxColumnWidths.value( atom.column );
      columnContext.right = columnContext.left + columnWidth;
      currentY = columnTop;
      column++;
      firstInColumn = true;
    }
    if ( !firstInColumn )
    {
      currentY += spaceAboveGroup( atom );
    }

    if ( context )
      drawGroup( atom, context, columnContext, currentY );
    else if ( painter )
      drawGroup( atom, columnContext, painter, currentY );

    currentY += atom.size.height();
    columnMaxHeight = std::max( currentY - columnTop, columnMaxHeight );

    firstInColumn = false;
  }
  const double totalWidth = columnContext.right + mSettings.boxSpace();

  size.rheight() = columnTop + columnMaxHeight + mSettings.boxSpace();
  size.rwidth() = totalWidth;
  if ( !mSettings.title().isEmpty() )
  {
    size.rwidth() = std::max( titleSize.width(), size.width() );
  }

  // override the size if it was set by the user
  if ( mLegendSize.isValid() )
  {
    qreal w = std::max( size.width(), mLegendSize.width() );
    qreal h = std::max( size.height(), mLegendSize.height() );
    size = QSizeF( w, h );
  }

  // Now we have set the correct total item width and can draw the title centered
  if ( !mSettings.title().isEmpty() )
  {
    if ( context )
      drawTitle( context, mSettings.boxSpace(), mSettings.titleAlignment(), size.width() );
    else
      drawTitle( painter, mSettings.boxSpace(), mSettings.titleAlignment(), size.width() );
  }

  return size;
}

void QgsLegendRenderer::widthAndOffsetForTitleText( const Qt::AlignmentFlag halignment, const double legendWidth, double &textBoxWidth, double &textBoxLeft )
{
  switch ( halignment )
  {
    default:
      textBoxLeft = mSettings.boxSpace();
      textBoxWidth = legendWidth - 2 * mSettings.boxSpace();
      break;

    case Qt::AlignHCenter:
    {
      // not sure on this logic, I just moved it -- don't blame me for it being totally obscure!
      const double centerX = legendWidth / 2;
      textBoxWidth = ( std::min( static_cast< double >( centerX ), legendWidth - centerX ) - mSettings.boxSpace() ) * 2.0;
      textBoxLeft = centerX - textBoxWidth / 2.;
      break;
    }
  }
}

QList<QgsLegendRenderer::LegendComponentGroup> QgsLegendRenderer::createComponentGroupList( QgsLayerTreeGroup *parentGroup, bool splitLayer )
{
  QList<LegendComponentGroup> atoms;

  if ( !parentGroup ) return atoms;

  const auto constChildren = parentGroup->children();
  for ( QgsLayerTreeNode *node : constChildren )
  {
    if ( QgsLayerTree::isGroup( node ) )
    {
      QgsLayerTreeGroup *nodeGroup = QgsLayerTree::toGroup( node );

      // Group subitems
      QList<LegendComponentGroup> groupAtoms = createComponentGroupList( nodeGroup, splitLayer );
      bool hasSubItems = !groupAtoms.empty();

      if ( nodeLegendStyle( nodeGroup ) != QgsLegendStyle::Hidden )
      {
        LegendComponent nucleon;
        nucleon.item = node;
        nucleon.size = drawGroupTitle( nodeGroup );

        if ( !groupAtoms.isEmpty() )
        {
          // Add internal space between this group title and the next nucleon
          groupAtoms[0].size.rheight() += spaceAboveGroup( groupAtoms[0] );
          // Prepend this group title to the first atom
          groupAtoms[0].components.prepend( nucleon );
          groupAtoms[0].size.rheight() += nucleon.size.height();
          groupAtoms[0].size.rwidth() = std::max( nucleon.size.width(), groupAtoms[0].size.width() );
        }
        else
        {
          // no subitems, append new atom
          LegendComponentGroup atom;
          atom.components.append( nucleon );
          atom.size.rwidth() += nucleon.size.width();
          atom.size.rheight() += nucleon.size.height();
          atom.size.rwidth() = std::max( nucleon.size.width(), atom.size.width() );
          groupAtoms.append( atom );
        }
      }

      if ( hasSubItems ) //leave away groups without content
      {
        atoms.append( groupAtoms );
      }

    }
    else if ( QgsLayerTree::isLayer( node ) )
    {
      QgsLayerTreeLayer *nodeLayer = QgsLayerTree::toLayer( node );

      LegendComponentGroup atom;

      if ( nodeLegendStyle( nodeLayer ) != QgsLegendStyle::Hidden )
      {
        LegendComponent nucleon;
        nucleon.item = node;
        nucleon.size = drawLayerTitle( nodeLayer );
        atom.components.append( nucleon );
        atom.size.rwidth() = nucleon.size.width();
        atom.size.rheight() = nucleon.size.height();
      }

      QList<QgsLayerTreeModelLegendNode *> legendNodes = mLegendModel->layerLegendNodes( nodeLayer );

      // workaround for the issue that "filtering by map" does not remove layer nodes that have no symbols present
      // on the map. We explicitly skip such layers here. In future ideally that should be handled directly
      // in the layer tree model
      if ( legendNodes.isEmpty() && mLegendModel->legendFilterMapSettings() )
        continue;

      QList<LegendComponentGroup> layerAtoms;

      for ( int j = 0; j < legendNodes.count(); j++ )
      {
        QgsLayerTreeModelLegendNode *legendNode = legendNodes.at( j );

        LegendComponent symbolNucleon = drawSymbolItem( legendNode );

        if ( !mSettings.splitLayer() || j == 0 )
        {
          // append to layer atom
          // the width is not correct at this moment, we must align all symbol labels
          atom.size.rwidth() = std::max( symbolNucleon.size.width(), atom.size.width() );
          // Add symbol space only if there is already title or another item above
          if ( !atom.components.isEmpty() )
          {
            // TODO: for now we keep Symbol and SymbolLabel Top margin in sync
            atom.size.rheight() += mSettings.style( QgsLegendStyle::Symbol ).margin( QgsLegendStyle::Top );
          }
          atom.size.rheight() += symbolNucleon.size.height();
          atom.components.append( symbolNucleon );
        }
        else
        {
          LegendComponentGroup symbolAtom;
          symbolAtom.components.append( symbolNucleon );
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


void QgsLegendRenderer::setColumns( QList<LegendComponentGroup> &atomList )
{
  if ( mSettings.columnCount() == 0 ) return;

  // Divide atoms to columns
  double totalHeight = 0;
  qreal maxAtomHeight = 0;
  const auto constAtomList = atomList;
  for ( const LegendComponentGroup &atom : constAtomList )
  {
    totalHeight += spaceAboveGroup( atom );
    totalHeight += atom.size.height();
    maxAtomHeight = std::max( atom.size.height(), maxAtomHeight );
  }

  // We know height of each atom and we have to split them into columns
  // minimizing max column height. It is sort of bin packing problem, NP-hard.
  // We are using simple heuristic, brute fore appeared to be to slow,
  // the number of combinations is N = n!/(k!*(n-k)!) where n = atomsCount-1
  // and k = columnsCount-1
  double maxColumnHeight = 0;
  int currentColumn = 0;
  int currentColumnAtomCount = 0; // number of atoms in current column
  double currentColumnHeight = 0;
  double closedColumnsHeight = 0;

  for ( int i = 0; i < atomList.size(); i++ )
  {
    // Recalc average height for remaining columns including current
    double avgColumnHeight = ( totalHeight - closedColumnsHeight ) / ( mSettings.columnCount() - currentColumn );

    LegendComponentGroup atom = atomList.at( i );
    double currentHeight = currentColumnHeight;
    if ( currentColumnAtomCount > 0 )
      currentHeight += spaceAboveGroup( atom );
    currentHeight += atom.size.height();

    bool canCreateNewColumn = ( currentColumnAtomCount > 0 )  // do not leave empty column
                              && ( currentColumn < mSettings.columnCount() - 1 ); // must not exceed max number of columns

    bool shouldCreateNewColumn = ( currentHeight - avgColumnHeight ) > atom.size.height() / 2  // center of current atom is over average height
                                 && currentColumnAtomCount > 0 // do not leave empty column
                                 && currentHeight > maxAtomHeight  // no sense to make smaller columns than max atom height
                                 && currentHeight > maxColumnHeight; // no sense to make smaller columns than max column already created

    // also should create a new column if the number of items left < number of columns left
    // in this case we should spread the remaining items out over the remaining columns
    shouldCreateNewColumn |= ( atomList.size() - i < mSettings.columnCount() - currentColumn );

    if ( canCreateNewColumn && shouldCreateNewColumn )
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
    maxColumnHeight = std::max( currentColumnHeight, maxColumnHeight );
  }

  // Align labels of symbols for each layr/column to the same labelXOffset
  QMap<QString, qreal> maxSymbolWidth;
  for ( int i = 0; i < atomList.size(); i++ )
  {
    LegendComponentGroup &atom = atomList[i];
    for ( int j = 0; j < atom.components.size(); j++ )
    {
      if ( QgsLayerTreeModelLegendNode *legendNode = qobject_cast<QgsLayerTreeModelLegendNode *>( atom.components.at( j ).item ) )
      {
        QString key = QStringLiteral( "%1-%2" ).arg( reinterpret_cast< qulonglong >( legendNode->layerNode() ) ).arg( atom.column );
        maxSymbolWidth[key] = std::max( atom.components.at( j ).symbolSize.width(), maxSymbolWidth[key] );
      }
    }
  }
  for ( int i = 0; i < atomList.size(); i++ )
  {
    LegendComponentGroup &atom = atomList[i];
    for ( int j = 0; j < atom.components.size(); j++ )
    {
      if ( QgsLayerTreeModelLegendNode *legendNode = qobject_cast<QgsLayerTreeModelLegendNode *>( atom.components.at( j ).item ) )
      {
        QString key = QStringLiteral( "%1-%2" ).arg( reinterpret_cast< qulonglong >( legendNode->layerNode() ) ).arg( atom.column );
        double space = mSettings.style( QgsLegendStyle::Symbol ).margin( QgsLegendStyle::Right ) +
                       mSettings.style( QgsLegendStyle::SymbolLabel ).margin( QgsLegendStyle::Left );
        atom.components[j].labelXOffset = maxSymbolWidth[key] + space;
        atom.components[j].maxSiblingSymbolWidth = maxSymbolWidth[key];
        atom.components[j].size.rwidth() = maxSymbolWidth[key] + space + atom.components.at( j ).labelSize.width();
      }
    }
  }
}

QSizeF QgsLegendRenderer::drawTitle( QPainter *painter, double top, Qt::AlignmentFlag halignment, double legendWidth )
{
  return drawTitleInternal( nullptr, painter, top, halignment, legendWidth );
}

QSizeF QgsLegendRenderer::drawTitleInternal( QgsRenderContext *context, QPainter *painter, const double top, Qt::AlignmentFlag halignment, double legendWidth )
{
  QSizeF size( 0, 0 );
  if ( mSettings.title().isEmpty() )
  {
    return size;
  }

  QStringList lines = mSettings.splitStringForWrapping( mSettings.title() );
  double y = top;

  if ( context && context->painter() )
  {
    context->painter()->setPen( mSettings.fontColor() );
  }
  else if ( painter )
  {
    painter->setPen( mSettings.fontColor() );
  }

  //calculate width and left pos of rectangle to draw text into
  double textBoxWidth;
  double textBoxLeft;
  widthAndOffsetForTitleText( halignment, legendWidth, textBoxWidth, textBoxLeft );

  QFont titleFont = mSettings.style( QgsLegendStyle::Title ).font();

  for ( QStringList::Iterator titlePart = lines.begin(); titlePart != lines.end(); ++titlePart )
  {
    //last word is not drawn if rectangle width is exactly text width, so add 1
    //TODO - correctly calculate size of italicized text, since QFontMetrics does not
    qreal width = mSettings.textWidthMillimeters( titleFont, *titlePart ) + 1;
    qreal height = mSettings.fontAscentMillimeters( titleFont ) + mSettings.fontDescentMillimeters( titleFont );

    QRectF r( textBoxLeft, y, textBoxWidth, height );

    if ( context && context->painter() )
    {
      mSettings.drawText( context->painter(), r, *titlePart, titleFont, halignment, Qt::AlignVCenter, Qt::TextDontClip );
    }
    else if ( painter )
    {
      mSettings.drawText( painter, r, *titlePart, titleFont, halignment, Qt::AlignVCenter, Qt::TextDontClip );
    }

    //update max width of title
    size.rwidth() = std::max( width, size.rwidth() );

    y += height;
    if ( titlePart != ( lines.end() - 1 ) )
    {
      y += mSettings.lineSpacing();
    }
  }
  size.rheight() = y - top;

  return size;
}


double QgsLegendRenderer::spaceAboveGroup( const LegendComponentGroup &atom )
{
  if ( atom.components.isEmpty() ) return 0;

  LegendComponent nucleon = atom.components.first();

  if ( QgsLayerTreeGroup *nodeGroup = qobject_cast<QgsLayerTreeGroup *>( nucleon.item ) )
  {
    return mSettings.style( nodeLegendStyle( nodeGroup ) ).margin( QgsLegendStyle::Top );
  }
  else if ( QgsLayerTreeLayer *nodeLayer = qobject_cast<QgsLayerTreeLayer *>( nucleon.item ) )
  {
    return mSettings.style( nodeLegendStyle( nodeLayer ) ).margin( QgsLegendStyle::Top );
  }
  else if ( qobject_cast<QgsLayerTreeModelLegendNode *>( nucleon.item ) )
  {
    // TODO: use Symbol or SymbolLabel Top margin
    return mSettings.style( QgsLegendStyle::Symbol ).margin( QgsLegendStyle::Top );
  }

  return 0;
}


// Draw atom and expand its size (using actual nucleons labelXOffset)
QSizeF QgsLegendRenderer::drawGroup( const LegendComponentGroup &atom, const ColumnContext &columnContext, QPainter *painter, double top )
{
  return drawGroupInternal( atom, nullptr, columnContext, painter, top );
}

QSizeF QgsLegendRenderer::drawGroupInternal( const LegendComponentGroup &atom, QgsRenderContext *context, const ColumnContext &columnContext, QPainter *painter, const double top )
{
  bool first = true;
  QSizeF size = QSizeF( atom.size );
  double currentY = top;
  for ( const LegendComponent &nucleon : qgis::as_const( atom.components ) )
  {
    if ( QgsLayerTreeGroup *groupItem = qobject_cast<QgsLayerTreeGroup *>( nucleon.item ) )
    {
      QgsLegendStyle::Style s = nodeLegendStyle( groupItem );
      if ( s != QgsLegendStyle::Hidden )
      {
        if ( !first )
        {
          currentY += mSettings.style( s ).margin( QgsLegendStyle::Top );
        }
        if ( context )
          drawGroupTitle( groupItem, context, columnContext, currentY );
        else
          drawGroupTitle( groupItem, columnContext, painter, currentY );
      }
    }
    else if ( QgsLayerTreeLayer *layerItem = qobject_cast<QgsLayerTreeLayer *>( nucleon.item ) )
    {
      QgsLegendStyle::Style s = nodeLegendStyle( layerItem );
      if ( s != QgsLegendStyle::Hidden )
      {
        if ( !first )
        {
          currentY += mSettings.style( s ).margin( QgsLegendStyle::Top );
        }
        if ( context )
          drawLayerTitle( layerItem, context, columnContext, currentY );
        else
          drawLayerTitle( layerItem, columnContext, painter, currentY );
      }
    }
    else if ( QgsLayerTreeModelLegendNode *legendNode = qobject_cast<QgsLayerTreeModelLegendNode *>( nucleon.item ) )
    {
      if ( !first )
      {
        currentY += mSettings.style( QgsLegendStyle::Symbol ).margin( QgsLegendStyle::Top );
      }

      LegendComponent symbolNucleon = context ? drawSymbolItem( legendNode, context, columnContext, currentY, nucleon.maxSiblingSymbolWidth )
                                      : drawSymbolItem( legendNode, columnContext, painter, currentY, nucleon.maxSiblingSymbolWidth );
      // expand width, it may be wider because of labelXOffset
      size.rwidth() = std::max( symbolNucleon.size.width(), size.width() );
    }
    currentY += nucleon.size.height();
    first = false;
  }
  return size;
}

QgsLegendRenderer::LegendComponent QgsLegendRenderer::drawSymbolItem( QgsLayerTreeModelLegendNode *symbolItem, const ColumnContext &columnContext, QPainter *painter, double top, double maxSiblingSymbolWidth )
{
  return drawSymbolItemInternal( symbolItem, columnContext, nullptr, painter, top, maxSiblingSymbolWidth );
}

QgsLegendRenderer::LegendComponent QgsLegendRenderer::drawSymbolItemInternal( QgsLayerTreeModelLegendNode *symbolItem, const ColumnContext &columnContext, QgsRenderContext *context, QPainter *painter, double top, double maxSiblingSymbolWidth )
{
  QgsLayerTreeModelLegendNode::ItemContext ctx;
  ctx.context = context;

  // add a layer expression context scope
  QgsExpressionContextScope *layerScope = nullptr;
  if ( context && symbolItem->layerNode()->layer() )
  {
    layerScope = QgsExpressionContextUtils::layerScope( symbolItem->layerNode()->layer() );
    context->expressionContext().appendScope( layerScope );
  }

  ctx.painter = context ? context->painter() : painter;
  Q_NOWARN_DEPRECATED_PUSH
  ctx.point = QPointF( columnContext.left, top );
  ctx.labelXOffset = maxSiblingSymbolWidth;
  Q_NOWARN_DEPRECATED_POP

  ctx.top = top;
  ctx.columnLeft = columnContext.left;
  ctx.columnRight = columnContext.right;
  ctx.maxSiblingSymbolWidth = maxSiblingSymbolWidth;

  QgsLayerTreeModelLegendNode::ItemMetrics im = symbolItem->draw( mSettings, context ? &ctx
      : ( painter ? &ctx : nullptr ) );

  if ( layerScope )
    delete context->expressionContext().popScope();

  LegendComponent nucleon;
  nucleon.item = symbolItem;
  nucleon.symbolSize = im.symbolSize;
  nucleon.labelSize = im.labelSize;
  //QgsDebugMsg( QStringLiteral( "symbol height = %1 label height = %2").arg( symbolSize.height()).arg( labelSize.height() ));
  // NOTE -- we hard code left/right margins below, because those are the only ones exposed for use currently.
  // ideally we could (should?) expose all these margins as settings, and then adapt the below to respect the current symbol/text alignment
  // and consider the correct margin sides...
  double width = std::max( static_cast< double >( im.symbolSize.width() ), maxSiblingSymbolWidth )
                 + mSettings.style( QgsLegendStyle::Symbol ).margin( QgsLegendStyle::Right )
                 + mSettings.style( QgsLegendStyle::SymbolLabel ).margin( QgsLegendStyle::Left )
                 + im.labelSize.width();

  double height = std::max( im.symbolSize.height(), im.labelSize.height() );
  nucleon.size = QSizeF( width, height );
  return nucleon;
}

QSizeF QgsLegendRenderer::drawLayerTitle( QgsLayerTreeLayer *nodeLayer, const ColumnContext &columnContext, QPainter *painter, double top )
{
  return drawLayerTitleInternal( nodeLayer, columnContext, nullptr, painter, top );
}

QSizeF QgsLegendRenderer::drawLayerTitleInternal( QgsLayerTreeLayer *nodeLayer, const ColumnContext &columnContext, QgsRenderContext *context, QPainter *painter, const double top )
{
  QSizeF size( 0, 0 );
  QModelIndex idx = mLegendModel->node2index( nodeLayer );

  //Let the user omit the layer title item by having an empty layer title string
  if ( mLegendModel->data( idx, Qt::DisplayRole ).toString().isEmpty() )
    return size;

  double y = top;

  if ( context && context->painter() )
    context->painter()->setPen( mSettings.layerFontColor() );
  else if ( painter )
    painter->setPen( mSettings.layerFontColor() );

  QFont layerFont = mSettings.style( nodeLegendStyle( nodeLayer ) ).font();

  QgsExpressionContextScope *layerScope = nullptr;
  if ( context && nodeLayer->layer() )
  {
    layerScope = QgsExpressionContextUtils::layerScope( nodeLayer->layer() );
    context->expressionContext().appendScope( layerScope );
  }

  QgsExpressionContext tempContext;

  const QStringList lines = mSettings.evaluateItemText( mLegendModel->data( idx, Qt::DisplayRole ).toString(),
                            context ? context->expressionContext() : tempContext );
  int i = 0;
  for ( QStringList::ConstIterator layerItemPart = lines.constBegin(); layerItemPart != lines.constEnd(); ++layerItemPart )
  {
    y += mSettings.fontAscentMillimeters( layerFont );
    if ( QPainter *destPainter = context && context->painter() ? context->painter() : painter )
    {
      double x = columnContext.left;
      if ( mSettings.style( nodeLegendStyle( nodeLayer ) ).alignment() != Qt::AlignLeft )
      {
        const double labelWidth = mSettings.textWidthMillimeters( layerFont, *layerItemPart );
        if ( mSettings.style( nodeLegendStyle( nodeLayer ) ).alignment() == Qt::AlignRight )
          x = columnContext.right - labelWidth;
        else if ( mSettings.style( nodeLegendStyle( nodeLayer ) ).alignment() == Qt::AlignHCenter )
          x = columnContext.left + ( columnContext.right - columnContext.left - labelWidth ) / 2;
      }
      mSettings.drawText( destPainter, x, y, *layerItemPart, layerFont );
    }
    qreal width = mSettings.textWidthMillimeters( layerFont, *layerItemPart );
    size.rwidth() = std::max( width, size.width() );
    if ( layerItemPart != ( lines.end() - 1 ) )
    {
      y += mSettings.lineSpacing();
    }
    i++;
  }
  size.rheight() = y - top;
  size.rheight() += mSettings.style( nodeLegendStyle( nodeLayer ) ).margin( QgsLegendStyle::Side::Bottom );

  if ( layerScope )
    delete context->expressionContext().popScope();

  return size;
}

QSizeF QgsLegendRenderer::drawGroupTitle( QgsLayerTreeGroup *nodeGroup, const ColumnContext &columnContext, QPainter *painter, double top )
{
  return drawGroupTitleInternal( nodeGroup, columnContext, nullptr, painter, top );
}

QSizeF QgsLegendRenderer::drawGroupTitleInternal( QgsLayerTreeGroup *nodeGroup, const ColumnContext &columnContext, QgsRenderContext *context, QPainter *painter, const double top )
{
  QSizeF size( 0, 0 );
  QModelIndex idx = mLegendModel->node2index( nodeGroup );

  double y = top;

  if ( context && context->painter() )
    context->painter()->setPen( mSettings.fontColor() );
  else if ( painter )
    painter->setPen( mSettings.fontColor() );

  QFont groupFont = mSettings.style( nodeLegendStyle( nodeGroup ) ).font();

  QgsExpressionContext tempContext;

  const QStringList lines = mSettings.evaluateItemText( mLegendModel->data( idx, Qt::DisplayRole ).toString(),
                            context ? context->expressionContext() : tempContext );
  for ( QStringList::ConstIterator groupPart = lines.constBegin(); groupPart != lines.constEnd(); ++groupPart )
  {
    y += mSettings.fontAscentMillimeters( groupFont );

    if ( QPainter *destPainter = context && context->painter() ? context->painter() : painter )
    {
      double x = columnContext.left;
      if ( mSettings.style( nodeLegendStyle( nodeGroup ) ).alignment() != Qt::AlignLeft )
      {
        const double labelWidth = mSettings.textWidthMillimeters( groupFont, *groupPart );
        if ( mSettings.style( nodeLegendStyle( nodeGroup ) ).alignment() == Qt::AlignRight )
          x = columnContext.right - labelWidth;
        else if ( mSettings.style( nodeLegendStyle( nodeGroup ) ).alignment() == Qt::AlignHCenter )
          x = columnContext.left + ( columnContext.right - columnContext.left - labelWidth ) / 2;
      }
      mSettings.drawText( destPainter, x, y, *groupPart, groupFont );
    }
    qreal width = mSettings.textWidthMillimeters( groupFont, *groupPart );
    size.rwidth() = std::max( width, size.width() );
    if ( groupPart != ( lines.end() - 1 ) )
    {
      y += mSettings.lineSpacing();
    }
  }
  size.rheight() = y - top + mSettings.style( nodeLegendStyle( nodeGroup ) ).margin( QgsLegendStyle::Bottom );
  return size;
}

QgsLegendStyle::Style QgsLegendRenderer::nodeLegendStyle( QgsLayerTreeNode *node, QgsLayerTreeModel *model )
{
  QString style = node->customProperty( QStringLiteral( "legend/title-style" ) ).toString();
  if ( style == QLatin1String( "hidden" ) )
    return QgsLegendStyle::Hidden;
  else if ( style == QLatin1String( "group" ) )
    return QgsLegendStyle::Group;
  else if ( style == QLatin1String( "subgroup" ) )
    return QgsLegendStyle::Subgroup;

  // use a default otherwise
  if ( QgsLayerTree::isGroup( node ) )
    return QgsLegendStyle::Group;
  else if ( QgsLayerTree::isLayer( node ) )
  {
    if ( model->legendNodeEmbeddedInParent( QgsLayerTree::toLayer( node ) ) )
      return QgsLegendStyle::Hidden;
    return QgsLegendStyle::Subgroup;
  }

  return QgsLegendStyle::Undefined; // should not happen, only if corrupted project file
}

QgsLegendStyle::Style QgsLegendRenderer::nodeLegendStyle( QgsLayerTreeNode *node )
{
  return nodeLegendStyle( node, mLegendModel );
}

void QgsLegendRenderer::setNodeLegendStyle( QgsLayerTreeNode *node, QgsLegendStyle::Style style )
{
  QString str;
  switch ( style )
  {
    case QgsLegendStyle::Hidden:
      str = QStringLiteral( "hidden" );
      break;
    case QgsLegendStyle::Group:
      str = QStringLiteral( "group" );
      break;
    case QgsLegendStyle::Subgroup:
      str = QStringLiteral( "subgroup" );
      break;
    default:
      break; // nothing
  }

  if ( !str.isEmpty() )
    node->setCustomProperty( QStringLiteral( "legend/title-style" ), str );
  else
    node->removeCustomProperty( QStringLiteral( "legend/title-style" ) );
}

QSizeF QgsLegendRenderer::drawTitle( QgsRenderContext *rendercontext, double top, Qt::AlignmentFlag halignment, double legendWidth )
{
  return drawTitleInternal( rendercontext, nullptr, top, halignment, legendWidth );
}

QSizeF QgsLegendRenderer::drawGroup( const LegendComponentGroup &atom, QgsRenderContext *rendercontext, const ColumnContext &columnContext, double top )
{
  return drawGroupInternal( atom, rendercontext, columnContext, nullptr, top );
}

QgsLegendRenderer::LegendComponent QgsLegendRenderer::drawSymbolItem( QgsLayerTreeModelLegendNode *symbolItem, QgsRenderContext *rendercontext, const ColumnContext &columnContext, double top, double maxSiblingSymbolWidth )
{
  return drawSymbolItemInternal( symbolItem, columnContext, rendercontext, nullptr, top, maxSiblingSymbolWidth );
}

QSizeF QgsLegendRenderer::drawLayerTitle( QgsLayerTreeLayer *nodeLayer, QgsRenderContext *rendercontext, const ColumnContext &columnContext, double top )
{
  return drawLayerTitleInternal( nodeLayer, columnContext, rendercontext, nullptr, top );
}

QSizeF QgsLegendRenderer::drawGroupTitle( QgsLayerTreeGroup *nodeGroup, QgsRenderContext *rendercontext, const ColumnContext &columnContext, double top )
{
  return drawGroupTitleInternal( nodeGroup, columnContext, rendercontext, nullptr, top );
}

void QgsLegendRenderer::drawLegend( QgsRenderContext &context )
{
  paintAndDetermineSize( &context );
}

QSizeF QgsLegendRenderer::paintAndDetermineSize( QgsRenderContext *context )
{
  return paintAndDetermineSizeInternal( context, nullptr );
}
