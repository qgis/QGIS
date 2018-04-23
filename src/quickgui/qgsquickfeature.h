/***************************************************************************
  qgsquickfeature.h
 ---------------------
  Date                 : Nov 2017
  Copyright            : (C) 2017 by Peter Petrik
  Email                : zilolv at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSQUICKFEATURE_H
#define QGSQUICKFEATURE_H

#include <QObject>

#include "qgsfeature.h"

#include "qgis_quick.h"

class QgsVectorLayer;

/**
 * \ingroup quick
 * Helper class for QgsFeature and QgsVectorLayer where it belongs.
 *
 * \note QML Type: IdentifyResult
 *
 * \since QGIS 3.2
 */
class QUICK_EXPORT QgsQuickFeature
{
    Q_GADGET

    //! feature
    Q_PROPERTY( QgsVectorLayer *layer READ layer )
    //! layer to which \a feature belogs
    Q_PROPERTY( QgsFeature feature READ feature )
    //! whether is feature valid
    Q_PROPERTY( bool valid READ valid )

  public:
    //! create new feature
    QgsQuickFeature();

    //! create new feature
    QgsQuickFeature( const QgsFeature &feature,
                     QgsVectorLayer *layer );

    //! Return layer
    QgsVectorLayer *layer() const;

    //! Return feature
    QgsFeature feature() const;

    //! Return whether is feature valid
    bool valid() const;

  private:
    QgsVectorLayer *mLayer;
    QgsFeature mFeature;
};

Q_DECLARE_METATYPE( QgsQuickFeature )

#endif // QGSQUICKFEATURE_H
