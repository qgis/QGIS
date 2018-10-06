/***************************************************************************
                              qgslayerrestorer.h
                              -------------------
  begin                : April 24, 2017
  copyright            : (C) 2017 by Paul Blottiere
  email                : paul.blottiere@oslandia.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLAYERRESTORER_H
#define QGSLAYERRESTORER_H

#include <QList>
#include <QDomDocument>

#include "qgsfeatureid.h"

class QgsMapLayer;

/**
 * \ingroup server
 * RAII class to restore layer configuration on destruction (opacity,
 * filters, ...)
 * \since QGIS 3.0
 */
class QgsLayerRestorer
{
  public:

    /**
     * Constructor for QgsLayerRestorer.
     * \param layers List of layers to restore in their initial states
     */
    QgsLayerRestorer( const QList<QgsMapLayer *> &layers );

    /**
     * Destructor.
     *
     * Restores layers in their initial states.
     */
    ~QgsLayerRestorer();

  private:
    struct QgsLayerSettings
    {
      QString name;
      double mOpacity;
      QString mNamedStyle;
      QDomDocument mSldStyle;
      QString mFilter;
      QgsFeatureIds mSelectedFeatureIds;
    };

    QMap<QgsMapLayer *, QgsLayerSettings> mLayerSettings;
};

#endif
