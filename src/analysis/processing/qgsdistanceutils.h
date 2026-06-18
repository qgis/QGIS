/***************************************************************************
                         qgsdistanceutils.cpp
                         ---------------------
    begin                : June 2026
    copyright            : (C) 2026 by Alexander Bruy
    email                : alexander dot bruy at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSDISTANCEUTILS_H
#define QGSDISTANCEUTILS_H

#include <utility>
#include <vector>

#include "qgis_analysis.h"
#include "qgis_sip.h"
#include "qgsfeatureid.h"
#include "qgspointxy.h"

#define SIP_NO_FILE

class QgsDistanceArea;
class QgsProcessingFeedback;

///@cond PRIVATE

namespace QgsDistanceUtils
{
  struct ANALYSIS_EXPORT NeighborResult
  {
      QgsFeatureId featureId;
      double distance;
      QgsPointXY point;
  };

  /**
   * Calculates the k-nearest neighbors using ellipsoidal or Cartesian calculations via a QgsDistanceArea object.
   * \param sourcePoint The point from which distances are measured.
   * \param targetPoints A list of target feature IDs and their point geometries.
   * \param da Configured QgsDistanceArea object for measuring lines.
   * \param k The maximum number of nearest neighbors to return. If k <= 0, all targets are evaluated.
   * \param feedback Optional feedback object for cancellation and progress reporting.
   * \returns A vector of NeighborResults sorted by distance in ascending order.
   */
  ANALYSIS_EXPORT std::vector<QgsDistanceUtils::NeighborResult> nearestNeighbors(
    const QgsPointXY &sourcePoint, const std::vector<std::pair<QgsFeatureId, QgsPointXY>> &targetPoints, const QgsDistanceArea &da, long long k, QgsProcessingFeedback *feedback = nullptr
  );
} //namespace QgsDistanceUtils

///@endcond PRIVATE

#endif // QGSDISTANCEUTILS_H
