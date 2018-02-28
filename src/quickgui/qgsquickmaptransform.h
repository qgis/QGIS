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
    Q_PROPERTY( QgsQuickMapSettings *mapSettings READ mapSettings WRITE setMapSettings NOTIFY mapSettingsChanged )

  public:
    QgsQuickMapTransform();
    ~QgsQuickMapTransform();

    void applyTo( QMatrix4x4 *matrix ) const;

    QgsQuickMapSettings *mapSettings() const;
    void setMapSettings( QgsQuickMapSettings *mapSettings );

  signals:
    void mapSettingsChanged();

  private slots:
    void updateMatrix();

  private:
    QgsQuickMapSettings *mMapSettings = nullptr;
    QMatrix4x4 mMatrix;
};

#endif // QGSQUICKMAPTRANSFORM_H
