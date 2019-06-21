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

#define SIP_NO_FILE

///@cond PRIVATE

class GUI_EXPORT QgsLabelingGui : public QgsTextFormatWidget, private QgsExpressionContextGenerator
{
    Q_OBJECT

  public:
    QgsLabelingGui( QgsVectorLayer *layer, QgsMapCanvas *mapCanvas, const QgsPalLayerSettings &settings, QWidget *parent = nullptr,
                    QgsWkbTypes::GeometryType geomType = QgsWkbTypes::UnknownGeometry );

    QgsPalLayerSettings layerSettings();

    enum LabelMode
    {
      NoLabels,
      Labels,
      ObstaclesOnly,
    };

    void setLabelMode( LabelMode mode );

    void setLayer( QgsMapLayer *layer );

    void setSettings( const QgsPalLayerSettings &settings );

    /**
     * Deactivate a field from data defined properties and update the
     * corresponding button.
     *
     * \param key The property key to deactivate
     *
     * \since QGIS 3.0
     */
    void deactivateField( QgsPalLayerSettings::Property key );

  signals:

    void auxiliaryFieldCreated();

  public slots:

    void updateUi();

    void createAuxiliaryField();

  protected slots:
    void setFormatFromStyle( const QString &name, QgsStyle::StyleEntity type ) override;
    void saveFormat() override;

  protected:
    void blockInitSignals( bool block );
    void syncDefinedCheckboxFrame( QgsPropertyOverrideButton *ddBtn, QCheckBox *chkBx, QFrame *f );

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

  private:
    QgsVectorLayer *mLayer = nullptr;
    QgsWkbTypes::GeometryType mGeomType = QgsWkbTypes::UnknownGeometry;
    QgsPalLayerSettings mSettings;
    QgsPropertyCollection mDataDefinedProperties;
    LabelMode mMode;
    QgsFeature mPreviewFeature;
    QgsMapCanvas *mCanvas = nullptr;

    QgsExpressionContext createExpressionContext() const override;

    void populateDataDefinedButtons();
    void registerDataDefinedButton( QgsPropertyOverrideButton *button, QgsPalLayerSettings::Property key );

    QMap<QgsPalLayerSettings::Property, QgsPropertyOverrideButton *> mButtons;

  private slots:

    void updateProperty();

};

class GUI_EXPORT QgsLabelSettingsDialog : public QDialog
{
    Q_OBJECT

  public:

    QgsLabelSettingsDialog( const QgsPalLayerSettings &settings, QgsVectorLayer *layer, QgsMapCanvas *mapCanvas, QWidget *parent SIP_TRANSFERTHIS = nullptr,
                            QgsWkbTypes::GeometryType geomType = QgsWkbTypes::UnknownGeometry );

    QgsPalLayerSettings settings() const { return mWidget->layerSettings(); }

  private:

    QgsLabelingGui *mWidget = nullptr;

};

///@endcond PRIVATE

#endif // QGSLABELINGGUI_H


