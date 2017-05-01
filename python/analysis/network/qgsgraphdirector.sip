/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/analysis/network/qgsgraphdirector.h                              *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





%ModuleHeaderCode
%Docstring
*************************************************************************
qgsgraphdirector.h
--------------------------------------
Date                 : 2010-10-18
Copyright            : (C) 2010 by Yakushev Sergey
Email                : YakushevS <at> list.ru
***************************************************************************
                                                                          *
   This program is free software; you can redistribute it and/or modify   *
   it under the terms of the GNU General Public License as published by   *
   the Free Software Foundation; either version 2 of the License, or      *
   (at your option) any later version.                                    *
                                                                          *
**************************************************************************
%End
#include <qgsvectorlayerdirector.h>
%End

class QgsGraphDirector : QObject
{
%Docstring
 Determine making the graph. QgsGraphBuilder and QgsGraphDirector implemented
 using "builder" design patter.
%End

%TypeHeaderCode
#include "qgsgraphdirector.h"
%End

%ConvertToSubClassCode
    if ( dynamic_cast< QgsVectorLayerDirector * >( sipCpp ) != NULL )
      sipType = sipType_QgsVectorLayerDirector;
    else
      sipType = NULL;
%End
  public:

    virtual ~QgsGraphDirector();

    virtual void makeGraph( QgsGraphBuilderInterface *builder,
                            const QVector< QgsPoint > &additionalPoints,
                            QVector< QgsPoint > &snappedPoints /Out/ ) const;
%Docstring
 Make a graph using QgsGraphBuilder

 \param builder the graph builder
 \param additionalPoints list of points that should be snapped to the graph
 \param snappedPoints list of snapped points
.. note::

   if snappedPoints[i] == QgsPoint(0.0,0.0) then snapping failed.
%End

    void addStrategy( QgsNetworkStrategy *prop /Transfer/ );
%Docstring
Add optimization strategy
%End

    virtual QString name() const = 0;
%Docstring
Returns director name
 :rtype: str
%End

  protected:
};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/analysis/network/qgsgraphdirector.h                              *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
