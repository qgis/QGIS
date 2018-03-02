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

class QgsQuickMapSettings;

/**
 * \ingroup quick
 * The QgsQuickMapTransform is transformation that can be attached to any QQuickItem. Transformation scales and translates
 * Item based on the current QgsQuickMapSettings settings.
 *
 * For example it can be used on QgsQuickFeatureHighlight to place it correctly on the map canvas.
 *
 * \note QML Type: MapTransform
 *
 * \since QGIS 3.2
 */
class QUICK_EXPORT QgsQuickMapTransform : public QQuickTransform
{
    Q_OBJECT

    //! map settings
    Q_PROPERTY( QgsQuickMapSettings *mapSettings READ mapSettings WRITE setMapSettings NOTIFY mapSettingsChanged )

  public:
    //! create new map transform
    QgsQuickMapTransform();
    ~QgsQuickMapTransform();

    /**
     * Apply transformation based on current map settings to a matrix.
     *
     * Also optimize resulting matrix after transformation
     * \param matrix Matrix to be transformed
     */
    void applyTo( QMatrix4x4 *matrix ) const;

    //! Return map settings
    QgsQuickMapSettings *mapSettings() const;

    //! Set map settings
    void setMapSettings( QgsQuickMapSettings *mapSettings );

  signals:
    //! Map settings changed
    void mapSettingsChanged();

  private slots:
    void updateMatrix();

  private:
    QgsQuickMapSettings *mMapSettings = nullptr;
    QMatrix4x4 mMatrix;
};

#endif // QGSQUICKMAPTRANSFORM_H
