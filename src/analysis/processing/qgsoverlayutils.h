/***************************************************************************
  qgsoverlayutils.h
  ---------------------
  Date                 : April 2018
  Copyright            : (C) 2018 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSOVERLAYUTILS_H
#define QGSOVERLAYUTILS_H

#include <QList>
#include "qgswkbtypes.h"

#define SIP_NO_FILE

///@cond PRIVATE

class QgsFeatureSource;
class QgsFeatureSink;
class QgsFields;
class QgsProcessingContext;
class QgsProcessingFeedback;
class QgsGeometry;

namespace QgsOverlayUtils
{

  //! how to write out attributes of features after overlay operation
  enum DifferenceOutput
  {
    OutputA,   //!< Write only attributes of the first layer
    OutputAB,  //!< Write attributes of both layers
    OutputBA,  //!< Write attributes of both layers, inverted (first attributes of B, then attributes of A)
  };

  void difference( const QgsFeatureSource &sourceA, const QgsFeatureSource &sourceB, QgsFeatureSink &sink, QgsProcessingContext &context, QgsProcessingFeedback *feedback, int &count, int totalCount, DifferenceOutput outputAttrs );

  void intersection( const QgsFeatureSource &sourceA, const QgsFeatureSource &sourceB, QgsFeatureSink &sink, QgsProcessingContext &context, QgsProcessingFeedback *feedback, int &count, int totalCount, const QList<int> &fieldIndicesA, const QList<int> &fieldIndicesB );

  //! Makes sure that what came out from intersection of two geometries is good to be used in the output
  bool sanitizeIntersectionResult( QgsGeometry &geom, QgsWkbTypes::GeometryType geometryType );

  /**
   * Copies features from the source to the sink and resolves overlaps: for each pair of overlapping features A and B
   * it will produce:
   *
   * # a feature with geometry A - B with A's attributes
   * # a feature with geometry B - A with B's attributes
   * # two features with geometry intersection(A, B) - one with A's attributes, one with B's attributes.
   *
   * As a result, for all pairs of features in the output, a pair either has no common interior or their interior is the same.
   */
  void resolveOverlaps( const QgsFeatureSource &source, QgsFeatureSink &sink, QgsProcessingFeedback *feedback );
}

///@endcond PRIVATE

#endif // QGSOVERLAYUTILS_H
