/***************************************************************************
                              qgsfilterrestorer.h
                              --------------
  begin                : March 24, 2014
  copyright            : (C) 2006 by Marco Hugentobler
  email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSFILTERRESTORER_H
#define QGSFILTERRESTORER_H

#define SIP_NO_FILE


#include "qgis_server.h"

#include <QHash>

class QgsMapLayer;
class QgsAccessControl;

/**
 * \ingroup server
 * \brief RAII class to restore layer filters on destruction
 */
class SERVER_EXPORT QgsOWSServerFilterRestorer
{
  public:
    /**
     * Default constructor for QgsOWSServerFilterRestorer.
     */
    QgsOWSServerFilterRestorer() = default;

    //! Destructor. When object is destroyed all original layer filters will be restored.
    ~QgsOWSServerFilterRestorer()
    {
      restoreLayerFilters( mOriginalLayerFilters );
    }

    QgsOWSServerFilterRestorer( const QgsOWSServerFilterRestorer &rh ) = delete;
    QgsOWSServerFilterRestorer &operator=( const QgsOWSServerFilterRestorer &rh ) = delete;

    void restoreLayerFilters( const QHash<QgsMapLayer *, QString> &filterMap );

    /**
     * Returns a reference to the object's hash of layers to original subsetString filters.
     * Original layer subsetString filters MUST be inserted into this hash before modifying them.
     */
    QHash<QgsMapLayer *, QString> &originalFilters() { return mOriginalLayerFilters; }

    //! Apply filter from AccessControl
    //XXX May be this method should be owned QgsAccessControl
    static void applyAccessControlLayerFilters( const QgsAccessControl *accessControl, QgsMapLayer *mapLayer, QHash<QgsMapLayer *, QString> &originalLayerFilters );

    /**
     * Applies filters from access control on layer.
     * \param accessControl The access control instance
     * \param mapLayer The layer on which the filter has to be applied
     */
    static void applyAccessControlLayerFilters( const QgsAccessControl *accessControl, QgsMapLayer *mapLayer );

  private:
    QHash<QgsMapLayer *, QString> mOriginalLayerFilters;
};

#endif // QGSFILTERRESTORER_H
