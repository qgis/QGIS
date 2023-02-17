/***************************************************************************
  qgslightswidget.h
  --------------------------------------
  Date                 : November 2018
  Copyright            : (C) 2018 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLIGHTSWIDGET_H
#define QGSLIGHTSWIDGET_H

#include <QWidget>

#include "ui_qgslightswidget.h"

#include "qgspointlightsettings.h"
#include "qgsdirectionallightsettings.h"

class QgsLightsModel : public QAbstractListModel
{
    Q_OBJECT
  public:

    enum LightType
    {
      Point,
      Directional
    };

    enum Role
    {
      LightTypeRole = Qt::UserRole,
      LightListIndex,
    };

    explicit QgsLightsModel( QObject *parent = nullptr );

    int rowCount( const QModelIndex &parent ) const override;
    QVariant data( const QModelIndex &index, int role ) const override;
    bool removeRows( int row, int count, const QModelIndex &parent = QModelIndex() ) override;

    void setPointLights( const QList<QgsPointLightSettings> &lights );
    void setDirectionalLights( const QList<QgsDirectionalLightSettings> &lights );

    QList<QgsPointLightSettings> pointLights() const;
    QList<QgsDirectionalLightSettings> directionalLights() const;

    void setPointLightSettings( int index, const QgsPointLightSettings &light );
    void setDirectionalLightSettings( int index, const QgsDirectionalLightSettings &light );

    QModelIndex addPointLight( const QgsPointLightSettings &light );
    QModelIndex addDirectionalLight( const QgsDirectionalLightSettings &light );

  private:

    QList<QgsPointLightSettings> mPointLights;
    QList<QgsDirectionalLightSettings> mDirectionalLights;
};

/**
 * Widget for configuration of lights in 3D map scene
 * \since QGIS 3.6
 */
class QgsLightsWidget : public QWidget, private Ui::QgsLightsWidget
{
    Q_OBJECT
  public:
    explicit QgsLightsWidget( QWidget *parent = nullptr );

    void setLights( const QList<QgsLightSource *> sources );

    QList<QgsLightSource *> lightSources();

    int directionalLightCount() const;
    int lightSourceCount() const;

  signals:
    void directionalLightsCountChanged( int count );
    void lightsRemoved();
    void lightsAdded();

  private slots:
    void selectedLightChanged( const QItemSelection &selected, const QItemSelection &deselected );
    void updateCurrentLightParameters();
    void onAddLight();
    void onRemoveLight();

    void updateCurrentDirectionalLightParameters();
    void onAddDirectionalLight();
    void setAzimuthAltitude();
    void onDirectionChange();
  private:

    void showSettingsForPointLight( const QgsPointLightSettings &settings );
    void showSettingsForDirectionalLight( const QgsDirectionalLightSettings &settings );

  private:
    double mDirectionX = 0;
    double mDirectionY = -1;
    double mDirectionZ = 0;
    QgsLightsModel *mLightsModel = nullptr;
};


#endif // QGSLIGHTSWIDGET_H
