/***************************************************************************
  qgstextsettingswidget.h
  -------------------
         begin                : August 2014
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

#ifndef QGSTEXTSETTINGSWIDGET_H
#define QGSTEXTSETTINGSWIDGET_H

#include <QDialog>
#include <QFontDatabase>
#include <ui_qgstextsettingswidgetbase.h>

class QgsVectorLayer;
class QgsCharacterSelectorDialog;

#include "qgspallabeling.h"

class GUI_EXPORT QgsTextSettingsWidget : public QWidget, private Ui::QgsTextSettingsGuiBase
{
    Q_OBJECT

  public:
    QgsTextSettingsWidget( QWidget* parent = 0 );
    ~QgsTextSettingsWidget();

    void loadSettings( QgsTextRendererSettings *settings );

    void saveToSettings( QgsTextRendererSettings* settings );

    void setLayer( QgsVectorLayer* layer );

  private:
    QgsVectorLayer* mLayer;
    QFontDatabase mFontDB;
    QgsCharacterSelectorDialog* mCharDlg;

    QButtonGroup* mQuadrantBtnGrp;
    QButtonGroup* mDirectSymbBtnGrp;
    QButtonGroup* mUpsidedownBtnGrp;

    QButtonGroup* mPlacePointBtnGrp;
    QButtonGroup* mPlaceLineBtnGrp;
    QButtonGroup* mPlacePolygonBtnGrp;

    // background reference font
    QFont mRefFont;
    int mPreviewSize;

    int mMinPixelLimit;

    bool mLoadSvgParams;

    bool mLoadingSettings;

    void enableDataDefinedAlignment( bool enable );
    void toggleDDButtons( const bool visible );
    void blockInitSignals( bool block );
    void blockFontChangeSignals( bool blk );
    void setPreviewBackground( QColor color );
    void syncDefinedCheckboxFrame( QgsDataDefinedButton* ddBtn, QCheckBox* chkBx, QFrame* f );
    void populateFontCapitalsComboBox();
    void populateFontStyleComboBox();
    void populateDataDefinedButtons( QgsPalLayerSettings& s );
    /**Sets data defined property attribute to map */
    void setDataDefinedProperty( const QgsDataDefinedButton* ddBtn, QgsPalLayerSettings::DataDefinedProperties p, QgsPalLayerSettings& lyr );
    void updateFont( QFont font );

  private slots:
    void optionsStackedWidget_CurrentChanged( int indx );
    void showBackgroundRadius( bool show );
    void showBackgroundPenStyle( bool show );
    void on_mShapeSVGPathLineEdit_textChanged( const QString& text );
    void collapseSample( bool collapse );
    void updateUi();
    void updatePreview();
    void scrollPreview();
    void updatePlacementWidgets();
    void updateSvgWidgets( const QString& svgPath );
    void on_mPreviewSizeSlider_valueChanged( int i );
    void on_mFontSizeSpinBox_valueChanged( double d );
    void on_mFontCapitalsComboBox_currentIndexChanged( int index );
    void on_mFontFamilyCmbBx_currentFontChanged( const QFont& f );
    void on_mFontStyleComboBox_currentIndexChanged( const QString & text );
    void on_mFontUnderlineBtn_toggled( bool ckd );
    void on_mFontStrikethroughBtn_toggled( bool ckd );
    void on_mFontWordSpacingSpinBox_valueChanged( double spacing );
    void on_mFontLetterSpacingSpinBox_valueChanged( double spacing );
    void on_mFontSizeUnitWidget_changed();
    void on_mFontMinPixelSpinBox_valueChanged( int px );
    void on_mFontMaxPixelSpinBox_valueChanged( int px );
    void on_mBufferUnitWidget_changed();
    void on_mCoordXDDBtn_dataDefinedActivated( bool active );
    void on_mCoordYDDBtn_dataDefinedActivated( bool active );
    void on_mShapeTypeCmbBx_currentIndexChanged( int index );
    void on_mShapeRotationCmbBx_currentIndexChanged( int index );
    void on_mShapeSVGParamsBtn_clicked();
    void on_mShapeSVGSelectorBtn_clicked();
    void on_mPreviewTextEdit_textChanged( const QString & text );
    void on_mPreviewTextBtn_clicked();
    void on_mPreviewBackgroundBtn_colorChanged( const QColor &color );
    void on_mDirectSymbLeftToolBtn_clicked();
    void on_mDirectSymbRightToolBtn_clicked();

};

#endif //QGSTEXTSETTINGSWIDGET_H


