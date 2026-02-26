/***************************************************************************
  qgsfeedback.cpp
  --------------------------------------
  Date                 : December 2025
  Copyright            : (C) 2025 by Even Rouault
  Email                : event dot rouault at spatialys dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsfeedback.h"

#include "moc_qgsfeedback.cpp"

QgsFeedback::~QgsFeedback() = default;

/* static */
std::unique_ptr<QgsFeedback> QgsFeedback::createScaledFeedback(
  QgsFeedback *parentFeedback, double startPercentage, double endPercentage )
{
  auto scaledFeedback = std::make_unique<QgsFeedback>();
  if ( parentFeedback )
  {
    const double ratio = ( endPercentage - startPercentage ) / 100.0;
    QObject::connect( scaledFeedback.get(), &QgsFeedback::progressChanged,
                      parentFeedback, [parentFeedback, startPercentage, ratio]( double progress )
    {
      parentFeedback->setProgress( startPercentage + progress * ratio );
    } );
    QObject::connect( scaledFeedback.get(), &QgsFeedback::canceled,
                      parentFeedback, &QgsFeedback::cancel );
  }
  return scaledFeedback;
}
