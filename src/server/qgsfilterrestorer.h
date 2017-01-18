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

#include "qgsaccesscontrol.h"
#include "qgis_server.h"

#include <QHash>

class QgsMapLayer;

/** RAII class to restore layer filters on destruction
 */
class SERVER_EXPORT QgsOWSServerFilterRestorer
{
  public:

    QgsOWSServerFilterRestorer( const QgsAccessControl* accessControl )
        : mAccessControl( accessControl )
    {}

    //! Destructor. When object is destroyed all original layer filters will be restored.
    ~QgsOWSServerFilterRestorer()
    {
      restoreLayerFilters( mOriginalLayerFilters );
    }

    void restoreLayerFilters( const QHash<QgsMapLayer*, QString>& filterMap );

    /** Returns a reference to the object's hash of layers to original subsetString filters.
     * Original layer subsetString filters MUST be inserted into this hash before modifying them.
     */
    QHash<QgsMapLayer*, QString>& originalFilters() { return mOriginalLayerFilters; }

    //! Apply filter from AccessControl
    //XXX May be this method should be owned QgsAccessControl
    static void applyAccessControlLayerFilters( const QgsAccessControl* accessControl, QgsMapLayer* mapLayer,
        QHash<QgsMapLayer*, QString>& originalLayerFilters );

  private:
    const QgsAccessControl* mAccessControl;
    QHash<QgsMapLayer*, QString> mOriginalLayerFilters;

    QgsOWSServerFilterRestorer( const QgsOWSServerFilterRestorer& rh );
    QgsOWSServerFilterRestorer& operator=( const QgsOWSServerFilterRestorer& rh );
};

#endif // QGSFILTERRESTORER_H

