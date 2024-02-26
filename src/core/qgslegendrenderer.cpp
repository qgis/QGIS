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
#include "qgsrendercontext.h"
#include "qgsexpressioncontextutils.h"
#include "qgstextrenderer.h"
#include <QJsonObject>
#include <QPainter>



QgsLegendRenderer::QgsLegendRenderer( QgsLayerTreeModel *legendModel, const QgsLegendSettings &settings )
  : mLegendModel( legendModel )
  , mSettings( settings )
{
}

QSizeF QgsLegendRenderer::minimumSize( QgsRenderContext *renderContext )
{
  std::unique_ptr< QgsRenderContext > tmpContext;

  if ( !renderContext )
  {
    // QGIS 4.0 - make render context mandatory
    Q_NOWARN_DEPRECATED_PUSH
    tmpContext.reset( new QgsRenderContext( QgsRenderContext::fromQPainter( nullptr ) ) );
    tmpContext->setRendererScale( mSettings.mapScale() );
    tmpContext->setMapToPixel( QgsMapToPixel( 1 / ( mSettings.mmPerMapUnit() * tmpContext->scaleFactor() ) ) );
    tmpContext->setFlag( Qgis::RenderContextFlag::ApplyScalingWorkaroundForTextRendering, true );
    renderContext = tmpContext.get();
    Q_NOWARN_DEPRECATED_POP
  }

  QgsScopedRenderContextPainterSwap nullPainterSwap( *renderContext, nullptr );
  return paintAndDetermineSize( *renderContext );
}

void QgsLegendRenderer::drawLegend( QPainter *painter )
{
  Q_NOWARN_DEPRECATED_PUSH
  QgsRenderContext context = QgsRenderContext::fromQPainter( painter );
  QgsScopedRenderContextScaleToMm scaleToMm( context );

  context.setRendererScale( mSettings.mapScale() );
  context.setMapToPixel( QgsMapToPixel( 1 / ( mSettings.mmPerMapUnit() * context.scaleFactor() ) ) );
  Q_NOWARN_DEPRECATED_POP

  paintAndDetermineSize( context );
}

QJsonObject QgsLegendRenderer::exportLegendToJson( const QgsRenderContext &context )
{
  QJsonObject json;

  QgsLayerTreeGroup *rootGroup = mLegendModel->rootGroup();
  if ( !rootGroup )
    return json;

  json = exportLegendToJson( context, rootGroup );
  json[QStringLiteral( "title" )] = mSettings.title();
  return json;
}

QJsonObject QgsLegendRenderer::exportLegendToJson( const QgsRenderContext &context, QgsLayerTreeGroup *nodeGroup )
{
  QJsonObject json;
  QJsonArray nodes;
  const QList<QgsLayerTreeNode *> childNodes = nodeGroup->children();
  for ( QgsLayerTreeNode *node : childNodes )
  {
    if ( QgsLayerTree::isGroup( node ) )
    {
      QgsLayerTreeGroup *nodeGroup = QgsLayerTree::toGroup( node );
      const QModelIndex idx = mLegendModel->node2index( nodeGroup );
      const QString text = mLegendModel->data( idx, Qt::DisplayRole ).toString();

      QJsonObject group = exportLegendToJson( context, nodeGroup );
      group[ QStringLiteral( "type" ) ] = QStringLiteral( "group" );
      group[ QStringLiteral( "title" ) ] = text;
      nodes.append( group );
    }
    else if ( QgsLayerTree::isLayer( node ) )
    {
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
        QJsonObject group = legendNodes.at( 0 )->exportToJson( mSettings, context );
        group[ QStringLiteral( "type" ) ] = QStringLiteral( "layer" );
        if ( mSettings.jsonRenderFlags().testFlag( Qgis::LegendJsonRenderFlag::ShowRuleDetails ) )
        {
          if ( QgsVectorLayer *vLayer = qobject_cast<QgsVectorLayer *>( nodeLayer->layer() ) )
          {
            if ( vLayer->renderer() )
            {
              const QString ruleKey { legendNodes.at( 0 )->data( static_cast< int >( QgsLayerTreeModelLegendNode::CustomRole::RuleKey ) ).toString() };
              if ( ! ruleKey.isEmpty() )
              {
                bool ok;
                const QString ruleExp { vLayer->renderer()->legendKeyToExpression( ruleKey, vLayer, ok ) };
                if ( ok )
                {
                  group[ QStringLiteral( "rule" ) ] = ruleExp;
                }
              }
            }
          }
        }
        nodes.append( group );
      }
      else if ( legendNodes.count() > 1 )
      {
        QJsonObject group;
        group[ QStringLiteral( "type" ) ] = QStringLiteral( "layer" );
        group[ QStringLiteral( "title" ) ] = text;

        QJsonArray symbols;
        for ( int j = 0; j < legendNodes.count(); j++ )
        {
          QgsLayerTreeModelLegendNode *legendNode = legendNodes.at( j );
          QJsonObject symbol = legendNode->exportToJson( mSettings, context );
          if ( mSettings.jsonRenderFlags().testFlag( Qgis::LegendJsonRenderFlag::ShowRuleDetails ) )
          {
            if ( QgsVectorLayer *vLayer = qobject_cast<QgsVectorLayer *>( nodeLayer->layer() ) )
            {
              if ( vLayer->renderer() )
              {
                const QString ruleKey { legendNode->data( static_cast< int >( QgsLayerTreeModelLegendNode::CustomRole::RuleKey ) ).toString() };
                if ( ! ruleKey.isEmpty() )
                {
                  bool ok;
                  const QString ruleExp { vLayer->renderer()->legendKeyToExpression( ruleKey, vLayer, ok ) };
                  if ( ok )
                  {
                    symbol[ QStringLiteral( "rule" ) ] = ruleExp;
                  }
                }
              }
            }
          }
          symbols.append( symbol );
        }
        group[ QStringLiteral( "symbols" ) ] = symbols;

        nodes.append( group );
      }
    }
  }

  json[QStringLiteral( "nodes" )] = nodes;
  return json;
}

