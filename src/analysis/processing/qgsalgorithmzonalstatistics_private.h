/***************************************************************************
                         qgsalgorithmzonalstatistics.cpp
                         ---------------------
    begin                : December 2019
    copyright            : (C) 2019 by Alexander Bruy
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

#ifndef QGSALGORITHMZONALSTATISTICS_PRIVATE_H
#define QGSALGORITHMZONALSTATISTICS_PRIVATE_H

///@cond PRIVATE

#include "qgis.h"

const std::vector<Qgis::ZonalStatistic> STATS {
  Qgis::ZonalStatistic::Count,
  Qgis::ZonalStatistic::Sum,
  Qgis::ZonalStatistic::Mean,
  Qgis::ZonalStatistic::Median,
  Qgis::ZonalStatistic::StDev,
  Qgis::ZonalStatistic::Min,
  Qgis::ZonalStatistic::Max,
  Qgis::ZonalStatistic::Range,
  Qgis::ZonalStatistic::Minority,
  Qgis::ZonalStatistic::Majority,
  Qgis::ZonalStatistic::Variety,
  Qgis::ZonalStatistic::Variance,
};

///@endcond

#endif
