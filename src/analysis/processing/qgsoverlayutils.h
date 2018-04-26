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

#define SIP_NO_FILE

///@cond PRIVATE

class QgsFeatureSource;
class QgsFeatureSink;
class QgsFields;
class QgsProcessingContext;
class QgsProcessingFeedback;

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

}

///@endcond PRIVATE

#endif // QGSOVERLAYUTILS_H
