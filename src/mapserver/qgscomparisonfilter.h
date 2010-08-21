/***************************************************************************
                              qgscomparisonfilter.h    
                              ---------------------
  begin                : Jan 31, 2008
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

#ifndef QGSCOMPARISONFILTER_H
#define QGSCOMPARISONFILTER_H

#include <qgsfilter.h>

/**A filter for the comparison operators <,<=,>,>=,!=,= 
Sample xml fragment:
<Filter xmlns="http://www.opengis.net/ogc">
<PropertyIsLessThan>
<PropertyName>LENGTH</PropertyName>
<Literal>1000</Literal>
</PropertyIsLessThan>
</Filter>
*/
class QgsComparisonFilter: public QgsFilter
{
 public:
  enum COMPARISON_TYPE
    {
      EQUAL,
      NOT_EQUAL,
      LESSER,
      GREATER,
      LESSER_OR_EQUAL,
      GREATER_OR_EQUAL,
      UNKNOWN
    };
  
  QgsComparisonFilter();
  /**Constructor that takes index of the feature attribute, type of comparison \
   and reference value to compare against*/
  QgsComparisonFilter(int propertyIndex, COMPARISON_TYPE ct, QString value);
  ~QgsComparisonFilter();

  /**Evaluates a feature against the filter.
   @return true if the filter applies for the feature*/
  bool evaluate(const QgsFeature& f) const;
  
  //setters and getters
  COMPARISON_TYPE comparisonType() const {return mComparisonType;}
  void setComparisonType(COMPARISON_TYPE t){mComparisonType = t;}

 private:
  COMPARISON_TYPE mComparisonType;
  QString mComparisonValue;
};

#endif //QGSCOMPARISONFILTER_H
