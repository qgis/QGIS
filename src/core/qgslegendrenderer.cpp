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

  QList<Atom> atomList = createAtomList( rootGroup, mSettings.splitLayer() );

  setColumns( atomList );

  qreal maxColumnWidth = 0;
  if ( mSettings.equalColumnWidth() )
  {
    Q_FOREACH ( const Atom &atom, atomList )
    {
      maxColumnWidth = std::max( atom.size.width(), maxColumnWidth );
    }
  }

  //calculate size of title
  QSizeF titleSize = drawTitle();
  //add title margin to size of title text
  titleSize.rwidth() += mSettings.boxSpace() * 2.0;
  double columnTop = mSettings.boxSpace() + titleSize.height() + mSettings.style( QgsLegendStyle::Title ).margin( QgsLegendStyle::Bottom );

  QPointF point( mSettings.boxSpace(), columnTop );
  bool firstInColumn = true;
  double columnMaxHeight = 0;
  qreal columnWidth = 0;
  int column = 0;
  Q_FOREACH ( const Atom &atom, atomList )
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

    QSizeF atomSize = context ? drawAtom( atom, context, point )
                      : drawAtom( atom, painter, point );
    columnWidth = std::max( atomSize.width(), columnWidth );

    point.ry() += atom.size.height();
    columnMaxHeight = std::max( point.y() - columnTop, columnMaxHeight );

    firstInColumn = false;
  }
  point.rx() += columnWidth + mSettings.boxSpace();

  size.rheight() = columnTop + columnMaxHeight + mSettings.boxSpace();
  size.rwidth() = point.x();
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
    if ( context )
      drawTitle( context, point, mSettings.titleAlignment(), size.width() );
    else
      drawTitle( painter, point, mSettings.titleAlignment(), size.width() );
  }

  return size;
}


