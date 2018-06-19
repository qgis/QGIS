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

#include "ui_qgstextformatwidgetbase.h"
#include "qgis.h"
#include "qgstextrenderer.h"
#include "qgsstringutils.h"
#include "qgsguiutils.h"
#include <QFontDatabase>
#include "qgis_gui.h"

class QgsMapCanvas;
class QgsCharacterSelectorDialog;


/**
 * \class QgsTextFormatWidget
 * \ingroup gui
 * A widget for customizing text formatting settings.
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
 * \since QGIS 3.0
 */

class GUI_EXPORT QgsTextFormatWidget : public QWidget, protected Ui::QgsTextFormatWidgetBase
{
    Q_OBJECT
    Q_PROPERTY( QgsTextFormat format READ format )

  public:

    /**
     * Constructor for QgsTextFormatWidget.
     * \param format initial formatting settings to show in widget
     * \param mapCanvas associated map canvas
     * \param parent parent widget
     */
    QgsTextFormatWidget( const QgsTextFormat &format = QgsTextFormat(), QgsMapCanvas *mapCanvas = nullptr, QWidget *parent SIP_TRANSFERTHIS = nullptr );

    ~QgsTextFormatWidget() override;

    /**
     * Returns the current formatting settings defined by the widget.
     */
    QgsTextFormat format() const;

    /**
     * Sets the current formatting settings
     * \since QGIS 3.2
     */
    void setFormat( const QgsTextFormat &format );

  public slots:

    /**
     * Sets whether the widget should be shown in a compact dock mode.
     * \param enabled set to true to show in dock mode.
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

    /**
     * Constructor for QgsTextFormatWidget.
     * \param mapCanvas associated map canvas
     * \param parent parent widget
     * \param mode widget mode
     */
    QgsTextFormatWidget( QgsMapCanvas *mapCanvas, QWidget *parent SIP_TRANSFERTHIS, Mode mode );

    /**
     * Updates the widget's state to reflect the settings in a QgsTextFormat.
     * \param format source format
     */
    void updateWidgetForFormat( const QgsTextFormat &format );

    /**
     * Sets the background color for the text preview widget.
     * \param color background color
     */
    void setPreviewBackground( const QColor &color );

    /**
     * Controls whether data defined alignment buttons are enabled.
     * \param enable set to true to enable alignment controls
     */
    void enableDataDefinedAlignment( bool enable );

    //! Text substitution list
    QgsStringReplacementCollection mSubstitutions;
    //! Quadrant button group
    QButtonGroup *mQuadrantBtnGrp = nullptr;
    //! Symbol direction button group
    QButtonGroup *mDirectSymbBtnGrp = nullptr;
    //! Upside down labels button group
    QButtonGroup *mUpsidedownBtnGrp = nullptr;
    //! Point placement button group
    QButtonGroup *mPlacePointBtnGrp = nullptr;
    //! Line placement button group
    QButtonGroup *mPlaceLineBtnGrp = nullptr;
    //! Polygon placement button group
    QButtonGroup *mPlacePolygonBtnGrp = nullptr;
    //! Pixel size font limit
    int mMinPixelLimit = 0;

  protected slots:

    //! Updates line placement options to reflect current state of widget
    void updateLinePlacementOptions();

    //! Updates label placement options to reflect current state of widget
    void updatePlacementWidgets();

  private:
    Mode mWidgetMode = Text;
    QgsMapCanvas *mMapCanvas = nullptr;
    QgsCharacterSelectorDialog *mCharDlg = nullptr;
    std::unique_ptr< QgsPaintEffect > mBufferEffect;
    std::unique_ptr< QgsPaintEffect > mBackgroundEffect;

    QFontDatabase mFontDB;

    // background reference font
    QFont mRefFont;
    bool mDockMode;

    bool mLoadSvgParams = false;

    void initWidget();
    void setWidgetMode( Mode mode );
    void toggleDDButtons( bool visible );
    void blockFontChangeSignals( bool blk );
    void populateFontCapitalsComboBox();
    void populateFontStyleComboBox();
    void updateFont( const QFont &font );
    void connectValueChanged( const QList<QWidget *> &widgets, const char *slot );

