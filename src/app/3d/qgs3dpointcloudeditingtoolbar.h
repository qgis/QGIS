/***************************************************************************
    qgs3dpointcloudeditingtoolbar.h
    -------------------
    begin                : November 2025
    copyright            : (C) 2025 Oslandia
    email                : benoit dot de dot mezzo at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGS3DPOINTCLOUDEDITINGTOOLBAR_H
#define QGS3DPOINTCLOUDEDITINGTOOLBAR_H

#include "qgs3deditingtoolbar.h"
#include "qgs3dmapcanvaswidget.h"

/**
 * Allow creation of 3D primitive on point cloud layers
 *
 * \since QGIS 3.44
 */
class Qgs3DMapToolPointCloudChangeAttribute;

class Qgs3DPointCloudEditingToolBar : public Qgs3DEditingToolBar
{
    Q_OBJECT

  public:
    /**
     * Default constructor
     * \param parent parent widget
     */
    Qgs3DPointCloudEditingToolBar( Qgs3DMapCanvasWidget *parent );

    bool accept( QgsMapLayer *layer ) const override;
    void activate( QgsMapLayer *layer ) override;
    void deactivate() override;
    QList<QAction *> groupActions() const override;

    Qgs3DMapToolPointCloudChangeAttribute *tool() const { return mMapToolChangeAttribute; }

  private:
    Qgs3DMapToolPointCloudChangeAttribute *mMapToolChangeAttribute = nullptr;
    QComboBox *mCboChangeAttribute = nullptr;
    QComboBox *mCboChangeAttributeValue = nullptr;
    ClassValidator *mClassValidator = nullptr;
    QgsDoubleSpinBox *mSpinChangeAttributeValue = nullptr;
    QAction *mCboChangeAttributeValueAction = nullptr;
    QAction *mSpinChangeAttributeValueAction = nullptr;
    QString mChangeAttributePointFilter;

    QMenu *mEditingToolsMenu = nullptr;
    QAction *mEditingToolsAction = nullptr;

    QList<QAction *> mGroupActions;

    Qgs3DMapCanvas *mapCanvas3D() { return dynamic_cast<Qgs3DMapCanvasWidget *>( mParentWidget )->mapCanvas3D(); }

  private slots:
    void changePointCloudAttributeByPaintbrush();
    void changePointCloudAttributeByPolygon();
    void changePointCloudAttributeByAboveLine();
    void changePointCloudAttributeByBelowLine();
    void changePointCloudAttributePointFilter();
    void onPointCloudChangeAttributeSettingsChanged();
};

#endif // QGS3DPOINTCLOUDEDITINGTOOLBAR_H
