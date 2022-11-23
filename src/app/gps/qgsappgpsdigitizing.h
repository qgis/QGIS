/***************************************************************************
    qgsappgpsdigitizing.h
    -------------------
    begin                : October 2022
    copyright            : (C) 2022 Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSAPPGPSDIGITIZING_H
#define QGSAPPGPSDIGITIZING_H

#include <QObject>

#include "qgscoordinatetransform.h"
#include "qgis_app.h"
#include "qgssettingsentryimpl.h"
#include "qgsgpslogger.h"
#include "qgsmaplayeraction.h"
#include "qgsattributes.h"

class QgsAppGpsConnection;
class QgsMapCanvas;
class QgsRubberBand;
class QgsPoint;
class QgsGpsInformation;
class QgsVectorLayer;

class QgsAppGpsDigitizing;

class QgsUpdateGpsDetailsAction : public QgsMapLayerAction
{
    Q_OBJECT

  public:

    QgsUpdateGpsDetailsAction( QgsAppGpsConnection *connection, QgsAppGpsDigitizing *digitizing, QObject *parent );
    bool canRunUsingLayer( QgsMapLayer *layer ) const override;
    bool canRunUsingLayer( QgsMapLayer *layer, const QgsMapLayerActionContext &context ) const override;
    void triggerForFeature( QgsMapLayer *layer, const QgsFeature &feature, const QgsMapLayerActionContext &context ) override;
  private:
    QgsAppGpsConnection *mConnection = nullptr;
    QgsAppGpsDigitizing *mDigitizing = nullptr;

};

class APP_EXPORT QgsAppGpsDigitizing: public QgsGpsLogger
{
    Q_OBJECT

  public:

    static const inline QgsSettingsEntryString settingTrackLineSymbol = QgsSettingsEntryString( QStringLiteral( "track-line-symbol" ), QgsSettings::Prefix::GPS, QStringLiteral( "<symbol alpha=\"1\" name=\"gps-track-symbol\" force_rhr=\"0\" clip_to_extent=\"1\" type=\"line\"><layer enabled=\"1\" pass=\"0\" locked=\"0\" class=\"SimpleLine\"><Option type=\"Map\"><Option name=\"line_color\" type=\"QString\" value=\"219,30,42,255\"/><Option name=\"line_style\" type=\"QString\" value=\"solid\"/><Option name=\"line_width\" type=\"QString\" value=\"0.4\"/></Option></layer></symbol>" ), QStringLiteral( "Line symbol to use for GPS track line" ), Qgis::SettingsOptions(), 0 );

    QgsAppGpsDigitizing( QgsAppGpsConnection *connection, QgsMapCanvas *canvas, QObject *parent = nullptr );
    ~QgsAppGpsDigitizing() override;

    QgsMapCanvas *canvas();

    /**
     * Returns the map of attributes which are auto-populated from GPS information, with their current derived values.
     */
    QgsAttributeMap derivedAttributes() const;

  public slots:
    void createFeature();
    void createVertexAtCurrentLocation();

  private slots:
    void addVertex( const QgsPoint &wgs84Point );
    void onTrackReset();
    void gpsSettingsChanged();
    void updateTrackAppearance();

    void gpsConnected();
    void gpsDisconnected();

  private:
    void createRubberBand();
    QVariant timestamp( QgsVectorLayer *vlayer, int idx ) const;

    QgsAppGpsConnection *mConnection = nullptr;
    QgsMapCanvas *mCanvas = nullptr;

    QgsRubberBand *mRubberBand = nullptr;

    QgsCoordinateTransform mCanvasToWgs84Transform;

    QgsUpdateGpsDetailsAction *mUpdateGpsDetailsAction = nullptr;

    friend class TestQgsGpsIntegration;
};

#endif // QGSAPPGPSDIGITIZING