QSizeF QgsLegendRenderer::paintAndDetermineSize( QgsRenderContext &context )
{
  QSizeF size( 0, 0 );
  QgsLayerTreeGroup *rootGroup = mLegendModel->rootGroup();
  if ( !rootGroup )
    return size;

  // temporarily remove painter from context -- we don't need to actually draw anything yet. But we DO need
  // to send the full render context so that an expression context is available during the size calculation
  QgsScopedRenderContextPainterSwap noPainter( context, nullptr );

  QList<LegendComponentGroup> componentGroups = createComponentGroupList( rootGroup, context );

  const int columnCount = setColumns( componentGroups );

  QMap< int, double > maxColumnWidths;
  qreal maxEqualColumnWidth = 0;
  // another iteration -- this one is required to calculate the maximum item width for each
  // column. Unfortunately, we can't trust the component group widths at this stage, as they are minimal widths
  // only. When actually rendering a symbol node, the text is aligned according to the WIDEST
  // symbol in a column. So that means we can't possibly determine the exact size of legend components
  // until now. BUUUUUUUUUUUUT. Because everything sucks, we can't even start the actual render of items
  // at the same time we calculate this -- legend items REQUIRE the REAL width of the columns in order to
  // correctly align right or center-aligned symbols/text. Bah -- A triple iteration it is!
  for ( const LegendComponentGroup &group : std::as_const( componentGroups ) )
  {
    const QSizeF actualSize = drawGroup( group, context, ColumnContext() );
    maxEqualColumnWidth = std::max( actualSize.width(), maxEqualColumnWidth );
    maxColumnWidths[ group.column ] = std::max( actualSize.width(), maxColumnWidths.value( group.column, 0 ) );
  }

  if ( columnCount == 1 )
  {
    // single column - use the full available width
    maxEqualColumnWidth = std::max( maxEqualColumnWidth, mLegendSize.width() - 2 * mSettings.boxSpace() );
    maxColumnWidths[ 0 ] = maxEqualColumnWidth;
  }

  //calculate size of title
  QSizeF titleSize = drawTitle( context, 0 );
  //add title margin to size of title text
  titleSize.rwidth() += mSettings.boxSpace() * 2.0;
  double columnTop = mSettings.boxSpace() + titleSize.height() + mSettings.style( QgsLegendStyle::Title ).margin( QgsLegendStyle::Bottom );

  noPainter.reset();

  bool firstInColumn = true;
  double columnMaxHeight = 0;
  qreal columnWidth = 0;
  int column = -1;
  ColumnContext columnContext;
  columnContext.left = mSettings.boxSpace();
  columnContext.right = std::max( mLegendSize.width() - mSettings.boxSpace(), mSettings.boxSpace() );
  double currentY = columnTop;

  for ( const LegendComponentGroup &group : std::as_const( componentGroups ) )
  {
    if ( group.column > column )
    {
      // Switch to next column
      columnContext.left = group.column > 0 ? ( columnContext.right + mSettings.columnSpace() ) : mSettings.boxSpace();
      columnWidth = mSettings.equalColumnWidth() ? maxEqualColumnWidth : maxColumnWidths.value( group.column );
      columnContext.right = columnContext.left + columnWidth;
      currentY = columnTop;
      column++;
      firstInColumn = true;
    }
    if ( !firstInColumn )
    {
      currentY += spaceAboveGroup( group );
    }

    drawGroup( group, context, columnContext, currentY );

    currentY += group.size.height();
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
    drawTitle( context, mSettings.boxSpace(), mSettings.titleAlignment(), size.width() );
  }

  return size;
}

void QgsLegendRenderer::widthAndOffsetForTitleText( const Qt::AlignmentFlag halignment, const double legendWidth, double &textBoxWidth, double &textBoxLeft ) const
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

