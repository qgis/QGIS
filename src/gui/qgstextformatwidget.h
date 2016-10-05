/***************************************************************************
    qgstextformatwidget.h
    ---------------------
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


#ifndef QGSTEXTFORMATWIDGET_H
#define QGSTEXTFORMATWIDGET_H

#include <ui_qgstextformatwidgetbase.h>
#include "qgstextrenderer.h"
#include "qgsstringutils.h"
#include "qgisgui.h"
#include <QFontDatabase>

class QgsMapCanvas;
class QgsCharacterSelectorDialog;


/** \class QgsTextFormatWidget
 * \ingroup gui
 * A widget for customising text formatting settings.
 *
 * QgsTextFormatWidget provides a widget for controlling the appearance of text rendered
 * using QgsTextRenderer. The widget includes all settings contained within
 * a QgsTextFormat, including shadow, background and buffer.
 *
 * Additionally, the widget can handle labeling settings due to the large overlap between
 * the text renderer settings and the labeling settings. This mode is possible by
 * subclassing QgsTextFormatWidget and calling the protected constructor with a mode
 * of Labeling.
 *
 * @note Added in QGIS 3.0
 */

class GUI_EXPORT QgsTextFormatWidget : public QWidget, protected Ui::QgsTextFormatWidgetBase
{
    Q_OBJECT
    Q_PROPERTY( QgsTextFormat format READ format )

  public:

    /** Constructor for QgsTextFormatWidget.
     * @param format initial formatting settings to show in widget
     * @param mapCanvas associated map canvas
     * @param parent parent widget
     */
    QgsTextFormatWidget( const QgsTextFormat& format = QgsTextFormat(), QgsMapCanvas* mapCanvas = nullptr, QWidget* parent = nullptr );

    ~QgsTextFormatWidget();

    /** Returns the current formatting settings defined by the widget.
     */
    QgsTextFormat format() const;

  public slots:

    /** Sets whether the widget should be shown in a compact dock mode.
     * @param enabled set to true to show in dock mode.
     */
    void setDockMode( bool enabled );

  signals:

    //! Emitted when the text format defined by the widget changes
    void widgetChanged();

  protected:

    //! Widget mode
    enum Mode
    {
      Text = 0, //!< Default mode, show text formatting settings only
      Labeling, //!< Show labeling settings in addition to text formatting settings
    };

    /** Constructor for QgsTextFormatWidget.
     * @param mapCanvas associated map canvas
     * @param parent parent widget
     * @param mode widget mode
     */
    QgsTextFormatWidget( QgsMapCanvas* mapCanvas, QWidget* parent, Mode mode );

    /** Updates the widget's state to reflect the settings in a QgsTextFormat.
     * @param format source format
     */
    void updateWidgetForFormat( const QgsTextFormat& format );

    /** Sets the background color for the text preview widget.
     * @param color background color
     */
    void setPreviewBackground( const QColor& color );

    /** Controls whether data defined alignment buttons are enabled.
     * @param enable set to true to enable alignment controls
     */
    void enableDataDefinedAlignment( bool enable );

    //! Text substitution list
    QgsStringReplacementCollection mSubstitutions;
    //! Quadrant button group
    QButtonGroup* mQuadrantBtnGrp;
    //! Symbol direction button group
    QButtonGroup* mDirectSymbBtnGrp;
    //! Upside down labels button group
    QButtonGroup* mUpsidedownBtnGrp;
    //! Point placement button group
    QButtonGroup* mPlacePointBtnGrp;
    //! Line placement button group
    QButtonGroup* mPlaceLineBtnGrp;
    //! Polygon placement button group
    QButtonGroup* mPlacePolygonBtnGrp;
    //! Pixel size font limit
    int mMinPixelLimit;

  protected slots:

    //! Updates line placement options to reflect current state of widget
    void updateLinePlacementOptions();

    //! Updates label placement options to reflect current state of widget
    void updatePlacementWidgets();

  private:
    Mode mWidgetMode;
    QgsMapCanvas* mMapCanvas;
    QgsCharacterSelectorDialog* mCharDlg;

    QFontDatabase mFontDB;

    // background reference font
    QFont mRefFont;
    bool mDockMode;

    bool mLoadSvgParams;

    void initWidget();
    void setWidgetMode( Mode mode );
    void toggleDDButtons( bool visible );
    void blockFontChangeSignals( bool blk );
    void populateFontCapitalsComboBox();
    void populateFontStyleComboBox();
    void updateFont( const QFont& font );
    void connectValueChanged( QList<QWidget*> widgets, const char* slot );

  private slots:
    void optionsStackedWidget_CurrentChanged( int indx );
    void showBackgroundRadius( bool show );
    void showBackgroundPenStyle( bool show );
    void on_mShapeSVGPathLineEdit_textChanged( const QString& text );
    void onSubstitutionsChanged( const QgsStringReplacementCollection& substitutions );
    void previewScaleChanged( double scale );
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
    void on_mChkNoObstacle_toggled( bool active );
    void on_chkLineOrientationDependent_toggled( bool active );
    void on_mToolButtonConfigureSubstitutes_clicked();
    void collapseSample( bool collapse );
    void changeTextColor( const QColor &color );
    void changeBufferColor( const QColor &color );
    void updatePreview();
    void scrollPreview();
    void updateSvgWidgets( const QString& svgPath );
};


/** \class QgsTextFormatDialog
 * \ingroup gui
 * A simple dialog for customising text formatting settings.
 *
 * QgsTextFormatDialog provides a dialog for controlling the appearance of text rendered
 * using QgsTextRenderer. The dialog includes all settings contained within
 * a QgsTextFormat, including shadow, background and buffer.
 *
 * @note Added in QGIS 3.0
 */

class GUI_EXPORT QgsTextFormatDialog : public QDialog
{
    Q_OBJECT

  public:

    /** Constructor for QgsTextFormatDialog.
     * @param format initial format settings to show in dialog
     * @param mapCanvas optional associated map canvas
     * @param parent parent widget
     * @param fl window flags for dialog
     */
    QgsTextFormatDialog( const QgsTextFormat& format, QgsMapCanvas* mapCanvas = nullptr, QWidget* parent = nullptr, Qt::WindowFlags fl = QgisGui::ModalDialogFlags );

    virtual ~QgsTextFormatDialog();

    /** Returns the current formatting settings defined by the widget.
     */
    QgsTextFormat format() const;

  private:

    QgsTextFormatWidget* mFormatWidget;
};

#endif //QGSTEXTFORMATWIDGET_H


