/***************************************************************************
                              qgscomparisonfilter.cpp    
                              -----------------------
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

#include "qgscomparisonfilter.h"

QgsComparisonFilter::QgsComparisonFilter(): QgsFilter(), mComparisonType(QgsComparisonFilter::UNKNOWN)
{
}
 
QgsComparisonFilter::QgsComparisonFilter(int propertyIndex, COMPARISON_TYPE ct, QString value): QgsFilter(propertyIndex), mComparisonType(ct), mComparisonValue(value)
{
}
  
QgsComparisonFilter::~QgsComparisonFilter()
{
}

bool QgsComparisonFilter::evaluate(const QgsFeature& f) const
{
  QVariant propertyValue = propertyIndexValue(f);
  
  if(!propertyValue.isValid())
    {
      return false;
    }

  bool numeric = true;
  if (propertyValue.type() == QVariant::String)
    {
      numeric = false;
    }

  switch(mComparisonType)
    {
    case EQUAL:
      if(numeric)
	{
	  return (propertyValue.toDouble() == mComparisonValue.toDouble());
	}
      else
	{
	  return (propertyValue.toString() == mComparisonValue);
	}
      break;
    case NOT_EQUAL:
      if(numeric)
	{
	  return (propertyValue.toDouble() != mComparisonValue.toDouble());
	}
      else
	{
	  return (propertyValue.toString() != mComparisonValue);
	}
      break;
    case LESSER:
      if(numeric)
	{
	  return (propertyValue.toDouble() < mComparisonValue.toDouble());
	}
      else
	{
	  return (propertyValue.toString() < mComparisonValue); 
	}
      break;
    case GREATER:
      if(numeric)
	{
	  return (propertyValue.toDouble() > mComparisonValue.toDouble());
	}
      else
	{
	  return (propertyValue.toString() > mComparisonValue); 
	}
      break;
    case LESSER_OR_EQUAL:
      if(numeric)
	{
	  return (propertyValue.toDouble() <= mComparisonValue.toDouble());
	}
      else
	{
	  return (propertyValue.toString() <= mComparisonValue); 
	}
      break;
    case GREATER_OR_EQUAL:
      if(numeric)
	{
	  return (propertyValue.toDouble() >= mComparisonValue.toDouble());
	}
      else
	{
	  return (propertyValue.toString() >= mComparisonValue); 
	}
      break;
    case UNKNOWN:
    default:
      return false;
    }
  return false; //soon...
}