QList<QgsLegendRenderer::LegendComponentGroup> QgsLegendRenderer::createComponentGroupList( QgsLayerTreeGroup *parentGroup, QgsRenderContext &context, double indent )
{
  QList<LegendComponentGroup> componentGroups;

  if ( !parentGroup )
    return componentGroups;

  const QList<QgsLayerTreeNode *> childNodes = parentGroup->children();
  for ( QgsLayerTreeNode *node : childNodes )
  {
    if ( QgsLayerTree::isGroup( node ) )
    {
      QgsLayerTreeGroup *nodeGroup = QgsLayerTree::toGroup( node );
      QString style = node->customProperty( QStringLiteral( "legend/title-style" ) ).toString();
      // Update the required indent for the group/subgroup items, starting from the indent accumulated from parent groups
      double newIndent = indent;
      if ( style == QLatin1String( "subgroup" ) )
      {
        newIndent += mSettings.style( QgsLegendStyle::Subgroup ).indent( );
      }
      else
      {
        newIndent += mSettings.style( QgsLegendStyle::Group ).indent( );
      }

      // Group subitems
      QList<LegendComponentGroup> subgroups = createComponentGroupList( nodeGroup, context, newIndent );

      bool hasSubItems = !subgroups.empty();

      if ( nodeLegendStyle( nodeGroup ) != QgsLegendStyle::Hidden )
      {
        LegendComponent component;
        component.item = node;
        component.indent = newIndent;
        component.size = drawGroupTitle( nodeGroup, context );

        if ( !subgroups.isEmpty() )
        {
          // Add internal space between this group title and the next component
          subgroups[0].size.rheight() += spaceAboveGroup( subgroups[0] );
          // Prepend this group title to the first group
          subgroups[0].components.prepend( component );
          subgroups[0].size.rheight() += component.size.height();
          subgroups[0].size.rwidth() = std::max( component.size.width(), subgroups[0].size.width() );
          if ( nodeGroup->customProperty( QStringLiteral( "legend/column-break" ) ).toInt() )
            subgroups[0].placeColumnBreakBeforeGroup = true;
        }
        else
        {
          // no subitems, create new group
          LegendComponentGroup group;
          group.placeColumnBreakBeforeGroup = nodeGroup->customProperty( QStringLiteral( "legend/column-break" ) ).toInt();
          group.components.append( component );
          group.size.rwidth() += component.size.width();
          group.size.rheight() += component.size.height();
          group.size.rwidth() = std::max( component.size.width(), group.size.width() );
          subgroups.append( group );
        }
      }

      if ( hasSubItems ) //leave away groups without content
      {
        componentGroups.append( subgroups );
      }

    }
    else if ( QgsLayerTree::isLayer( node ) )
    {
      QgsLayerTreeLayer *nodeLayer = QgsLayerTree::toLayer( node );
      QgsLegendStyle::Style layerStyle = nodeLegendStyle( nodeLayer );
      bool allowColumnSplit = false;
      switch ( nodeLayer->legendSplitBehavior() )
      {
        case QgsLayerTreeLayer::UseDefaultLegendSetting:
          allowColumnSplit = mSettings.splitLayer();
          break;
        case QgsLayerTreeLayer::AllowSplittingLegendNodesOverMultipleColumns:
          allowColumnSplit = true;
          break;
        case QgsLayerTreeLayer::PreventSplittingLegendNodesOverMultipleColumns:
          allowColumnSplit = false;
          break;
      }

      LegendComponentGroup group;
      group.placeColumnBreakBeforeGroup = nodeLayer->customProperty( QStringLiteral( "legend/column-break" ) ).toInt();

      if ( layerStyle != QgsLegendStyle::Hidden )
      {
        LegendComponent component;
        component.item = node;
        component.size = drawLayerTitle( nodeLayer, context );
        component.indent = indent;
        group.components.append( component );
        group.size.rwidth() = component.size.width();
        group.size.rheight() = component.size.height();
      }

      QList<QgsLayerTreeModelLegendNode *> legendNodes = mLegendModel->layerLegendNodes( nodeLayer );

      // workaround for the issue that "filtering by map" does not remove layer nodes that have no symbols present
      // on the map. We explicitly skip such layers here. In future ideally that should be handled directly
      // in the layer tree model
      if ( legendNodes.isEmpty() && mLegendModel->legendFilterMapSettings() )
        continue;

      QList<LegendComponentGroup> layerGroups;
      layerGroups.reserve( legendNodes.count() );

      bool groupIsLayerGroup = true;
      double symbolIndent = indent;
      switch ( layerStyle )
      {
        case QgsLegendStyle::Subgroup:
        case QgsLegendStyle::Group:
          symbolIndent += mSettings.style( layerStyle ).indent( );
          break;
        default:
          break;
      }
      for ( int j = 0; j < legendNodes.count(); j++ )
      {
        QgsLayerTreeModelLegendNode *legendNode = legendNodes.at( j );

        LegendComponent symbolComponent = drawSymbolItem( legendNode, context, ColumnContext(), 0 );

        const bool forceBreak = legendNode->columnBreak();

        if ( !allowColumnSplit || j == 0 )
        {
          if ( forceBreak )
          {
            if ( groupIsLayerGroup )
              layerGroups.prepend( group );
            else
              layerGroups.append( group );

            group = LegendComponentGroup();
            group.placeColumnBreakBeforeGroup = true;
            groupIsLayerGroup = false;
          }

          // append to layer group
          // the width is not correct at this moment, we must align all symbol labels
          group.size.rwidth() = std::max( symbolComponent.size.width(), group.size.width() );
          // Add symbol space only if there is already title or another item above
          if ( !group.components.isEmpty() )
          {
            // TODO: for now we keep Symbol and SymbolLabel Top margin in sync
            group.size.rheight() += mSettings.style( QgsLegendStyle::Symbol ).margin( QgsLegendStyle::Top );
          }
          group.size.rheight() += symbolComponent.size.height();
          symbolComponent.indent = symbolIndent;
          group.components.append( symbolComponent );
        }
        else
        {
          if ( group.size.height() > 0 )
          {
            if ( groupIsLayerGroup )
              layerGroups.prepend( group );
            else
              layerGroups.append( group );
            group = LegendComponentGroup();
            groupIsLayerGroup = false;
          }
          LegendComponentGroup symbolGroup;
          symbolGroup.placeColumnBreakBeforeGroup = forceBreak;
          symbolComponent.indent = symbolIndent;
          symbolGroup.components.append( symbolComponent );
          symbolGroup.size.rwidth() = symbolComponent.size.width();
          symbolGroup.size.rheight() = symbolComponent.size.height();
          layerGroups.append( symbolGroup );
        }
      }
      if ( group.size.height() > 0 )
      {
        if ( groupIsLayerGroup )
          layerGroups.prepend( group );
        else
          layerGroups.append( group );
      }
      componentGroups.append( layerGroups );
    }
  }

  return componentGroups;
}


