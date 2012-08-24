/***************************************************************************
  qgslabelinggui.h
  Smart labeling for vector layers
  -------------------
         begin                : June 2009
         copyright            : (C) Martin Dobias
         email                : wonder.sk at gmail.com

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QgsLabelingGUI_H
#define QgsLabelingGUI_H

#include <QDialog>
#include <QFontDatabase>
#include <ui_qgslabelingguibase.h>

class QgsVectorLayer;
class QgsMapCanvas;

#include "qgspallabeling.h"

class QgsLabelingGui : public QWidget, private Ui::QgsLabelingGuiBase
{
    Q_OBJECT

  public:
    QgsLabelingGui( QgsPalLabeling *lbl, QgsVectorLayer* layer, QgsMapCanvas* mapCanvas, QWidget* parent );
    ~QgsLabelingGui();

    QgsPalLayerSettings layerSettings();

  public slots:
    void apply();
    void changeTextColor();
    void changeTextFont();
    void showEngineConfigDialog();
    void showExpressionDialog();
    void changeBufferColor();

    void updateUi();
    void updatePreview();
    void scrollPreview();
    void updateOptions();

    void on_mPreviewSizeSlider_valueChanged( int i );
    void on_mFontSizeSpinBox_valueChanged( double d );
    void on_mFontStyleComboBox_currentIndexChanged( const QString & text );
    void on_mFontUnderlineBtn_toggled( bool ckd );
    void on_mFontStrikethroughBtn_toggled( bool ckd );
    void on_mFontWordSpacingSpinBox_valueChanged( double spacing );
    void on_mFontLetterSpacingSpinBox_valueChanged( double spacing );
    void on_mBufferUnitComboBox_currentIndexChanged( int index );
    void on_mXCoordinateComboBox_currentIndexChanged( const QString & text );
    void on_mYCoordinateComboBox_currentIndexChanged( const QString & text );

    void on_mPreviewTextEdit_textChanged( const QString & text );
    void on_mPreviewTextBtn_clicked();
    void on_mPreviewBackgroundBtn_clicked();

  protected:
    void blockFontChangeSignals( bool blk );
    void setPreviewBackground( QColor color );
    void updateFontViaStyle( const QString & fontstyle );
    void populateFontStyleComboBox();
    void populatePlacementMethods();
    void populateFieldNames();
    void populateDataDefinedCombos( QgsPalLayerSettings& s );
    /**Sets data defined property attribute to map (if selected in combo box)*/
    void setDataDefinedProperty( const QComboBox* c, QgsPalLayerSettings::DataDefinedProperties p, QgsPalLayerSettings& lyr );
    void setCurrentComboValue( QComboBox* c, const QgsPalLayerSettings& s, QgsPalLayerSettings::DataDefinedProperties p );
    void updateFont( QFont font );

  private:
    QgsPalLabeling* mLBL;
    QgsVectorLayer* mLayer;
    QgsMapCanvas* mMapCanvas;
    QFontDatabase mFontDB;

    // background reference font
    QFont mRefFont;
    int mPreviewSize;

    void disableDataDefinedAlignment();
    void enableDataDefinedAlignment();
};

#endif
