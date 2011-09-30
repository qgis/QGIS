/***************************************************************************
 *   Copyright (C) 2011 by Sergey Yakushev                                 *
 *   yakushevs@list.ru                                                     *
 *                                                                         *
 *   This is file define vrp plugins time, distance and speed units        *
 *   classes                                                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef ROADGRAPH_SPEEDPROPERTER_H
#define ROADGRAPH_SPEEDPROPERTER_H

#include <qgsarcproperter.h>

class RgSpeedProperter : public QgsArcProperter
{
  public:
    RgSpeedProperter( int attributeId, double defaultValue, double toMetricFactor );

    QVariant property( double distance, const QgsFeature& f ) const;

    QgsAttributeList requiredAttributes() const;
  private:
    int mAttributeId;
    double mDefaultValue;
    double mToMetricFactor;

};

#endif
