/***************************************************************************
  qgslabelinggui.h
  Smart labeling for vector layers
  -------------------
         begin                : June 2009
         copyright            : (C) Martin Dobias
         email                : wonder dot sk at gmail dot com

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLABELINGGUI_H
#define QGSLABELINGGUI_H

#include "qgspallabeling.h"
#include "qgstextformatwidget.h"
#include "qgspropertyoverridebutton.h"
#include "qgis_gui.h"

class QDialogButtonBox;

class QgsMeshLayer;
class QgsVectorTileLayer;

#define SIP_NO_FILE

///@cond PRIVATE

class GUI_EXPORT QgsLabelingGui : public QgsTextFormatWidget
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsLabelingGui, for configuring a vector \a layer labeling.
     */
    QgsLabelingGui( QgsVectorLayer *layer, QgsMapCanvas *mapCanvas, const QgsPalLayerSettings &settings, QWidget *parent = nullptr, Qgis::GeometryType geomType = Qgis::GeometryType::Unknown );

    /**
     * Constructor for QgsLabelingGui, for configuring a mesh \a layer labeling.
     */
    QgsLabelingGui( QgsMeshLayer *layer, QgsMapCanvas *mapCanvas, const QgsPalLayerSettings &settings, QWidget *parent = nullptr, Qgis::GeometryType geomType = Qgis::GeometryType::Unknown );

    /**
     * Constructor for QgsLabelingGui, for configuring a vector tile \a layer labeling.
     */
    QgsLabelingGui( QgsVectorTileLayer *layer, QgsMapCanvas *mapCanvas, const QgsPalLayerSettings &settings, QWidget *parent = nullptr, Qgis::GeometryType geomType = Qgis::GeometryType::Unknown );

    /**
     * Generic constructor for QgsLabelingGui, when no layer is available.
     */
    QgsLabelingGui( QgsMapCanvas *mapCanvas, const QgsPalLayerSettings &settings, QWidget *parent = nullptr );

    QgsPalLayerSettings layerSettings();

    enum LabelMode
    {
      NoLabels,
      Labels,
      ObstaclesOnly,
    };

    void setLabelMode( LabelMode mode );

    virtual void setLayer( QgsMapLayer *layer );

    void setSettings( const QgsPalLayerSettings &settings );

    void setContext( const QgsSymbolWidgetContext &context ) override;

  public slots:

    void updateUi();

  protected slots:
    void setFormatFromStyle( const QString &name, QgsStyle::StyleEntity type, const QString &stylePath ) override;
    void saveFormat() override;

  protected:
    /**
     * Constructor for QgsLabelingGui, for subclasses.
     *
     * \warning The subclass constructor must call the init() and setLayer() methods.
     *
     * \param mapCanvas associated map canvas
     * \param parent parent widget
     * \param layer associated layer
     *
     * \note Not available in Python bindings
     * \since QGIS 3.42
     */
    QgsLabelingGui( QgsMapCanvas *mapCanvas, QWidget *parent, QgsMapLayer *layer ) SIP_SKIP;

    /**
     * Initializes the widget.
     *
     * \since QGIS 3.42
     */
    void init();

    void blockInitSignals( bool block );
    void syncDefinedCheckboxFrame( QgsPropertyOverrideButton *ddBtn, QCheckBox *chkBx, QFrame *f );
    bool eventFilter( QObject *object, QEvent *event ) override;

    //! Dialog mode
    LabelMode mMode;

  private slots:

    /**
     * Called when the geometry type is changed and
     * configuration options which only work with a specific
     * geometry type should be updated.
     */
    void updateGeometryTypeBasedWidgets();
    void showGeometryGeneratorExpressionBuilder();
    void validateGeometryGeneratorExpression();
    void determineGeometryGeneratorType();

    /**
     * Update widget when callout type changes
     */
    void calloutTypeChanged();

  private:
    QgsPalLayerSettings mSettings;
    QgsFeature mPreviewFeature;

    QgsLabelObstacleSettings mObstacleSettings;
    QgsLabelLineSettings mLineSettings;

    QgsExpressionContext createExpressionContext() const override;

  private slots:

    void updateCalloutWidget( QgsCallout *callout );
    void showObstacleSettings();
    void showLineAnchorSettings();
};

class GUI_EXPORT QgsLabelSettingsDialog : public QDialog
{
    Q_OBJECT

  public:
    QgsLabelSettingsDialog( const QgsPalLayerSettings &settings, QgsVectorLayer *layer, QgsMapCanvas *mapCanvas, QWidget *parent SIP_TRANSFERTHIS = nullptr, Qgis::GeometryType geomType = Qgis::GeometryType::Unknown );

    QgsPalLayerSettings settings() const { return mWidget->layerSettings(); }

    /**
     * Returns a reference to the dialog's button box.
     */
    QDialogButtonBox *buttonBox() const;

  private:
    QgsLabelingGui *mWidget = nullptr;
    QDialogButtonBox *mButtonBox = nullptr;

  private slots:
    void showHelp();
};

///@endcond PRIVATE

#endif // QGSLABELINGGUI_H
