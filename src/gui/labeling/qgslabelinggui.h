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

#define SIP_NO_FILE

///@cond PRIVATE

class GUI_EXPORT QgsLabelingGui : public QgsTextFormatWidget
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

    void setContext( const QgsSymbolWidgetContext &context ) override;

  public slots:

    void updateUi();

  protected slots:
    void setFormatFromStyle( const QString &name, QgsStyle::StyleEntity type, const QString &stylePath ) override;
    void saveFormat() override;

  protected:
    void blockInitSignals( bool block );
    void syncDefinedCheckboxFrame( QgsPropertyOverrideButton *ddBtn, QCheckBox *chkBx, QFrame *f );
    bool eventFilter( QObject *object, QEvent *event ) override;

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
    LabelMode mMode;
    QgsFeature mPreviewFeature;
    QgsMapCanvas *mCanvas = nullptr;

    QgsLabelObstacleSettings mObstacleSettings;
    QgsLabelLineSettings mLineSettings;

    QgsExpressionContext createExpressionContext() const override;

  private slots:

    void initCalloutWidgets();
    void updateCalloutWidget( QgsCallout *callout );
    void showObstacleSettings();
    void showLineAnchorSettings();

};

class GUI_EXPORT QgsLabelSettingsDialog : public QDialog
{
    Q_OBJECT

  public:

    QgsLabelSettingsDialog( const QgsPalLayerSettings &settings, QgsVectorLayer *layer, QgsMapCanvas *mapCanvas, QWidget *parent SIP_TRANSFERTHIS = nullptr,
                            QgsWkbTypes::GeometryType geomType = QgsWkbTypes::UnknownGeometry );

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


