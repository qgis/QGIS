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
class CORE_EXPORT QgsRasterPipe
{
  public:
    // Role of known interfaces
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

    /** \brief Try to connect interfaces in pipe and to the provider at beginning.
        Returns true if connected or false if connection failed */
    bool connect( QVector<QgsRasterInterface*> theInterfaces );

    /** Try to insert interface at specified index and connect
     * if connection would fail, the interface is not inserted and false is returned */
    bool insert( int idx, QgsRasterInterface* theInterface );

    /** Try to replace interface at specified index and connect
     * if connection would fail, the interface is not inserted and false is returned */
    bool replace( int idx, QgsRasterInterface* theInterface );

    /** Insert a new known interface in default place or replace interface of the same
     * role if it already exists. Known interfaces are: QgsRasterDataProvider,
     * QgsRasterRenderer, QgsRasterResampleFilter, QgsRasterProjector and their
     * subclasses. For unknown interfaces it mus be explicitly specified position
     * where it should be inserted using insert() method.
     */
    bool set( QgsRasterInterface * theInterface );

    /** Get known interface by role */
    QgsRasterInterface * interface( Role role ) const;

    /** Remove and delete interface at given index if possible */
    bool remove( int idx );

    /** Remove and delete interface from pipe if possible */
    bool remove( QgsRasterInterface * theInterface );

    int size() { return mInterfaces.size(); }
    QgsRasterInterface * at( int idx ) { return mInterfaces.at( idx ); }
    QgsRasterInterface * last() { return mInterfaces.last(); }

    // Getters for special types of interfaces
    QgsRasterDataProvider * provider() const;
    QgsRasterRenderer * renderer() const;
    QgsRasterResampleFilter * resampleFilter() const;
    QgsRasterProjector * projector() const;

  private:
    /** Get known parent type_info of interface parent */
    Role interfaceRole( QgsRasterInterface * interface ) const;

    // Interfaces in pipe, the first is always provider
    QVector<QgsRasterInterface*> mInterfaces;

    QMap<Role, int> mRoleMap;

    // Set role in mRoleMap
    void setRole( QgsRasterInterface * theInterface, int idx );

    // Unset role in mRoleMap
    void unsetRole( QgsRasterInterface * theInterface );
};

#endif


