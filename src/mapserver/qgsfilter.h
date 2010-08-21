/***************************************************************************
                              qgsfilter.h    
                              -----------
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
#ifndef QGSFILTER_H
#define QGSFILTER_H 

#include <qgsfeature.h>
#include <QList>

class QDomElement;
class QgsVectorLayer;

/**Abstraction of an OGC filter for features*/
class QgsFilter
{
 public:
  QgsFilter();
  QgsFilter(int propertyIndex);
  virtual ~QgsFilter();
  /**Evaluates a feature against the filter.
   @return true if the filter applies for the feature*/
  virtual bool evaluate(const QgsFeature& f) const = 0;

  /**Creates a filter instance from xml.
     The calling function takes ownership of the created filter.
     @param filterElem the element containing the filter name (that is the first child below the <Filter> element)
     @return the filter or 0 in case of failure*/
  static QgsFilter* createFilterFromXml(const QDomElement& filterElem, QgsVectorLayer* vl);

  /**Returns the attribute indices needed for the filter*/
  virtual QList<int> attributeIndices() const;

 protected:
  /**Returns the QVariant on mPropertyIndex position*/
  QVariant propertyIndexValue(const QgsFeature& f) const;
  
  /**Index of the property (attribute) to evaluate*/
  int mPropertyIndex;
};

#endif //QGSFILTER_H
