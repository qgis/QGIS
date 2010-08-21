/***************************************************************************
                              qgsbetweenfilter.h    
                              -------------------
  begin                : Feb 07, 2008
  copyright            : (C) 2008 by Marco Hugentobler
  email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSBETWEENFILTER_H
#define QGSBETWEENFILTER_H

#include "qgsfilter.h"

/**Implements OGC between filter. Note that lower and upper boundary values are inclusive
Sample xml fragment:
<Filter xmlns="http://www.opengis.net/ogc">
<PropertyIsBetween>
<PropertyName>LENGTH</PropertyName>
<LowerBoundary>
<Literal>1000</Literal>
</LowerBoundary>
<UpperBoundary>
<Literal>2000</Literal>
</UpperBoundary>
</PropertyIsBetween>
</Filter>
*/
class QgsBetweenFilter: public QgsFilter
{
 public:
  QgsBetweenFilter();
  QgsBetweenFilter(int attributeIndex, const QString& lowerValue, const QString& upperValue);
  ~QgsBetweenFilter();
   
  /**Evaluates a feature against the filter.
   @return true if the filter applies for the feature*/
  bool evaluate(const QgsFeature& f) const;
  QString lowerValue() const {return mLowerValue;}
  QString upperValue() const {return mUpperValue;}
  void setLowerValue(const QString& lv){mLowerValue = lv;}
  void setUpperValue(const QString& uv){mUpperValue = uv;}

 private:
  /**Lower boundary*/
  QString mLowerValue;
  /**Upper boundary*/
  QString mUpperValue;
};

#endif //QGSBETWEENFILTER_H