QList<QgsLegendRenderer::Atom> QgsLegendRenderer::createAtomList( QgsLayerTreeGroup *parentGroup, bool splitLayer )
{
  QList<Atom> atoms;

  if ( !parentGroup ) return atoms;

  Q_FOREACH ( QgsLayerTreeNode *node, parentGroup->children() )
  {
    if ( QgsLayerTree::isGroup( node ) )
    {
      QgsLayerTreeGroup *nodeGroup = QgsLayerTree::toGroup( node );

      // Group subitems
      QList<Atom> groupAtoms = createAtomList( nodeGroup, splitLayer );
      bool hasSubItems = !groupAtoms.empty();

      if ( nodeLegendStyle( nodeGroup ) != QgsLegendStyle::Hidden )
      {
        Nucleon nucleon;
        nucleon.item = node;
        nucleon.size = drawGroupTitle( nodeGroup );

        if ( !groupAtoms.isEmpty() )
        {
          // Add internal space between this group title and the next nucleon
          groupAtoms[0].size.rheight() += spaceAboveAtom( groupAtoms[0] );
          // Prepend this group title to the first atom
          groupAtoms[0].nucleons.prepend( nucleon );
          groupAtoms[0].size.rheight() += nucleon.size.height();
          groupAtoms[0].size.rwidth() = std::max( nucleon.size.width(), groupAtoms[0].size.width() );
        }
        else
        {
          // no subitems, append new atom
          Atom atom;
          atom.nucleons.append( nucleon );
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

      Atom atom;

      if ( nodeLegendStyle( nodeLayer ) != QgsLegendStyle::Hidden )
      {
        Nucleon nucleon;
        nucleon.item = node;
        nucleon.size = drawLayerTitle( nodeLayer );
        atom.nucleons.append( nucleon );
        atom.size.rwidth() = nucleon.size.width();
        atom.size.rheight() = nucleon.size.height();
      }

      QList<QgsLayerTreeModelLegendNode *> legendNodes = mLegendModel->layerLegendNodes( nodeLayer );

      // workaround for the issue that "filtering by map" does not remove layer nodes that have no symbols present
      // on the map. We explicitly skip such layers here. In future ideally that should be handled directly
      // in the layer tree model
      if ( legendNodes.isEmpty() && mLegendModel->legendFilterMapSettings() )
        continue;

      QList<Atom> layerAtoms;

      for ( int j = 0; j < legendNodes.count(); j++ )
      {
        QgsLayerTreeModelLegendNode *legendNode = legendNodes.at( j );

        Nucleon symbolNucleon = drawSymbolItem( legendNode );

        if ( !mSettings.splitLayer() || j == 0 )
        {
          // append to layer atom
          // the width is not correct at this moment, we must align all symbol labels
          atom.size.rwidth() = std::max( symbolNucleon.size.width(), atom.size.width() );
          // Add symbol space only if there is already title or another item above
          if ( !atom.nucleons.isEmpty() )
          {
            // TODO: for now we keep Symbol and SymbolLabel Top margin in sync
            atom.size.rheight() += mSettings.style( QgsLegendStyle::Symbol ).margin( QgsLegendStyle::Top );
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


void QgsLegendRenderer::setColumns( QList<Atom> &atomList )
{
  if ( mSettings.columnCount() == 0 ) return;

  // Divide atoms to columns
  double totalHeight = 0;
  qreal maxAtomHeight = 0;
  Q_FOREACH ( const Atom &atom, atomList )
  {
    totalHeight += spaceAboveAtom( atom );
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

    Atom atom = atomList.at( i );
    double currentHeight = currentColumnHeight;
    if ( currentColumnAtomCount > 0 )
      currentHeight += spaceAboveAtom( atom );
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
    Atom &atom = atomList[i];
    for ( int j = 0; j < atom.nucleons.size(); j++ )
    {
      if ( QgsLayerTreeModelLegendNode *legendNode = qobject_cast<QgsLayerTreeModelLegendNode *>( atom.nucleons.at( j ).item ) )
      {
        QString key = QStringLiteral( "%1-%2" ).arg( reinterpret_cast< qulonglong >( legendNode->layerNode() ) ).arg( atom.column );
        maxSymbolWidth[key] = std::max( atom.nucleons.at( j ).symbolSize.width(), maxSymbolWidth[key] );
      }
    }
  }
  for ( int i = 0; i < atomList.size(); i++ )
  {
    Atom &atom = atomList[i];
    for ( int j = 0; j < atom.nucleons.size(); j++ )
    {
      if ( QgsLayerTreeModelLegendNode *legendNode = qobject_cast<QgsLayerTreeModelLegendNode *>( atom.nucleons.at( j ).item ) )
      {
        QString key = QStringLiteral( "%1-%2" ).arg( reinterpret_cast< qulonglong >( legendNode->layerNode() ) ).arg( atom.column );
        double space = mSettings.style( QgsLegendStyle::Symbol ).margin( QgsLegendStyle::Right ) +
                       mSettings.style( QgsLegendStyle::SymbolLabel ).margin( QgsLegendStyle::Left );
        atom.nucleons[j].labelXOffset = maxSymbolWidth[key] + space;
        atom.nucleons[j].size.rwidth() = maxSymbolWidth[key] + space + atom.nucleons.at( j ).labelSize.width();
      }
    }
  }
}

QSizeF QgsLegendRenderer::drawTitle( QPainter *painter, QPointF point, Qt::AlignmentFlag halignment, double legendWidth )
{
  return drawTitleInternal( nullptr, painter, point, halignment, legendWidth );
}

QSizeF QgsLegendRenderer::drawTitleInternal( QgsRenderContext *context, QPainter *painter, QPointF point, Qt::AlignmentFlag halignment, double legendWidth )
{
  QSizeF size( 0, 0 );
  if ( mSettings.title().isEmpty() )
  {
    return size;
  }

  QStringList lines = mSettings.splitStringForWrapping( mSettings.title() );
  double y = point.y();

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
  switch ( halignment )
  {
    case Qt::AlignHCenter:
      textBoxWidth = ( std::min( static_cast< double >( point.x() ), legendWidth - point.x() ) - mSettings.boxSpace() ) * 2.0;
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
  size.rheight() = y - point.y();

  return size;
}


double QgsLegendRenderer::spaceAboveAtom( const Atom &atom )
{
  if ( atom.nucleons.isEmpty() ) return 0;

  Nucleon nucleon = atom.nucleons.first();

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
QSizeF QgsLegendRenderer::drawAtom( const Atom &atom, QPainter *painter, QPointF point )
{
  return drawAtomInternal( atom, nullptr, painter, point );
}

QSizeF QgsLegendRenderer::drawAtomInternal( const Atom &atom, QgsRenderContext *context, QPainter *painter, QPointF point )
{
  bool first = true;
  QSizeF size = QSizeF( atom.size );
  Q_FOREACH ( const Nucleon &nucleon, atom.nucleons )
  {
    if ( QgsLayerTreeGroup *groupItem = qobject_cast<QgsLayerTreeGroup *>( nucleon.item ) )
    {
      QgsLegendStyle::Style s = nodeLegendStyle( groupItem );
      if ( s != QgsLegendStyle::Hidden )
      {
        if ( !first )
        {
          point.ry() += mSettings.style( s ).margin( QgsLegendStyle::Top );
        }
        if ( context )
          drawGroupTitle( groupItem, context, point );
        else
          drawGroupTitle( groupItem, painter, point );
      }
    }
    else if ( QgsLayerTreeLayer *layerItem = qobject_cast<QgsLayerTreeLayer *>( nucleon.item ) )
    {
      QgsLegendStyle::Style s = nodeLegendStyle( layerItem );
      if ( s != QgsLegendStyle::Hidden )
      {
        if ( !first )
        {
          point.ry() += mSettings.style( s ).margin( QgsLegendStyle::Top );
        }
        if ( context )
          drawLayerTitle( layerItem, context, point );
        else
          drawLayerTitle( layerItem, painter, point );
      }
    }
    else if ( QgsLayerTreeModelLegendNode *legendNode = qobject_cast<QgsLayerTreeModelLegendNode *>( nucleon.item ) )
    {
      if ( !first )
      {
        point.ry() += mSettings.style( QgsLegendStyle::Symbol ).margin( QgsLegendStyle::Top );
      }

      Nucleon symbolNucleon = context ? drawSymbolItem( legendNode, context, point, nucleon.labelXOffset )
                              : drawSymbolItem( legendNode, painter, point, nucleon.labelXOffset );
      // expand width, it may be wider because of labelXOffset
      size.rwidth() = std::max( symbolNucleon.size.width(), size.width() );
    }
    point.ry() += nucleon.size.height();
    first = false;
  }
  return size;
}

QgsLegendRenderer::Nucleon QgsLegendRenderer::drawSymbolItem( QgsLayerTreeModelLegendNode *symbolItem, QPainter *painter, QPointF point, double labelXOffset )
{
  return drawSymbolItemInternal( symbolItem, nullptr, painter, point, labelXOffset );
}

QgsLegendRenderer::Nucleon QgsLegendRenderer::drawSymbolItemInternal( QgsLayerTreeModelLegendNode *symbolItem, QgsRenderContext *context, QPainter *painter, QPointF point, double labelXOffset )
{
  QgsLayerTreeModelLegendNode::ItemContext ctx;
  ctx.context = context;
  ctx.painter = context ? context->painter() : painter;
  ctx.point = point;
  ctx.labelXOffset = labelXOffset;

  QgsLayerTreeModelLegendNode::ItemMetrics im = symbolItem->draw( mSettings, context ? &ctx
      : ( painter ? &ctx : nullptr ) );

  Nucleon nucleon;
  nucleon.item = symbolItem;
  nucleon.symbolSize = im.symbolSize;
  nucleon.labelSize = im.labelSize;
  //QgsDebugMsg( QStringLiteral( "symbol height = %1 label height = %2").arg( symbolSize.height()).arg( labelSize.height() ));
  double width = std::max( static_cast< double >( im.symbolSize.width() ), labelXOffset ) + im.labelSize.width();
  double height = std::max( im.symbolSize.height(), im.labelSize.height() );
  nucleon.size = QSizeF( width, height );
  return nucleon;
}

QSizeF QgsLegendRenderer::drawLayerTitle( QgsLayerTreeLayer *nodeLayer, QPainter *painter, QPointF point )
{
  return drawLayerTitleInternal( nodeLayer, nullptr, painter, point );
}

QSizeF QgsLegendRenderer::drawLayerTitleInternal( QgsLayerTreeLayer *nodeLayer, QgsRenderContext *context, QPainter *painter, QPointF point )
{
  QSizeF size( 0, 0 );
  QModelIndex idx = mLegendModel->node2index( nodeLayer );

  //Let the user omit the layer title item by having an empty layer title string
  if ( mLegendModel->data( idx, Qt::DisplayRole ).toString().isEmpty() )
    return size;

  double y = point.y();

  if ( context && context->painter() )
    context->painter()->setPen( mSettings.fontColor() );
  else if ( painter )
    painter->setPen( mSettings.fontColor() );

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
  for ( QStringList::ConstIterator layerItemPart = lines.constBegin(); layerItemPart != lines.constEnd(); ++layerItemPart )
  {
    y += mSettings.fontAscentMillimeters( layerFont );
    if ( context && context->painter() )
      mSettings.drawText( context->painter(), point.x(), y, *layerItemPart, layerFont );
    if ( painter )
      mSettings.drawText( painter, point.x(), y, *layerItemPart, layerFont );
    qreal width = mSettings.textWidthMillimeters( layerFont, *layerItemPart );
    size.rwidth() = std::max( width, size.width() );
    if ( layerItemPart != ( lines.end() - 1 ) )
    {
      y += mSettings.lineSpacing();
    }
  }
  size.rheight() = y - point.y();
  size.rheight() += mSettings.style( nodeLegendStyle( nodeLayer ) ).margin( QgsLegendStyle::Side::Bottom );

  if ( layerScope )
    delete context->expressionContext().popScope();

  return size;
}

QSizeF QgsLegendRenderer::drawGroupTitle( QgsLayerTreeGroup *nodeGroup, QPainter *painter, QPointF point )
{
  return drawGroupTitleInternal( nodeGroup, nullptr, painter, point );
}

QSizeF QgsLegendRenderer::drawGroupTitleInternal( QgsLayerTreeGroup *nodeGroup, QgsRenderContext *context, QPainter *painter, QPointF point )
{
  QSizeF size( 0, 0 );
  QModelIndex idx = mLegendModel->node2index( nodeGroup );

  double y = point.y();

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
    if ( context && context->painter() )
      mSettings.drawText( context->painter(), point.x(), y, *groupPart, groupFont );
    else if ( painter )
      mSettings.drawText( painter, point.x(), y, *groupPart, groupFont );
    qreal width = mSettings.textWidthMillimeters( groupFont, *groupPart );
    size.rwidth() = std::max( width, size.width() );
    if ( groupPart != ( lines.end() - 1 ) )
    {
      y += mSettings.lineSpacing();
    }
  }
  size.rheight() = y - point.y();
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

QSizeF QgsLegendRenderer::drawTitle( QgsRenderContext *rendercontext, QPointF point, Qt::AlignmentFlag halignment, double legendWidth )
{
  return drawTitleInternal( rendercontext, nullptr, point, halignment, legendWidth );
}

QSizeF QgsLegendRenderer::drawAtom( const Atom &atom, QgsRenderContext *rendercontext, QPointF point )
{
  return drawAtomInternal( atom, rendercontext, nullptr, point );
}

QgsLegendRenderer::Nucleon QgsLegendRenderer::drawSymbolItem( QgsLayerTreeModelLegendNode *symbolItem, QgsRenderContext *rendercontext, QPointF point, double labelXOffset )
{
  return drawSymbolItemInternal( symbolItem, rendercontext, nullptr, point, labelXOffset );
}

QSizeF QgsLegendRenderer::drawLayerTitle( QgsLayerTreeLayer *nodeLayer, QgsRenderContext *rendercontext, QPointF point )
{
  return drawLayerTitleInternal( nodeLayer, rendercontext, nullptr, point );
}

QSizeF QgsLegendRenderer::drawGroupTitle( QgsLayerTreeGroup *nodeGroup, QgsRenderContext *rendercontext, QPointF point )
{
  return drawGroupTitleInternal( nodeGroup, rendercontext, nullptr, point );
}

void QgsLegendRenderer::drawLegend( QgsRenderContext &context )
{
  paintAndDetermineSize( &context );
}

QSizeF QgsLegendRenderer::paintAndDetermineSize( QgsRenderContext *context )
{
  return paintAndDetermineSizeInternal( context, nullptr );
}