int QgsLegendRenderer::setColumns( QList<LegendComponentGroup> &componentGroups )
{
  // Divide groups to columns
  double totalHeight = 0;
  qreal maxGroupHeight = 0;
  int forcedColumnBreaks = 0;
  double totalSpaceAboveGroups = 0;

  for ( const LegendComponentGroup &group : std::as_const( componentGroups ) )
  {
    const double topMargin = spaceAboveGroup( group );
    totalHeight += topMargin;
    totalSpaceAboveGroups += topMargin;

    const double groupHeight = group.size.height();
    totalHeight += groupHeight;
    maxGroupHeight = std::max( groupHeight, maxGroupHeight );

    if ( group.placeColumnBreakBeforeGroup )
      forcedColumnBreaks++;
  }
  const double totalGroupHeight = ( totalHeight - totalSpaceAboveGroups );
  double averageGroupHeight = totalGroupHeight / componentGroups.size();

  if ( mSettings.columnCount() == 0 && forcedColumnBreaks == 0 )
    return 0;

  // the target number of columns allowed is dictated by the number of forced column
  // breaks OR the manually set column count (whichever is greater!)
  const int targetNumberColumns = std::max( forcedColumnBreaks + 1, mSettings.columnCount() );
  const int numberAutoPlacedBreaks = targetNumberColumns - forcedColumnBreaks - 1;

  // We know height of each group and we have to split them into columns
  // minimizing max column height. It is sort of bin packing problem, NP-hard.
  // We are using simple heuristic, brute fore appeared to be to slow,
  // the number of combinations is N = n!/(k!*(n-k)!) where n = groupCount-1
  // and k = columnsCount-1
  double maxColumnHeight = 0;
  int currentColumn = 0;
  int currentColumnGroupCount = 0; // number of groups in current column
  double currentColumnHeight = 0;
  int autoPlacedBreaks = 0;

  // Calculate the expected average space between items
  double averageSpaceAboveGroups = 0;
  if ( componentGroups.size() > targetNumberColumns )
    averageSpaceAboveGroups = totalSpaceAboveGroups / ( componentGroups.size() );

  double totalRemainingGroupHeight = totalGroupHeight;
  double totalRemainingSpaceAboveGroups = totalSpaceAboveGroups;
  for ( int i = 0; i < componentGroups.size(); i++ )
  {
    const LegendComponentGroup &group = componentGroups.at( i );
    const double currentGroupHeight = group.size.height();
    const double spaceAboveCurrentGroup = spaceAboveGroup( group );

    totalRemainingGroupHeight -= currentGroupHeight;
    totalRemainingSpaceAboveGroups -= spaceAboveCurrentGroup;

    double currentColumnHeightIfGroupIsIncluded = currentColumnHeight;
    if ( currentColumnGroupCount > 0 )
      currentColumnHeightIfGroupIsIncluded += spaceAboveCurrentGroup;
    currentColumnHeightIfGroupIsIncluded += currentGroupHeight;

    const int numberRemainingGroupsIncludingThisOne = componentGroups.size() - i;
    const int numberRemainingColumnsIncludingThisOne = numberAutoPlacedBreaks + 1 - autoPlacedBreaks;
    const int numberRemainingColumnBreaks = numberRemainingColumnsIncludingThisOne - 1;

    const double averageRemainingSpaceAboveGroups = numberRemainingGroupsIncludingThisOne > 1 ? ( totalRemainingSpaceAboveGroups / ( numberRemainingGroupsIncludingThisOne - 1 ) ) : 0;
    const double estimatedRemainingSpaceAboveGroupsWhichWontBeUsedBecauseGroupsAreFirstInColumn = numberRemainingColumnBreaks * averageRemainingSpaceAboveGroups;
    const double estimatedRemainingTotalHeightAfterThisGroup = totalRemainingGroupHeight
        + totalRemainingSpaceAboveGroups
        - estimatedRemainingSpaceAboveGroupsWhichWontBeUsedBecauseGroupsAreFirstInColumn;

    const double estimatedTotalHeightOfRemainingColumnsIncludingThisOne = currentColumnHeightIfGroupIsIncluded
        + estimatedRemainingTotalHeightAfterThisGroup;

    // Recalc average height for remaining columns including current
    double averageRemainingColumnHeightIncludingThisOne = estimatedTotalHeightOfRemainingColumnsIncludingThisOne / numberRemainingColumnsIncludingThisOne;

    // Round up to the next full number of groups to put in one column
    // This ensures that earlier columns contain more elements than later columns
    const int averageGroupsPerRemainingColumnsIncludingThisOne = std::ceil( averageRemainingColumnHeightIncludingThisOne / ( averageGroupHeight + averageSpaceAboveGroups ) );

    averageRemainingColumnHeightIncludingThisOne = averageGroupsPerRemainingColumnsIncludingThisOne * ( averageGroupHeight + averageSpaceAboveGroups ) - averageSpaceAboveGroups;

    bool canCreateNewColumn = ( currentColumnGroupCount > 0 )  // do not leave empty column
                              && ( currentColumn < targetNumberColumns - 1 ) // must not exceed max number of columns
                              && ( autoPlacedBreaks < numberAutoPlacedBreaks );

    bool shouldCreateNewColumn = currentColumnHeightIfGroupIsIncluded > averageRemainingColumnHeightIncludingThisOne  // current group height is greater than expected group height
                                 && currentColumnGroupCount > 0 // do not leave empty column
                                 && currentColumnHeightIfGroupIsIncluded > maxGroupHeight  // no sense to make smaller columns than max group height
                                 && currentColumnHeightIfGroupIsIncluded > maxColumnHeight; // no sense to make smaller columns than max column already created

    shouldCreateNewColumn |= group.placeColumnBreakBeforeGroup;
    canCreateNewColumn |= group.placeColumnBreakBeforeGroup;

    // also should create a new column if the number of items left < number of columns left
    // in this case we should spread the remaining items out over the remaining columns
    shouldCreateNewColumn |= ( componentGroups.size() - i < targetNumberColumns - currentColumn );

    if ( canCreateNewColumn && shouldCreateNewColumn )
    {
      // New column
      currentColumn++;
      if ( !group.placeColumnBreakBeforeGroup )
        autoPlacedBreaks++;
      currentColumnGroupCount = 0;
      currentColumnHeight = group.size.height();
    }
    else
    {
      currentColumnHeight = currentColumnHeightIfGroupIsIncluded;
    }
    componentGroups[i].column = currentColumn;
    currentColumnGroupCount++;
    maxColumnHeight = std::max( currentColumnHeight, maxColumnHeight );
  }

  auto refineColumns = [&componentGroups, this]() -> bool
  {
    QHash< int, double > columnHeights;
    QHash< int, int > columnGroupCounts;
    double currentColumnHeight = 0;
    int currentColumn = -1;
    int columnCount = 0;
    int groupCount = 0;
    double maxCurrentColumnHeight = 0;
    for ( int i = 0; i < componentGroups.size(); i++ )
    {
      const LegendComponentGroup &group = componentGroups.at( i );
      if ( group.column != currentColumn )
      {
        if ( currentColumn >= 0 )
        {
          columnHeights.insert( currentColumn, currentColumnHeight );
          columnGroupCounts.insert( currentColumn, groupCount );
        }

        currentColumn = group.column;
        currentColumnHeight = 0;
        groupCount = 0;
        columnCount = std::max( columnCount, currentColumn + 1 );
      }

      const double spaceAbove = spaceAboveGroup( group );
      currentColumnHeight += spaceAbove + group.size.height();
      groupCount++;
    }
    columnHeights.insert( currentColumn, currentColumnHeight );
    columnGroupCounts.insert( currentColumn, groupCount );

    double totalColumnHeights = 0;
    for ( int i = 0; i < columnCount; ++ i )
    {
      totalColumnHeights += columnHeights[i];
      maxCurrentColumnHeight = std::max( maxCurrentColumnHeight, columnHeights[i] );
    }

    const double averageColumnHeight = totalColumnHeights / columnCount;

    bool changed = false;
    int nextCandidateColumnForShift = 1;
    for ( int i = 0; i < componentGroups.size(); i++ )
    {
      LegendComponentGroup &group = componentGroups[ i ];
      if ( group.column < nextCandidateColumnForShift )
        continue;

      // try shifting item to previous group
      const bool canShift = !group.placeColumnBreakBeforeGroup
                            && columnGroupCounts[ group.column ] >= 2;

      if ( canShift
           && columnHeights[ group.column - 1 ] < averageColumnHeight
           && ( columnHeights[ group.column - 1 ] + group.size.height() ) * 0.9 < maxCurrentColumnHeight
         )
      {
        group.column -= 1;
        columnHeights[ group.column ] += group.size.height() + spaceAboveGroup( group );
        columnGroupCounts[ group.column ]++;
        columnHeights[ group.column + 1 ] -= group.size.height();
        columnGroupCounts[ group.column + 1]--;
        changed = true;
      }
      else
      {
        nextCandidateColumnForShift = group.column + 1;
      }
    }
    return changed;
  };

  bool wasRefined = true;
  int iterations = 0;
  while ( wasRefined && iterations < 2 )
  {
    wasRefined = refineColumns();
    iterations++;
  }

  // Align labels of symbols for each layer/column to the same labelXOffset
  QMap<QString, qreal> maxSymbolWidth;
  for ( int i = 0; i < componentGroups.size(); i++ )
  {
    LegendComponentGroup &group = componentGroups[i];
    for ( int j = 0; j < group.components.size(); j++ )
    {
      if ( QgsLayerTreeModelLegendNode *legendNode = qobject_cast<QgsLayerTreeModelLegendNode *>( group.components.at( j ).item ) )
      {
        QString key = QStringLiteral( "%1-%2" ).arg( reinterpret_cast< qulonglong >( legendNode->layerNode() ) ).arg( group.column );
        maxSymbolWidth[key] = std::max( group.components.at( j ).symbolSize.width(), maxSymbolWidth[key] );
      }
    }
  }
  for ( int i = 0; i < componentGroups.size(); i++ )
  {
    LegendComponentGroup &group = componentGroups[i];
    for ( int j = 0; j < group.components.size(); j++ )
    {
      if ( QgsLayerTreeModelLegendNode *legendNode = qobject_cast<QgsLayerTreeModelLegendNode *>( group.components.at( j ).item ) )
      {
        QString key = QStringLiteral( "%1-%2" ).arg( reinterpret_cast< qulonglong >( legendNode->layerNode() ) ).arg( group.column );
        double space = mSettings.style( QgsLegendStyle::Symbol ).margin( QgsLegendStyle::Right ) +
                       mSettings.style( QgsLegendStyle::SymbolLabel ).margin( QgsLegendStyle::Left );
        group.components[j].labelXOffset = maxSymbolWidth[key] + space;
        group.components[j].maxSiblingSymbolWidth = maxSymbolWidth[key];
        group.components[j].size.rwidth() = maxSymbolWidth[key] + space + group.components.at( j ).labelSize.width();
      }
    }
  }
  return targetNumberColumns;
}

