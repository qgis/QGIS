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

#include "qgsmaplayer.h"

/**
 * RAII class to restore layer configuration on destruction (opacity,
 *  filters, ...)
 * \since QGIS 3.0
 */
class QgsLayerRestorer
{
    struct QgsLayerSettings
    {
      QString name;
      double mOpacity;
      QString mNamedStyle;
      QDomDocument mSldStyle;
      QString mFilter;
      QgsFeatureIds mSelectedFeatureIds;
    };

  public:
    QgsLayerRestorer( const QList<QgsMapLayer *> &layers );
    ~QgsLayerRestorer();

  private:
    QMap<QgsMapLayer *, QgsLayerSettings> mLayerSettings;
};

#endif
