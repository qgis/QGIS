/***************************************************************************
                              qgsbetweenfilter.cpp    
                              --------------------
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

#include "qgsbetweenfilter.h"

QgsBetweenFilter::QgsBetweenFilter(): QgsFilter(-1)
{
}
  
QgsBetweenFilter::QgsBetweenFilter(int attributeIndex, const QString& lowerValue, const QString& upperValue): QgsFilter(attributeIndex), mLowerValue(lowerValue), mUpperValue(upperValue)
{
}
  
QgsBetweenFilter::~QgsBetweenFilter()
{
}

bool QgsBetweenFilter::evaluate(const QgsFeature& f) const
{
  QVariant propertyValue = propertyIndexValue(f);
  
  //numeric or alphanumeric?
  bool numeric = true;
  if (propertyValue.type() == QVariant::String)
    {
      numeric = false;
    }
  
  if(numeric)
    {
      return (propertyValue.toDouble() >= mLowerValue.toDouble() && propertyValue.toDouble() <= mUpperValue.toDouble());
    }
  else
    {
      return (propertyValue.toString() >= mLowerValue && propertyValue.toString() <= mUpperValue);
    }
}