QSizeF QgsLegendRenderer::drawTitle( QgsRenderContext &context, double top, Qt::AlignmentFlag halignment, double legendWidth ) const
{
  QSizeF size( 0, 0 );
  if ( mSettings.title().isEmpty() )
  {
    return size;
  }

  QStringList lines = mSettings.splitStringForWrapping( mSettings.title() );

  //calculate width and left pos of rectangle to draw text into
  double textBoxWidth;
  double textBoxLeft;
  widthAndOffsetForTitleText( halignment, legendWidth, textBoxWidth, textBoxLeft );

  const QgsTextFormat titleFormat = mSettings.style( QgsLegendStyle::Title ).textFormat();
  const double dotsPerMM = context.scaleFactor();

  double overallTextHeight = 0;
  double overallTextWidth = 0;

  {
    QgsScopedRenderContextScaleToPixels contextToPixels( context );
    overallTextHeight = QgsTextRenderer::textHeight( context, titleFormat, lines, Qgis::TextLayoutMode::Rectangle );
    overallTextWidth = QgsTextRenderer::textWidth( context, titleFormat, lines );
  }

  size.rheight() = overallTextHeight / dotsPerMM;
  size.rwidth() = overallTextWidth / dotsPerMM;

  if ( context.painter() )
  {
    QgsScopedRenderContextScaleToPixels contextToPixels( context );

    const QRectF r( textBoxLeft * dotsPerMM, top * dotsPerMM, textBoxWidth * dotsPerMM, overallTextHeight );

    Qgis::TextHorizontalAlignment halign = halignment == Qt::AlignLeft ? Qgis::TextHorizontalAlignment::Left :
                                           halignment == Qt::AlignRight ? Qgis::TextHorizontalAlignment::Right : Qgis::TextHorizontalAlignment::Center;

    QgsTextRenderer::drawText( r, 0, halign, lines, context, titleFormat );
  }

  return size;
}


