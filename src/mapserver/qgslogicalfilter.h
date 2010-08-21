/***************************************************************************
                              qgslogicalfilter.h
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

#ifndef QGSLOGICALFILTER_H
#define QGSLOGICALFILTER_H

#include "qgsfilter.h"

/**OGC Filter for 'AND'/ 'OR'.
 The class uses two filterobjects for evaluation. It is also possible to nest logical filters*/
class QgsLogicalFilter: public QgsFilter
{
  public:
    enum FILTER_TYPE
    {
      AND,
      OR,
      NOT,
      UNKNOWN
    };

    QgsLogicalFilter();
    QgsLogicalFilter( FILTER_TYPE t, QgsFilter* filter1, QgsFilter* filter2 );
    ~QgsLogicalFilter();

    /**Adds first sub-filter. This class takes ownership of the passed object*/
    void addFilter1( QgsFilter* filter );
    /**Adds second sub-filter. This class takes ownership of the passed object*/
    void addFilter2( QgsFilter* filter );

    /**Evaluates a feature against the filter.
     @return true if the filter applies for the feature*/
    bool evaluate( const QgsFeature& f ) const;

    /**Returns the attribute indices needed for the filter*/
    QList<int> attributeIndices() const;

  private:
    const QgsFilter* mFilter1;
    const QgsFilter* mFilter2;
    FILTER_TYPE mFilterType;
};

#endif //QGSLOGICALFILTER_H
