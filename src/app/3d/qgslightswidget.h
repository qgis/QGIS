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

#include "ui_qgslightswidget.h"

#include "qgsdirectionallightsettings.h"
#include "qgspointlightsettings.h"
#include "qgssunlightsettings.h"

#include <QSortFilterProxyModel>
#include <QWidget>

class QgsLightsModel : public QAbstractListModel
{
    Q_OBJECT
  public:
    enum Role
    {
      LightTypeRole = Qt::UserRole,
      LightListIndex,
      LightId
    };

    explicit QgsLightsModel( QObject *parent = nullptr );

    int rowCount( const QModelIndex &parent ) const override;
    QVariant data( const QModelIndex &index, int role ) const override;
    bool removeRows( int row, int count, const QModelIndex &parent = QModelIndex() ) override;

    void setPointLights( const QList<QgsPointLightSettings> &lights );
    void setDirectionalLights( const QList<QgsDirectionalLightSettings> &lights );
    void setSunLights( const QList<QgsSunLightSettings> &lights );

    QList<QgsPointLightSettings> pointLights() const;
    QList<QgsDirectionalLightSettings> directionalLights() const;
    QList<QgsSunLightSettings> sunLights() const;

    void setPointLightSettings( int index, const QgsPointLightSettings &light );
    void setDirectionalLightSettings( int index, const QgsDirectionalLightSettings &light );
    void setSunLightSettings( int index, const QgsSunLightSettings &light );

    QModelIndex addPointLight( const QgsPointLightSettings &light );
    QModelIndex addDirectionalLight( const QgsDirectionalLightSettings &light );
    QModelIndex addSunLight( const QgsSunLightSettings &light );

    //! Returns the model index corresponding to a light's ID
    QModelIndex indexFromLightId( const QString &id ) const;

  private:
    QList<QgsPointLightSettings> mPointLights;
    QList<QgsDirectionalLightSettings> mDirectionalLights;
    QList<QgsSunLightSettings> mSunLights;
};

/**
 * \ingroup qgis_3d
 * \brief Proxy model for filtering a QgsLightsModel to specific allowed light types.
 */
class QgsLightsProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

  public:
    explicit QgsLightsProxyModel( QObject *parent = nullptr );

    /**
   * Sets the light \a types that to include in the model.
   */
    void setAllowedLightTypes( const QList<Qgis::LightSourceType> &types );

  protected:
    bool filterAcceptsRow( int source_row, const QModelIndex &source_parent ) const override;

  private:
    QSet<Qgis::LightSourceType> mAllowedTypes;
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

    void setSceneMode( Qgis::SceneMode mode );

    void setLights( const QList<QgsLightSource *> sources );

    QList<QgsLightSource *> lightSources();

    int directionalLightCount() const;
    int lightSourceCount() const;

    void setPointLightCrs( const QgsCoordinateReferenceSystem &crs );
    void setMapExtent( const QgsRectangle &extent );

    QgsLightsModel *lightSourceModel() { return mLightsModel; }

  signals:
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

    void updateCurrentSunLightParameters();
    void onAddSunLight();

  private:
    void showSettingsForPointLight( const QgsPointLightSettings &settings );
    void showSettingsForDirectionalLight( const QgsDirectionalLightSettings &settings );
    void showSettingsForSunLight( const QgsSunLightSettings &settings );

  private:
    double mDirectionX = 0;
    double mDirectionY = 0;
    double mDirectionZ = 0;
    QgsLightsModel *mLightsModel = nullptr;
    QgsRectangle mMapExtent;
    Qgis::SceneMode mSceneMode = Qgis::SceneMode::Local;
};


#endif // QGSLIGHTSWIDGET_H
