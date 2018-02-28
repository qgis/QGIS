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

    Q_PROPERTY( QgsQuickFeatureModel *model MEMBER mModel NOTIFY modelChanged )
    Q_PROPERTY( QColor color MEMBER mColor NOTIFY colorChanged )
    Q_PROPERTY( unsigned int width MEMBER mWidth NOTIFY widthChanged )
    Q_PROPERTY( QgsQuickMapSettings *mapSettings MEMBER mMapSettings NOTIFY mapSettingsChanged )

  public:
    explicit QgsQuickFeatureHighlight( QQuickItem *parent = 0 );

  signals:
    void modelChanged();
    void colorChanged();
    void mapCanvasChanged();
    void widthChanged();
    void mapSettingsChanged();

  private slots:
    void onDataChanged();
    void onModelDataChanged();

  private:
    virtual QSGNode *updatePaintNode( QSGNode *n, UpdatePaintNodeData * ) override;

    QColor mColor;
    QgsQuickFeatureModel *mModel;
    bool mDirty;
    unsigned int mWidth;
    QgsQuickMapSettings *mMapSettings;
};

#endif // QGSQUICKFEATUREHIGHLIGHT_H