double QgsLegendRenderer::spaceAboveGroup( const LegendComponentGroup &group )
{
  if ( group.components.isEmpty() ) return 0;

  LegendComponent component = group.components.first();

  if ( QgsLayerTreeGroup *nodeGroup = qobject_cast<QgsLayerTreeGroup *>( component.item ) )
  {
    return mSettings.style( nodeLegendStyle( nodeGroup ) ).margin( QgsLegendStyle::Top );
  }
  else if ( QgsLayerTreeLayer *nodeLayer = qobject_cast<QgsLayerTreeLayer *>( component.item ) )
  {
    return mSettings.style( nodeLegendStyle( nodeLayer ) ).margin( QgsLegendStyle::Top );
  }
  else if ( qobject_cast<QgsLayerTreeModelLegendNode *>( component.item ) )
  {
    // TODO: use Symbol or SymbolLabel Top margin
    return mSettings.style( QgsLegendStyle::Symbol ).margin( QgsLegendStyle::Top );
  }

  return 0;
}

QSizeF QgsLegendRenderer::drawGroup( const LegendComponentGroup &group, QgsRenderContext &context, ColumnContext columnContext, double top )
{
  bool first = true;
  QSizeF size = QSizeF( group.size );
  double currentY = top;
  for ( const LegendComponent &component : std::as_const( group.components ) )
  {
    if ( QgsLayerTreeGroup *groupItem = qobject_cast<QgsLayerTreeGroup *>( component.item ) )
    {
      QgsLegendStyle::Style s = nodeLegendStyle( groupItem );
      if ( s != QgsLegendStyle::Hidden )
      {
        if ( !first )
        {
          currentY += mSettings.style( s ).margin( QgsLegendStyle::Top );
        }
        QSizeF groupSize;
        ColumnContext columnContextForItem = columnContext;
        double indentWidth =  component.indent;
        if ( s == QgsLegendStyle::Subgroup )
        {
          // Remove indent - the subgroup items should be indented, not the subgroup title
          indentWidth -= mSettings.style( QgsLegendStyle::Subgroup ).indent( );
        }
        else
        {
          // Remove indent - the group items should be indented, not the group title
          indentWidth -= mSettings.style( QgsLegendStyle::Group ).indent( );
        }
        if ( mSettings.style( QgsLegendStyle::SymbolLabel ).alignment() == Qt::AlignLeft )
        {
          columnContextForItem.left += indentWidth;
        }
        if ( mSettings.style( QgsLegendStyle::SymbolLabel ).alignment() == Qt::AlignRight )
        {
          columnContextForItem.right -= indentWidth;
        }
        groupSize = drawGroupTitle( groupItem, context, columnContextForItem, currentY );
        size.rwidth() = std::max( groupSize.width(), size.width() );
      }
    }
    else if ( QgsLayerTreeLayer *layerItem = qobject_cast<QgsLayerTreeLayer *>( component.item ) )
    {
      QgsLegendStyle::Style s = nodeLegendStyle( layerItem );
      if ( s != QgsLegendStyle::Hidden )
      {
        if ( !first )
        {
          currentY += mSettings.style( s ).margin( QgsLegendStyle::Top );
        }
        QSizeF subGroupSize;

        ColumnContext columnContextForItem = columnContext;
        double indentWidth =  component.indent;
        columnContextForItem.left += indentWidth;
        subGroupSize = drawLayerTitle( layerItem, context, columnContextForItem, currentY );
        size.rwidth() = std::max( subGroupSize.width(), size.width() );
      }
    }
    else if ( QgsLayerTreeModelLegendNode *legendNode = qobject_cast<QgsLayerTreeModelLegendNode *>( component.item ) )
    {
      if ( !first )
      {
        currentY += mSettings.style( QgsLegendStyle::Symbol ).margin( QgsLegendStyle::Top );
      }

      ColumnContext columnContextForItem = columnContext;
      double indentWidth = 0;
      indentWidth = component.indent;
      if ( mSettings.style( QgsLegendStyle::SymbolLabel ).alignment() == Qt::AlignLeft )
      {
        columnContextForItem.left += indentWidth;
      }
      if ( mSettings.style( QgsLegendStyle::SymbolLabel ).alignment() == Qt::AlignRight )
      {
        columnContextForItem.right -= indentWidth;
      }

      LegendComponent symbolComponent = drawSymbolItem( legendNode, context, columnContextForItem, currentY, component.maxSiblingSymbolWidth );
      // expand width, it may be wider because of label offsets
      size.rwidth() = std::max( symbolComponent.size.width() + indentWidth, size.width() );
    }
    currentY += component.size.height();
    first = false;
  }
  return size;
}

