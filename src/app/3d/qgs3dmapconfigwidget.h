/***************************************************************************
  qgs3dmapconfigwidget.h
  --------------------------------------
  Date                 : July 2017
  Copyright            : (C) 2017 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGS3DMAPCONFIGWIDGET_H
#define QGS3DMAPCONFIGWIDGET_H

#include <QWidget>

#include <ui_map3dconfigwidget.h>
#include "qgis_app.h"

class QCheckBox;
class Qgs3DMapSettings;
class QgsMapCanvas;
class QgsMesh3DSymbolWidget;
class QgsSkyboxRenderingSettingsWidget;
class QgsShadowRenderingSettingsWidget;
class Qgs3DMapCanvas;
class QgsSkyboxSettings;

class APP_EXPORT Qgs3DMapConfigWidget : public QWidget, private Ui::Map3DConfigWidget
{
    Q_OBJECT
  public:
    //! construct widget. does not take ownership of the passed map.
    explicit Qgs3DMapConfigWidget( Qgs3DMapSettings *map, QgsMapCanvas *mainCanvas, Qgs3DMapCanvas *mapCanvas3D, QWidget *parent = nullptr );

    ~Qgs3DMapConfigWidget() override;

    void apply();

  signals:

    void isValidChanged( bool valid );

  private slots:
    void onTerrainTypeChanged();
    void onTerrainLayerChanged();
    void updateMaxZoomLevel();
    void validate();
    void on3DAxisChanged();

  private:
    Qgs3DMapSettings *mMap = nullptr;
    QgsMapCanvas *mMainCanvas = nullptr;
    QgsMesh3DSymbolWidget *mMeshSymbolWidget = nullptr;
    QgsSkyboxRenderingSettingsWidget *mSkyboxSettingsWidget = nullptr;
    QgsShadowRenderingSettingsWidget *mShadowSettingsWidget = nullptr;
    QCheckBox *mShowExtentIn2DViewCheckbox = nullptr;

    void init3DAxisPage();
};

#endif // QGS3DMAPCONFIGWIDGET_H
