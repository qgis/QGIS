/***************************************************************************
  qgsqguickfeaturehighlight.h
  ---------------------------
  Date                 : May 2018
  Copyright            : (C) 2018 by Peter Petrik
  Email                : zilolv at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSQUICKFEATUREHIGHLIGHT_H
#define QGSQUICKFEATUREHIGHLIGHT_H

#include <QQuickItem>

#include "qgsquickfeaturelayerpair.h"
#include "qgis_quick.h"
#include "qgsquickmaptransform.h"

class QgsQuickMapSettings;

/**
 * \ingroup quick
 *
 * Creates map highlights for a geometry provided by a AttributeModel.
 *
 * The highlights are compatible with the QtQuick scene graph and
 * can be direcly shown on map canvas
 *
 * \note QML Type: FeatureHighlight
 *
 * \since QGIS 3.4
 */
class QUICK_EXPORT QgsQuickFeatureHighlight : public QQuickItem
{
    Q_OBJECT

    /**
     * Associated map settings. Should be initialized from QML component before the first use.
     */
    Q_PROPERTY( QgsQuickMapSettings *mapSettings MEMBER mMapSettings NOTIFY mapSettingsChanged )

    /**
     * Feature to highlight
     */
    Q_PROPERTY( QgsQuickFeatureLayerPair featureLayerPair MEMBER mFeatureLayerPair NOTIFY featureLayerPairChanged )

    /**
     * Color of the highlighted geometry (feature).
     *
     * Default is yellow color
     */
    Q_PROPERTY( QColor color MEMBER mColor NOTIFY colorChanged )

    /**
     * Pen width of the highlighted geometry (feature).
     *
     * Default is 20, see QSGGeometry::setLineWidth()
     */
    Q_PROPERTY( float width MEMBER mWidth NOTIFY widthChanged )

  public:
    //! Creates a new feature highlight
    explicit QgsQuickFeatureHighlight( QQuickItem *parent = nullptr );

  signals:
    //! \copydoc QgsQuickFeatureHighlight::featureLayerPair
    void featureLayerPairChanged();

    //! \copydoc QgsQuickFeatureHighlight::color
    void colorChanged();

    //! \copydoc QgsQuickFeatureHighlight::width
    void widthChanged();

    //! \copydoc QgsQuickFeatureHighlight::mapSettings
    void mapSettingsChanged();

  private slots:
    void markDirty();
    void onMapSettingsChanged();

  private:
    QSGNode *updatePaintNode( QSGNode *n, UpdatePaintNodeData * ) override;

    QColor mColor = Qt::yellow;
    bool mDirty = false;
    float mWidth = 20;
    QgsQuickFeatureLayerPair mFeatureLayerPair;
    QgsQuickMapSettings *mMapSettings = nullptr; // not owned
    QgsQuickMapTransform mTransform;
};

#endif // QGSQUICKFEATUREHIGHLIGHT_H
