/***************************************************************************
    qgsrasterpipe.h - Internal raster processing modules interface
     --------------------------------------
    Date                 : Jun 21, 2012
    Copyright            : (C) 2012 by Radim Blazek
    email                : radim dot blazek at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSRASTERPIPE_H
#define QGSRASTERPIPE_H

#include <QObject>
#include <QImage>

#include "qgsrectangle.h"
#include "qgsrasterinterface.h"
#include "qgsrasterresamplefilter.h"
#include "qgsrasterdataprovider.h"
#include "qgsrasterrenderer.h"
#include "qgsrasterprojector.h"

/** \ingroup core
 * Base class for processing modules.
 */
class CORE_EXPORT QgsRasterPipe //: public QObject
{
    //Q_OBJECT

  public:
    // Role of known filters
    enum Role
    {
      UnknownRole   = 0,
      ProviderRole  = 1,
      RendererRole  = 2,
      ResamplerRole = 3,
      ProjectorRole = 4
    };

    QgsRasterPipe( );

    virtual ~QgsRasterPipe();

    /** \brief Try to connect filters in pipe and to the provider at beginning.
        Returns true if connected or false if connection failed */
    bool connectFilters ( QVector<QgsRasterInterface*> theFilters );


    /** Try to insert filter at specified index and connect
     * if connection would fail, the filter is not inserted and false is returned */
    bool insert ( int idx, QgsRasterInterface* theFilter ); 

    /** Try to replace filter at specified index and connect
     * if connection would fail, the filter is not inserted and false is returned */
    bool replace ( int idx, QgsRasterInterface* theFilter ); 

    /** Insert a new filter in prefered place or replace similar filter if it 
     *  already exists */
    bool setFilter ( QgsRasterInterface * theFilter );

    QgsRasterInterface * filter ( Role role ) const;

    int size() { return mFilters.size(); }
    QgsRasterInterface * at( int idx ) { return mFilters.at(idx); }
    QgsRasterInterface * last() { return mFilters.last(); }

    // Getters for special types of interfaces
    QgsRasterDataProvider * provider() const;
    QgsRasterRenderer * renderer() const;
    QgsRasterResampleFilter * resampleFilter() const;
    QgsRasterProjector * projector() const;

  private:
    /** Get known parent type_info of interface parent */ 
    Role filterRole ( QgsRasterInterface * filter ) const;

    // Filters in pipe, the first is always provider
    QVector<QgsRasterInterface*> mFilters;

    // Special types of interfaces
    QgsRasterDataProvider * mProvider;
    QgsRasterRenderer * mRenderer;
    QgsRasterResampleFilter * mResampleFilter;
    QgsRasterProjector * mProjector;

    QMap<Role,int> mRoleMap;

    // Set role in mFiltersMap
    void setRole ( QgsRasterInterface * theFilter, int idx );
};

#endif


