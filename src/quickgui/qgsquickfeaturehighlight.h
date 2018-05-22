/***************************************************************************
  qgsqguickfeaturehighlight.h
  --------------------------------------
  Date                 : 9.12.2014
  Copyright            : (C) 2014 by Matthias Kuhn
  Email                : matthias@opengis.ch
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

#include "qgis_quick.h"

class QgsQuickMapSettings;
class QgsQuickFeatureModel;

/**
 * \ingroup quick
 *
 * Creates map highlights for a geometry provided by a FeatureModel.
 *
 * The highlights are compatible with the QtQuick scene graph.
 *
 * \note QML Type: FeatureModelHighlight
 *
 * \since QGIS 3.2
 */
class QUICK_EXPORT QgsQuickFeatureHighlight : public QQuickItem
{
    Q_OBJECT

    /**
     * Associated map settings. Should be initialized from QML component before the first use.
     */
    Q_PROPERTY( QgsQuickMapSettings *mapSettings MEMBER mMapSettings NOTIFY mapSettingsChanged )

    /**
     * Feature model for geometry.
     */
    Q_PROPERTY( QgsQuickFeatureModel *model MEMBER mModel NOTIFY modelChanged )

    /**
     * Color of the highlighted geometry (feature).
     */
    Q_PROPERTY( QColor color MEMBER mColor NOTIFY colorChanged )

    /**
     * Pen width of the highlighted geometry (feature). Default is 20.
     */
    Q_PROPERTY( unsigned int width MEMBER mWidth NOTIFY widthChanged )

  public:
    //! Creates a new feature highlight
    explicit QgsQuickFeatureHighlight( QQuickItem *parent = nullptr );

  signals:
    //! \copydoc QgsQuickFeatureHighlight::model
    void modelChanged();

    //! \copydoc QgsQuickFeatureHighlight::color
    void colorChanged();

    //! \copydoc QgsQuickFeatureHighlight::width
    void widthChanged();

    //! \copydoc QgsQuickFeatureHighlight::mapSettings
    void mapSettingsChanged();

  private slots:
    void onDataChanged();
    void onModelDataChanged();

  private:
    QSGNode *updatePaintNode( QSGNode *n, UpdatePaintNodeData * ) override;

    QColor mColor = Qt::yellow;
    bool mDirty = false;
    unsigned int mWidth = 20;
    QgsQuickFeatureModel *mModel = nullptr; // not owned
    QgsQuickMapSettings *mMapSettings = nullptr; // not owned
};

#endif // QGSQUICKFEATUREHIGHLIGHT_H