QgsLegendRenderer::LegendComponent QgsLegendRenderer::drawSymbolItem( QgsLayerTreeModelLegendNode *symbolItem, QgsRenderContext &context, ColumnContext columnContext, double top, double maxSiblingSymbolWidth )
{
  QgsLayerTreeModelLegendNode::ItemContext ctx;
  ctx.context = &context;

  // add a layer expression context scope
  QgsExpressionContextScope *layerScope = nullptr;
  if ( symbolItem->layerNode()->layer() )
  {
    layerScope = QgsExpressionContextUtils::layerScope( symbolItem->layerNode()->layer() );
    context.expressionContext().appendScope( layerScope );
  }

  ctx.painter = context.painter();
  Q_NOWARN_DEPRECATED_PUSH
  ctx.point = QPointF( columnContext.left, top );
  ctx.labelXOffset = maxSiblingSymbolWidth;
  Q_NOWARN_DEPRECATED_POP

  ctx.top = top;

  ctx.columnLeft = columnContext.left;
  ctx.columnRight = columnContext.right;

  switch ( mSettings.symbolAlignment() )
  {
    case Qt::AlignLeft:
    default:
      ctx.columnLeft += mSettings.style( QgsLegendStyle::Symbol ).margin( QgsLegendStyle::Left );
      break;

    case Qt::AlignRight:
      ctx.columnRight -= mSettings.style( QgsLegendStyle::Symbol ).margin( QgsLegendStyle::Left );
      break;
  }

  ctx.maxSiblingSymbolWidth = maxSiblingSymbolWidth;

  QgsExpressionContextScope *symbolScope = nullptr;
  if ( const QgsSymbolLegendNode *symbolNode = dynamic_cast< const QgsSymbolLegendNode * >( symbolItem ) )
  {
    symbolScope = symbolNode->createSymbolScope();
    context.expressionContext().appendScope( symbolScope );
    ctx.patchShape = symbolNode->patchShape();
  }

  ctx.patchSize = symbolItem->userPatchSize();

  QgsLayerTreeModelLegendNode::ItemMetrics im = symbolItem->draw( mSettings, &ctx );

  if ( symbolScope )
    delete context.expressionContext().popScope();

  if ( layerScope )
    delete context.expressionContext().popScope();

  LegendComponent component;
  component.item = symbolItem;
  component.symbolSize = im.symbolSize;
  component.labelSize = im.labelSize;
  //QgsDebugMsgLevel( QStringLiteral( "symbol height = %1 label height = %2").arg( symbolSize.height()).arg( labelSize.height() ), 2);
  // NOTE -- we hard code left/right margins below, because those are the only ones exposed for use currently.
  // ideally we could (should?) expose all these margins as settings, and then adapt the below to respect the current symbol/text alignment
  // and consider the correct margin sides...
  double width = std::max( static_cast< double >( im.symbolSize.width() ), maxSiblingSymbolWidth )
                 + mSettings.style( QgsLegendStyle::Symbol ).margin( QgsLegendStyle::Left )
                 + mSettings.style( QgsLegendStyle::Symbol ).margin( QgsLegendStyle::Right )
                 + mSettings.style( QgsLegendStyle::SymbolLabel ).margin( QgsLegendStyle::Left )
                 + im.labelSize.width();

  double height = std::max( im.symbolSize.height(), im.labelSize.height() );
  component.size = QSizeF( width, height );
  return component;
}

