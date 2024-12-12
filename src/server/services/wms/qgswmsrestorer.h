/***************************************************************************
                              qgswmsrestorer.h
                              ----------------
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

#ifndef QGSWMSRESTORER_H
#define QGSWMSRESTORER_H

#include <QList>
#include <QDomDocument>
#include <QMap>

#include "qgsfeatureid.h"
#include "qgswmsrendercontext.h"

class QgsMapLayer;
class QgsAbstractVectorLayerLabeling;

/**
 * \ingroup server
 * \brief RAII class to restore layer configuration on destruction (opacity,
 * filters, ...)
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
        std::unique_ptr<QgsAbstractVectorLayerLabeling> mLabeling;
        QString mNamedStyle;
        QString mFilter;
        QgsFeatureIds mSelectedFeatureIds;
    };

    std::map<QgsMapLayer *, QgsLayerSettings> mLayerSettings;
};

namespace QgsWms
{

  /**
   * \ingroup server
   * \brief RAII class to restore the rendering context configuration on destruction
   * \since QGIS 3.14
   */
  class QgsWmsRestorer
  {
    public:
      /**
       * Constructor for QgsWmsRestorer.
       * \param context The rendering context to restore in its initial state
       */
      QgsWmsRestorer( const QgsWmsRenderContext &context );

      ~QgsWmsRestorer() = default;

    private:
      QgsLayerRestorer mLayerRestorer;
  };
}; // namespace QgsWms

#endif
