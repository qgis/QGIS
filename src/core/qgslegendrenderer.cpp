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

#include "qgscomposerlegenditem.h"
#include "qgslayertree.h"
#include "qgslayertreemodel.h"
#include "qgslayertreemodellegendnode.h"
#include "qgslegendmodel.h"
#include "qgsmaplayerlegend.h"
#include "qgsmaplayerregistry.h"
#include "qgssymbolv2.h"
#include "qgsvectorlayer.h"

#include <QPainter>



QgsLegendRenderer::QgsLegendRenderer( QgsLayerTreeModel* legendModel, const QgsLegendSettings& settings )
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
  QgsLayerTreeGroup* rootGroup = mLegendModel->rootGroup();
  if ( !rootGroup ) return size;

  QList<Atom> atomList = createAtomList( rootGroup, mSettings.splitLayer() );

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





QList<QgsLegendRenderer::Atom> QgsLegendRenderer::createAtomList( QgsLayerTreeGroup* parentGroup, bool splitLayer )
{
  QList<Atom> atoms;

  if ( !parentGroup ) return atoms;

  foreach ( QgsLayerTreeNode* node, parentGroup->children() )
  {
    if ( QgsLayerTree::isGroup( node ) )
    {
      QgsLayerTreeGroup* nodeGroup = QgsLayerTree::toGroup( node );

      // Group subitems
      QList<Atom> groupAtoms = createAtomList( nodeGroup, splitLayer );

      Nucleon nucleon;
      nucleon.item = node;
      nucleon.size = drawGroupTitle( nodeGroup );

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
    else if ( QgsLayerTree::isLayer( node ) )
    {
      QgsLayerTreeLayer* nodeLayer = QgsLayerTree::toLayer( node );

      Atom atom;

      if ( nodeLegendStyle( nodeLayer ) != QgsComposerLegendStyle::Hidden )
      {
        Nucleon nucleon;
        nucleon.item = node;
        nucleon.size = drawLayerTitle( nodeLayer );
        atom.nucleons.append( nucleon );
        atom.size.rwidth() = nucleon.size.width();
        atom.size.rheight() = nucleon.size.height();
      }

      QList<QgsLayerTreeModelLegendNode*> legendNodes = mLegendModel->layerLegendNodes( nodeLayer );

      QList<Atom> layerAtoms;

      for ( int j = 0; j < legendNodes.count(); j++ )
      {
        QgsLayerTreeModelLegendNode* legendNode = legendNodes.at( j );

        Nucleon symbolNucleon = drawSymbolItem( legendNode );

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
    Atom& atom = atomList[i];
    for ( int j = 0; j < atom.nucleons.size(); j++ )
    {
      if ( QgsLayerTreeModelLegendNode* legendNode = qobject_cast<QgsLayerTreeModelLegendNode*>( atom.nucleons[j].item ) )
      {
        QString key = QString( "%1-%2" ).arg(( qulonglong )legendNode->parent() ).arg( atom.column );
        maxSymbolWidth[key] = qMax( atom.nucleons[j].symbolSize.width(), maxSymbolWidth[key] );
      }
    }
  }
  for ( int i = 0; i < atomList.size(); i++ )
  {
    Atom& atom = atomList[i];
    for ( int j = 0; j < atom.nucleons.size(); j++ )
    {
      if ( QgsLayerTreeModelLegendNode* legendNode = qobject_cast<QgsLayerTreeModelLegendNode*>( atom.nucleons[j].item ) )
      {
        QString key = QString( "%1-%2" ).arg(( qulonglong )legendNode->parent() ).arg( atom.column );
        double space = mSettings.style( QgsComposerLegendStyle::Symbol ).margin( QgsComposerLegendStyle::Right ) +
                       mSettings.style( QgsComposerLegendStyle::SymbolLabel ).margin( QgsComposerLegendStyle::Left );
        atom.nucleons[j].labelXOffset =  maxSymbolWidth[key] + space;
        atom.nucleons[j].size.rwidth() =  maxSymbolWidth[key] + space + atom.nucleons[j].labelSize.width();
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

  QStringList lines = mSettings.splitStringForWrapping( mSettings.title() );
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
    qreal width = mSettings.textWidthMillimeters( titleFont, *titlePart ) + 1;
    qreal height = mSettings.fontAscentMillimeters( titleFont ) + mSettings.fontDescentMillimeters( titleFont );

    QRectF r( textBoxLeft, y, textBoxWidth, height );

    if ( painter )
    {
      mSettings.drawText( painter, r, *titlePart, titleFont, halignment, Qt::AlignVCenter, Qt::TextDontClip );
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

  if ( QgsLayerTreeGroup* nodeGroup = qobject_cast<QgsLayerTreeGroup*>( nucleon.item ) )
  {
    return mSettings.style( nodeLegendStyle( nodeGroup ) ).margin( QgsComposerLegendStyle::Top );
  }
  else if ( QgsLayerTreeLayer* nodeLayer = qobject_cast<QgsLayerTreeLayer*>( nucleon.item ) )
  {
    return mSettings.style( nodeLegendStyle( nodeLayer ) ).margin( QgsComposerLegendStyle::Top );
  }
  else if ( qobject_cast<QgsLayerTreeModelLegendNode*>( nucleon.item ) )
  {
    // TODO: use Symbol or SymbolLabel Top margin
    return mSettings.style( QgsComposerLegendStyle::Symbol ).margin( QgsComposerLegendStyle::Top );
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
    if ( QgsLayerTreeGroup* groupItem = qobject_cast<QgsLayerTreeGroup*>( nucleon.item ) )
    {
      QgsComposerLegendStyle::Style s = nodeLegendStyle( groupItem );
      if ( s != QgsComposerLegendStyle::Hidden )
      {
        if ( !first )
        {
          point.ry() += mSettings.style( s ).margin( QgsComposerLegendStyle::Top );
        }
        drawGroupTitle( groupItem, painter, point );
      }
    }
    else if ( QgsLayerTreeLayer* layerItem = qobject_cast<QgsLayerTreeLayer*>( nucleon.item ) )
    {
      QgsComposerLegendStyle::Style s = nodeLegendStyle( layerItem );
      if ( s != QgsComposerLegendStyle::Hidden )
      {
        if ( !first )
        {
          point.ry() += mSettings.style( s ).margin( QgsComposerLegendStyle::Top );
        }
        drawLayerTitle( layerItem, painter, point );
      }
    }
    else if ( QgsLayerTreeModelLegendNode* legendNode = qobject_cast<QgsLayerTreeModelLegendNode*>( nucleon.item ) )
    {
      if ( !first )
      {
        point.ry() += mSettings.style( QgsComposerLegendStyle::Symbol ).margin( QgsComposerLegendStyle::Top );
      }

      Nucleon symbolNucleon = drawSymbolItem( legendNode, painter, point, nucleon.labelXOffset );
      // expand width, it may be wider because of labelXOffset
      size.rwidth() = qMax( symbolNucleon.size.width(), size.width() );
    }
    point.ry() += nucleon.size.height();
    first = false;
  }
  return size;
}


QgsLegendRenderer::Nucleon QgsLegendRenderer::drawSymbolItem( QgsLayerTreeModelLegendNode* symbolItem, QPainter* painter, QPointF point, double labelXOffset )
{
  QgsLayerTreeModelLegendNode::ItemContext ctx;
  ctx.painter = painter;
  ctx.point = point;
  ctx.labelXOffset = labelXOffset;

  QgsLayerTreeModelLegendNode::ItemMetrics im = symbolItem->draw( mSettings, painter ? &ctx : 0 );

  Nucleon nucleon;
  nucleon.item = symbolItem;
  nucleon.symbolSize = im.symbolSize;
  nucleon.labelSize = im.labelSize;
  //QgsDebugMsg( QString( "symbol height = %1 label height = %2").arg( symbolSize.height()).arg( labelSize.height() ));
  double width = qMax(( double ) im.symbolSize.width(), labelXOffset ) + im.labelSize.width();
  double height = qMax( im.symbolSize.height(), im.labelSize.height() );
  nucleon.size = QSizeF( width, height );
  return nucleon;
}


QSizeF QgsLegendRenderer::drawLayerTitle( QgsLayerTreeLayer* nodeLayer, QPainter* painter, QPointF point )
{
  QSizeF size( 0, 0 );
  QModelIndex idx = mLegendModel->node2index( nodeLayer );

  //Let the user omit the layer title item by having an empty layer title string
  if ( mLegendModel->data( idx, Qt::DisplayRole ).toString().isEmpty() ) return size;

  double y = point.y();

  if ( painter ) painter->setPen( mSettings.fontColor() );

  QFont layerFont = mSettings.style( nodeLegendStyle( nodeLayer ) ).font();

  QStringList lines = mSettings.splitStringForWrapping( mLegendModel->data( idx, Qt::DisplayRole ).toString() );
  for ( QStringList::Iterator layerItemPart = lines.begin(); layerItemPart != lines.end(); ++layerItemPart )
  {
    y += mSettings.fontAscentMillimeters( layerFont );
    if ( painter ) mSettings.drawText( painter, point.x(), y, *layerItemPart , layerFont );
    qreal width = mSettings.textWidthMillimeters( layerFont, *layerItemPart );
    size.rwidth() = qMax( width, size.width() );
    if ( layerItemPart != lines.end() )
    {
      y += mSettings.lineSpacing();
    }
  }
  size.rheight() = y - point.y();

  return size;
}


QSizeF QgsLegendRenderer::drawGroupTitle( QgsLayerTreeGroup* nodeGroup, QPainter* painter, QPointF point )
{
  QSizeF size( 0, 0 );
  QModelIndex idx = mLegendModel->node2index( nodeGroup );

  double y = point.y();

  if ( painter ) painter->setPen( mSettings.fontColor() );

  QFont groupFont = mSettings.style( nodeLegendStyle( nodeGroup ) ).font();

  QStringList lines = mSettings.splitStringForWrapping( mLegendModel->data( idx, Qt::DisplayRole ).toString() );
  for ( QStringList::Iterator groupPart = lines.begin(); groupPart != lines.end(); ++groupPart )
  {
    y += mSettings.fontAscentMillimeters( groupFont );
    if ( painter ) mSettings.drawText( painter, point.x(), y, *groupPart, groupFont );
    qreal width = mSettings.textWidthMillimeters( groupFont, *groupPart );
    size.rwidth() = qMax( width, size.width() );
    if ( groupPart != lines.end() )
    {
      y += mSettings.lineSpacing();
    }
  }
  size.rheight() = y - point.y();
  return size;
}



QgsComposerLegendStyle::Style QgsLegendRenderer::nodeLegendStyle( QgsLayerTreeNode* node, QgsLayerTreeModel* model )
{
  QString style = node->customProperty( "legend/title-style" ).toString();
  if ( style == "hidden" )
    return QgsComposerLegendStyle::Hidden;
  else if ( style == "group" )
    return QgsComposerLegendStyle::Group;
  else if ( style == "subgroup" )
    return QgsComposerLegendStyle::Subgroup;

  // use a default otherwise
  if ( QgsLayerTree::isGroup( node ) )
    return QgsComposerLegendStyle::Group;
  else if ( QgsLayerTree::isLayer( node ) )
  {
    QList<QgsLayerTreeModelLegendNode*> legendNodes = model->layerLegendNodes( QgsLayerTree::toLayer( node ) );
    if ( legendNodes.count() == 1 && legendNodes[0]->isEmbeddedInParent() )
      return QgsComposerLegendStyle::Hidden;
    return QgsComposerLegendStyle::Subgroup;
  }

  return QgsComposerLegendStyle::Undefined; // should not happen, only if corrupted project file
}

QgsComposerLegendStyle::Style QgsLegendRenderer::nodeLegendStyle( QgsLayerTreeNode* node )
{
  return nodeLegendStyle( node, mLegendModel );
}

void QgsLegendRenderer::setNodeLegendStyle( QgsLayerTreeNode* node, QgsComposerLegendStyle::Style style )
{
  QString str;
  switch ( style )
  {
    case QgsComposerLegendStyle::Hidden:   str = "hidden"; break;
    case QgsComposerLegendStyle::Group:    str = "group"; break;
    case QgsComposerLegendStyle::Subgroup: str = "subgroup"; break;
    default: break; // nothing
  }

  if ( !str.isEmpty() )
    node->setCustomProperty( "legend/title-style", str );
  else
    node->removeCustomProperty( "legend/title-style" );
}
