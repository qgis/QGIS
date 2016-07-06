/***************************************************************************
    qgsglobevectorlayerproperties.h
     --------------------------------------
    Date                 : 9.7.2013
    Copyright            : (C) 2013 Matthias Kuhn
    Email                : matthias dot kuhn at gmx dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSGLOBEVECTORLAYERPROPERTIES_H
#define QGSGLOBEVECTORLAYERPROPERTIES_H

#include <QIcon>

#include "ui_qgsglobevectorlayerpropertiespage.h"
#include <qgsmaplayerconfigwidget.h>
#include <qgsmaplayerconfigwidgetfactory.h>
#include <osgEarthSymbology/AltitudeSymbol>

class QgsGlobeVectorLayerConfig;
class QgsMapLayer;
class QDomDocument;
class QDomElement;
class QListWidgetItem;

class QgsGlobeVectorLayerConfig : public QObject
{
  public:
    enum RenderingMode
    {
      RenderingModeRasterized,
      RenderingModeModelSimple,
      RenderingModeModelAdvanced
    };

    QgsGlobeVectorLayerConfig( QObject* parent = 0 )
        : QObject( parent )
        , renderingMode( RenderingModeRasterized )
        , altitudeClamping( osgEarth::Symbology::AltitudeSymbol::CLAMP_TO_TERRAIN )
        , altitudeTechnique( osgEarth::Symbology::AltitudeSymbol::TECHNIQUE_DRAPE )
        , altitudeBinding( osgEarth::Symbology::AltitudeSymbol::BINDING_VERTEX )
        , verticalOffset( 0.0 )
        , verticalScale( 0.0 )
        , clampingResolution( 0.0 )
        , extrusionEnabled( false )
        , extrusionHeight( "10" )
        , extrusionFlatten( false )
        , extrusionWallGradient( 0.5 )
        , labelingEnabled( false )
        , labelingDeclutter( false )
        , lightingEnabled( true )
    {
    }

    RenderingMode renderingMode;
    osgEarth::Symbology::AltitudeSymbol::Clamping altitudeClamping;
    osgEarth::Symbology::AltitudeSymbol::Technique altitudeTechnique;
    osgEarth::Symbology::AltitudeSymbol::Binding altitudeBinding;

    float verticalOffset;
    float verticalScale;
    float clampingResolution;

    bool extrusionEnabled;
    QString extrusionHeight;
    bool extrusionFlatten;
    float extrusionWallGradient;

    bool labelingEnabled;
    QString labelingField;
    bool labelingDeclutter;

    bool lightingEnabled;

    static QgsGlobeVectorLayerConfig* getConfig( QgsVectorLayer* layer );
};


class QgsGlobeVectorLayerPropertiesPage : public QgsMapLayerConfigWidget, private Ui::QgsGlobeVectorLayerPropertiesPage
{
    Q_OBJECT

  public:
    explicit QgsGlobeVectorLayerPropertiesPage( QgsVectorLayer* layer, QgsMapCanvas* canvas, QWidget *parent = 0 );

  public slots:
    virtual void apply();

  private slots:
    void onAltitudeClampingChanged( int index );
    void onAltituteTechniqueChanged( int index );
    void showRenderingModeWidget( int index );

  signals:
    void layerSettingsChanged( QgsMapLayer* );

  private:
    QgsVectorLayer* mLayer;
};


class QgsGlobeLayerPropertiesFactory : public QObject, public QgsMapLayerConfigWidgetFactory
{
    Q_OBJECT
  public:
    explicit QgsGlobeLayerPropertiesFactory( QObject* parent = 0 );
    QgsMapLayerConfigWidget* createWidget( QgsMapLayer *layer, QgsMapCanvas *canvas, bool dockWidget, QWidget *parent ) const override;

    QIcon icon() const override;

    QString title() const override;

    bool supportLayerPropertiesDialog() const override { return true; }

    bool supportsLayer( QgsMapLayer *layer ) const override;

  signals:
    void layerSettingsChanged( QgsMapLayer* layer );

  private slots:
    void readGlobeVectorLayerConfig( QgsMapLayer* mapLayer, const QDomElement &elem );
    void writeGlobeVectorLayerConfig( QgsMapLayer* mapLayer, QDomElement& elem, QDomDocument& doc );
};

#endif // QGSGLOBEVECTORLAYERPROPERTIES_H
