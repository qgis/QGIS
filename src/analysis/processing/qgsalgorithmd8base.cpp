/***************************************************************************
                         qgsalgorithmd8base.cpp
                         ---------------------
    begin                : September 2026
    copyright            : (C) 2026 by Nyall Dawson
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

#include "qgsalgorithmd8base.h"

#include "qgsacademicreference.h"
#include "qgsrasteranalysisutils.h"
#include "qgsrasterfilewriter.h"

#include <QString>

using namespace Qt::StringLiterals;

///@cond PRIVATE
QString QgsD8AnalysisAlgorithmBase::group() const
{
  return QObject::tr( "Raster terrain analysis" );
}

QString QgsD8AnalysisAlgorithmBase::groupId() const
{
  return u"rasterterrainanalysis"_s;
}

QList<QgsAcademicReference> QgsD8AnalysisAlgorithmBase::academicReferences() const
{
  return {
    QgsAcademicReference::createJournalArticle( { u"Strahler, A. N."_s }, 1957, u"Quantitative analysis of watershed geomorphology"_s, u"Eos, Transactions American Geophysical Union"_s, u"38"_s, u"6"_s, u"913-920"_s ),
    QgsAcademicReference::
      createJournalArticle( { u"O'Callaghan, J. F."_s, u"Mark, D. M."_s }, 1984, u"The extraction of drainage networks from digital elevation data"_s, u"Computer Vision, Graphics and Image Processing"_s, u"28"_s, QString(), u"323-344"_s )
  };
}

QList<QgsProcessingAlgorithm::ExternalLink> QgsD8AnalysisAlgorithmBase::externalLinks() const
{
  return {
    QgsProcessingAlgorithm::ExternalLink { QObject::tr( "SAGA tool source code" ), u"https://sourceforge.net/p/saga-gis/code/ci/33d1062b7120c696c9dd258378c48d86dc33560c/tree/saga-gis/src/tools/terrain_analysis/ta_channels/D8_Flow_Analysis.cpp"_s }
  };
}

void QgsD8AnalysisAlgorithmBase::computeStrahlerOrder(
  const QgsRasterBlock *demBlock, const std::vector<int8_t> &d8Directions, int width, int height, int threshold, int16_t *outOrder, QgsProcessingFeedback *feedback, int outputNoData
)
{
  const std::size_t totalCells = static_cast<std::size_t>( width ) * height;
  std::fill_n( outOrder, totalCells, static_cast<int16_t>( 0 ) );

  // see CStack in SAGA's CD8_Flow_Analysis
  struct StackFrame
  {
      int column;
      int row;
      int dir;
      int maxOrderCount;
      int maxOrder;
  };

  std::vector<StackFrame> stack;
  stack.reserve( 1024 );

  for ( int row = 0; row < height; ++row )
  {
    if ( feedback->isCanceled() )
      return;

    feedback->setProgress( 100.0 * static_cast<double>( row ) / height );

    const qgssize rowOffset = static_cast<qgssize>( row ) * width;
    for ( int column = 0; column < width; ++column )
    {
      const qgssize startIdx = rowOffset + column;
      if ( demBlock->isNoData( row, column ) || outOrder[startIdx] >= 1 )
        continue;

      // stack will always be empty here -- so just like SAGA's CD8_Flow_Analysis::Get_Order, we start
      // with an empty stack for each unique pixel. But reusing the same stack object here avoids
      // reallocating memory for every pixel...
      stack.push_back( { .column = column, .row = row, .dir = 0, .maxOrderCount = 0, .maxOrder = 1 } );

      while ( !stack.empty() )
      {
        StackFrame &current = stack.back();
        const qgssize currentIdx = static_cast<qgssize>( current.row ) * width + current.column;

        bool pushedChild = false;
        for ( ; current.dir < 8; )
        {
          const int oppositeDir = ( current.dir + 4 ) % 8;
          int neighborColumn = 0;
          int neighborRow = 0;
          if ( QgsRasterAnalysisUtils::neighborCellCoordinates( oppositeDir, current.row, current.column, neighborRow, neighborColumn, height, width ) )
          {
            const qgssize neighborIdx = static_cast<qgssize>( neighborRow ) * width + neighborColumn;
            if ( d8Directions[neighborIdx] == current.dir )
            {
              const int16_t neighborOrder = outOrder[neighborIdx];
              if ( neighborOrder < 1 )
              {
                // neighbour has not been processed yet!
                stack.push_back( { neighborColumn, neighborRow, 0, 0, 1 } );
                pushedChild = true;
                break;
              }
              else
              {
                if ( current.maxOrder < neighborOrder )
                {
                  current.maxOrder = neighborOrder;
                  current.maxOrderCount = 1;
                }
                else if ( current.maxOrder == neighborOrder )
                {
                  current.maxOrderCount++;
                }
              }
            }
          }
          current.dir++;
        }

        if ( !pushedChild )
        {
          int finalOrder = current.maxOrder;
          if ( current.maxOrderCount > 1 )
          {
            finalOrder++;
          }
          outOrder[currentIdx] = static_cast<int16_t>( finalOrder );
          stack.pop_back();
        }
      }
    }
  }

  if ( threshold > 1 )
  {
    const int shift = 1 - threshold;
    for ( std::size_t i = 0; i < totalCells; ++i )
    {
      if ( outOrder[i] >= threshold )
      {
        outOrder[i] += static_cast<int16_t>( shift );
      }
      else
      {
        outOrder[i] = outputNoData;
      }
    }
  }
  else
  {
    for ( std::size_t i = 0; i < totalCells; ++i )
    {
      if ( outOrder[i] < 1 )
      {
        outOrder[i] = outputNoData;
      }
    }
  }
}

///@endcond