QSizeF QgsLegendRenderer::drawLayerTitle( QgsLayerTreeLayer *nodeLayer, QgsRenderContext &context, ColumnContext columnContext, double top )
{
  QSizeF size( 0, 0 );
  QModelIndex idx = mLegendModel->node2index( nodeLayer );
  QString titleString = mLegendModel->data( idx, Qt::DisplayRole ).toString();
  //Let the user omit the layer title item by having an empty layer title string
  if ( titleString.isEmpty() )
    return size;

  const QgsTextFormat layerFormat = mSettings.style( nodeLegendStyle( nodeLayer ) ).textFormat();

  QgsExpressionContextScope *layerScope = nullptr;
  if ( nodeLayer->layer() )
  {
    layerScope = QgsExpressionContextUtils::layerScope( nodeLayer->layer() );
    context.expressionContext().appendScope( layerScope );
  }

  const QStringList lines = mSettings.evaluateItemText( titleString, context.expressionContext() );

  const double dotsPerMM = context.scaleFactor();

  double overallTextHeight = 0;
  double overallTextWidth = 0;
  {
    QgsScopedRenderContextScaleToPixels contextToPixels( context );
    overallTextHeight = QgsTextRenderer::textHeight( context, layerFormat, lines, Qgis::TextLayoutMode::RectangleAscentBased );
    overallTextWidth = QgsTextRenderer::textWidth( context, layerFormat, lines );
  }
  const double sideMargin = mSettings.style( nodeLegendStyle( nodeLayer ) ).margin( QgsLegendStyle::Left );

  size.rheight() = overallTextHeight / dotsPerMM;
  size.rwidth() = overallTextWidth / dotsPerMM + sideMargin *
                  ( mSettings.style( nodeLegendStyle( nodeLayer ) ).alignment() == Qt::AlignHCenter  ? 2 : 1 );

  if ( context.painter() )
  {
    QgsScopedRenderContextScaleToPixels contextToPixels( context );
    Qgis::TextHorizontalAlignment halign =  mSettings.style( nodeLegendStyle( nodeLayer ) ).alignment()  == Qt::AlignLeft ? Qgis::TextHorizontalAlignment::Left :
                                            mSettings.style( nodeLegendStyle( nodeLayer ) ).alignment()  == Qt::AlignRight ? Qgis::TextHorizontalAlignment::Right : Qgis::TextHorizontalAlignment::Center;

    const QRectF r( ( columnContext.left + ( halign == Qgis::TextHorizontalAlignment::Left ? sideMargin : 0 ) ) * dotsPerMM, top * dotsPerMM,
                    ( ( columnContext.right - columnContext.left ) - ( halign == Qgis::TextHorizontalAlignment::Right ? sideMargin : 0 ) ) * dotsPerMM, overallTextHeight );
    QgsTextRenderer::drawText( r, 0, halign, lines, context, layerFormat );
  }

  size.rheight() += mSettings.style( nodeLegendStyle( nodeLayer ) ).margin( QgsLegendStyle::Side::Bottom );

  if ( layerScope )
    delete context.expressionContext().popScope();

  return size;
}

QSizeF QgsLegendRenderer::drawGroupTitle( QgsLayerTreeGroup *nodeGroup, QgsRenderContext &context, ColumnContext columnContext, double top )
{
  QSizeF size( 0, 0 );
  QModelIndex idx = mLegendModel->node2index( nodeGroup );

  const QgsTextFormat groupFormat = mSettings.style( nodeLegendStyle( nodeGroup ) ).textFormat();

  const QStringList lines = mSettings.evaluateItemText( mLegendModel->data( idx, Qt::DisplayRole ).toString(), context.expressionContext() );

  double overallTextHeight = 0;
  double overallTextWidth = 0;

  {
    QgsScopedRenderContextScaleToPixels contextToPixels( context );
    overallTextHeight = QgsTextRenderer::textHeight( context, groupFormat, lines, Qgis::TextLayoutMode::RectangleAscentBased );
    overallTextWidth = QgsTextRenderer::textWidth( context, groupFormat, lines );
  }

  const double sideMargin = mSettings.style( nodeLegendStyle( nodeGroup ) ).margin( QgsLegendStyle::Left );
  const double dotsPerMM = context.scaleFactor();

  size.rheight() = overallTextHeight / dotsPerMM;
  size.rwidth() = overallTextWidth / dotsPerMM + sideMargin *
                  ( mSettings.style( nodeLegendStyle( nodeGroup ) ).alignment() == Qt::AlignHCenter  ? 2 : 1 );

  if ( context.painter() )
  {
    QgsScopedRenderContextScaleToPixels contextToPixels( context );

    Qgis::TextHorizontalAlignment halign =  mSettings.style( nodeLegendStyle( nodeGroup ) ).alignment()  == Qt::AlignLeft ? Qgis::TextHorizontalAlignment::Left :
                                            mSettings.style( nodeLegendStyle( nodeGroup ) ).alignment()  == Qt::AlignRight ? Qgis::TextHorizontalAlignment::Right : Qgis::TextHorizontalAlignment::Center;

    const QRectF r( dotsPerMM * ( columnContext.left + ( halign == Qgis::TextHorizontalAlignment::Left ? sideMargin : 0 ) ), top * dotsPerMM,
                    dotsPerMM * ( ( columnContext.right - columnContext.left ) - ( halign == Qgis::TextHorizontalAlignment::Right ? sideMargin : 0 ) ), overallTextHeight );
    QgsTextRenderer::drawText( r, 0, halign, lines, context, groupFormat );
  }

  size.rheight() += mSettings.style( nodeLegendStyle( nodeGroup ) ).margin( QgsLegendStyle::Bottom );
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

void QgsLegendRenderer::drawLegend( QgsRenderContext &context )
{
  paintAndDetermineSize( context );
}

