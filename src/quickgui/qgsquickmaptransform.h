/***************************************************************************
  qgsquickmaptransform.h
  --------------------------------------
  Date                 : 27.12.2014
  Copyright            : (C) 2014 by Matthias Kuhn
  Email                : matthias (at) opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSQUICKMAPTRANSFORM_H
#define QGSQUICKMAPTRANSFORM_H

#include <QQuickItem>
#include <QMatrix4x4>

#include "qgis_quick.h"
#include "qgsquickmapsettings.h"

/**
 * \ingroup quick
 * \brief The QgsQuickMapTransform is transformation that can be attached to any QQuickItem.
 *
 * If the item is based on the map coordinates, QgsQuickMapTransform will
 * transform it to the device coordinates based on the attached map settings.
 *
 * \note QML Type: MapTransform
 *
 * \since QGIS 3.4
 */
class QUICK_EXPORT QgsQuickMapTransform : public QQuickTransform
{
    Q_OBJECT

    /**
     * Associated map settings. Should be initialized before the first use from mapcanvas map settings.
     */
    Q_PROPERTY( QgsQuickMapSettings *mapSettings READ mapSettings WRITE setMapSettings NOTIFY mapSettingsChanged )

  public:
    //! Creates a new map transform
    QgsQuickMapTransform() = default;
    ~QgsQuickMapTransform() = default;

    /**
     * Applies transformation based on current map settings to a matrix.
     *
     * Also optimize resulting matrix after transformation
     * \param matrix Matrix to be transformed
     */
    void applyTo( QMatrix4x4 *matrix ) const;

    //! \copydoc QgsQuickMapTransform::mapSettings
    QgsQuickMapSettings *mapSettings() const;

    //! \copydoc QgsQuickMapTransform::mapSettings
    void setMapSettings( QgsQuickMapSettings *mapSettings );

  signals:
    //! \copydoc QgsQuickMapTransform::mapSettings
    void mapSettingsChanged();

  private slots:
    void updateMatrix();

  private:
    QgsQuickMapSettings *mMapSettings = nullptr; // not owned
    QMatrix4x4 mMatrix;
};

#endif // QGSQUICKMAPTRANSFORM_H
