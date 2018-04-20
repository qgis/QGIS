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

    //! map settings
    Q_PROPERTY( QgsQuickMapSettings *mapSettings MEMBER mMapSettings NOTIFY mapSettingsChanged )
    //! feature model (for geometry)
    Q_PROPERTY( QgsQuickFeatureModel *model MEMBER mModel NOTIFY modelChanged )
    //! color of the highlighed geometry
    Q_PROPERTY( QColor color MEMBER mColor NOTIFY colorChanged )
    //! pen width of the highlight
    Q_PROPERTY( unsigned int width MEMBER mWidth NOTIFY widthChanged )

  public:
    //! create new feature highlight
    explicit QgsQuickFeatureHighlight( QQuickItem *parent = 0 );

  signals:
    //! model changed
    void modelChanged();

    //! color changed
    void colorChanged();

    //! map canvas changed
    void mapCanvasChanged();

    //! width changed
    void widthChanged();

    //! map settings changed
    void mapSettingsChanged();

  private slots:
    void onDataChanged();
    void onModelDataChanged();

  private:
    virtual QSGNode *updatePaintNode( QSGNode *n, UpdatePaintNodeData * ) override;

    QColor mColor;
    QgsQuickFeatureModel *mModel = nullptr;
    bool mDirty = false;
    unsigned int mWidth;
    QgsQuickMapSettings *mMapSettings = nullptr;
};

#endif // QGSQUICKFEATUREHIGHLIGHT_H