  private slots:
    void optionsStackedWidget_CurrentChanged( int indx );
    void showBackgroundRadius( bool show );
    void showBackgroundPenStyle( bool show );
    void mShapeSVGPathLineEdit_textChanged( const QString &text );
    void onSubstitutionsChanged( const QgsStringReplacementCollection &substitutions );
    void previewScaleChanged( double scale );
    void mFontSizeSpinBox_valueChanged( double d );
    void mFontCapitalsComboBox_currentIndexChanged( int index );
    void mFontFamilyCmbBx_currentFontChanged( const QFont &f );
    void mFontStyleComboBox_currentIndexChanged( const QString &text );
    void mFontUnderlineBtn_toggled( bool ckd );
    void mFontStrikethroughBtn_toggled( bool ckd );
    void mFontWordSpacingSpinBox_valueChanged( double spacing );
    void mFontLetterSpacingSpinBox_valueChanged( double spacing );
    void mFontSizeUnitWidget_changed();
    void mFontMinPixelSpinBox_valueChanged( int px );
    void mFontMaxPixelSpinBox_valueChanged( int px );
    void mBufferUnitWidget_changed();
    void mCoordXDDBtn_activated( bool active );
    void mCoordYDDBtn_activated( bool active );
    void mShapeTypeCmbBx_currentIndexChanged( int index );
    void mShapeRotationCmbBx_currentIndexChanged( int index );
    void mShapeSVGParamsBtn_clicked();
    void mShapeSVGSelectorBtn_clicked();
    void mPreviewTextEdit_textChanged( const QString &text );
    void mPreviewTextBtn_clicked();
    void mPreviewBackgroundBtn_colorChanged( const QColor &color );
    void mDirectSymbLeftToolBtn_clicked();
    void mDirectSymbRightToolBtn_clicked();
    void mChkNoObstacle_toggled( bool active );
    void chkLineOrientationDependent_toggled( bool active );
    void mToolButtonConfigureSubstitutes_clicked();
    void collapseSample( bool collapse );
    void changeTextColor( const QColor &color );
    void changeBufferColor( const QColor &color );
    void updatePreview();
    void scrollPreview();
    void updateSvgWidgets( const QString &svgPath );
};


/**
 * \class QgsTextFormatDialog
 * \ingroup gui
 * A simple dialog for customizing text formatting settings.
 *
 * QgsTextFormatDialog provides a dialog for controlling the appearance of text rendered
 * using QgsTextRenderer. The dialog includes all settings contained within
 * a QgsTextFormat, including shadow, background and buffer.
 *
 * \since QGIS 3.0
 */

class GUI_EXPORT QgsTextFormatDialog : public QDialog
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsTextFormatDialog.
     * \param format initial format settings to show in dialog
     * \param mapCanvas optional associated map canvas
     * \param parent parent widget
     * \param fl window flags for dialog
     */
    QgsTextFormatDialog( const QgsTextFormat &format, QgsMapCanvas *mapCanvas = nullptr, QWidget *parent SIP_TRANSFERTHIS = nullptr, Qt::WindowFlags fl = QgsGuiUtils::ModalDialogFlags );

    ~QgsTextFormatDialog() override;

    /**
     * Returns the current formatting settings defined by the widget.
     */
    QgsTextFormat format() const;

  private:

    QgsTextFormatWidget *mFormatWidget = nullptr;
};

/**
 * \class QgsTextFormatPanelWidget
 * \ingroup gui
 * A panel widget for customizing text formatting settings.
 *
 * QgsTextFormatPanelWidget provides a panel widget for controlling the appearance of text rendered
 * using QgsTextRenderer. The dialog includes all settings contained within
 * a QgsTextFormat, including shadow, background and buffer.
 *
 * \since QGIS 3.0
 */

class GUI_EXPORT QgsTextFormatPanelWidget : public QgsPanelWidgetWrapper
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsTextFormatPanelWidget.
     * \param format initial format settings to show in dialog
     * \param mapCanvas optional associated map canvas
     * \param parent parent widget
     */
    QgsTextFormatPanelWidget( const QgsTextFormat &format, QgsMapCanvas *mapCanvas = nullptr, QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Returns the current formatting settings defined by the widget.
     */
    QgsTextFormat format() const;

    void setDockMode( bool dockMode ) override;

  private:

    QgsTextFormatWidget *mFormatWidget = nullptr;
};

#endif //QGSTEXTFORMATWIDGET_H


