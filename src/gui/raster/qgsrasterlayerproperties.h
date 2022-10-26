
/**
 * \brief The qgsrasterlayerproperties class is used to set up how raster layers are displayed.
 */
/* **************************************************************************
                          qgsrasterlayerproperties.h  -  description
                             -------------------
    begin                : Sun Aug 11 2002
    copyright            : (C) 2002 by Tim Sutton
    email                : tim@linfiniti.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSRASTERLAYERPROPERTIES_H
#define QGSRASTERLAYERPROPERTIES_H

#include "qgsoptionsdialogbase.h"
#include "ui_qgsrasterlayerpropertiesbase.h"
#include "qgsguiutils.h"
#include "qgshelp.h"
#include "qgsmaplayerstylemanager.h"
#include "qgsmaptoolemitpoint.h"
#include "qgis_gui.h"
#include "qgsresamplingutils.h"
#include "qgsrasterpipe.h"
#include "qgsexpressioncontextgenerator.h"

class QgsPointXY;
class QgsMapLayer;
class QgsMapCanvas;
class QgsRasterLayer;
class QgsMetadataWidget;
class QgsRasterRenderer;
class QgsRasterRendererWidget;
class QgsRasterHistogramWidget;
class QgsRasterLayerTemporalPropertiesWidget;
class QgsWebView;
class QgsProviderSourceWidget;
class QgsMapLayerConfigWidgetFactory;
class QgsMapLayerConfigWidget;
class QgsPropertyOverrideButton;
class QgsRasterTransparencyWidget;
class QgsRasterAttributeTableWidget;


/**
 * \ingroup gui
 * \class QgsRasterLayerProperties
 * \brief Property sheet for a raster map layer
 * \since QGIS 3.12 (in the GUI API)
 */

class GUI_EXPORT QgsRasterLayerProperties : public QgsOptionsDialogBase, private Ui::QgsRasterLayerPropertiesBase, private QgsExpressionContextGenerator
{
    Q_OBJECT

  public:

    /**
     * enumeration for the different types of style
     */
#ifndef SIP_RUN
    enum StyleType
    {
      QML,
      SLD
    };
    Q_ENUM( StyleType )
#endif

    /**
     * Constructor
     * \param lyr Map layer for which properties will be displayed
     * \param canvas the QgsMapCanvas instance
     * \param parent the parent of this widget
     * \param fl windows flag
     */
    QgsRasterLayerProperties( QgsMapLayer *lyr, QgsMapCanvas *canvas, QWidget *parent = nullptr, Qt::WindowFlags = QgsGuiUtils::ModalDialogFlags );

    /**
     * Adds a properties page factory to the raster layer properties dialog.
     * \since QGIS 3.18
     */
    void addPropertiesPageFactory( const QgsMapLayerConfigWidgetFactory *factory );

    QgsExpressionContext createExpressionContext() const override;

  protected slots:
    //! \brief auto slot executed when the active page in the main widget stack is changed
    void optionsStackedWidget_CurrentChanged( int index ) override SIP_SKIP ;

  private:

    // TODO -- consider moving these to a common raster widget base class

    /**
     * Registers a property override button, setting up its initial value, connections and description.
     * \param button button to register
     * \param key corresponding data defined property key
     * \since QGIS 3.22
     */
    void initializeDataDefinedButton( QgsPropertyOverrideButton *button, QgsRasterPipe::Property key );

    /**
     * Updates all property override buttons to reflect the widgets's current properties.
     * \since QGIS 3.22
     */
    void updateDataDefinedButtons();

    /**
     * Updates a specific property override \a button to reflect the widgets's current properties.
     * \since QGIS 3.22
     */
    void updateDataDefinedButton( QgsPropertyOverrideButton *button );

    //! Temporary property collection
    QgsPropertyCollection mPropertyCollection;

  private slots:

    void updateProperty();

    //! \brief Applies the settings made in the dialog without closing the box
    void apply();
    //! \brief Called when cancel button is pressed
    void onCancel();
    //! \brief this slot asks the rasterlayer to construct pyramids
    void buttonBuildPyramids_clicked();
    //! \brief slot executed when user changes the layer's CRS
    void mCrsSelector_crsChanged( const QgsCoordinateReferenceSystem &crs );

    // Server properties
    void addMetadataUrl();
    void removeSelectedMetadataUrl();

    /**
     * updates gamma spinbox on slider changes
     * \since QGIS 3.16
     */
    void updateGammaSpinBox( int value );

    /**
     * updates gamma slider on spinbox changes
     * \since QGIS 3.16
     */
    void updateGammaSlider( double value );

    void mRenderTypeComboBox_currentIndexChanged( int index );
    //! Load the default style when appropriate button is pressed.
    void loadDefaultStyle_clicked();
    //! Save the default style when appropriate button is pressed.
    void saveDefaultStyle_clicked();
    //! Load a saved style when appropriate button is pressed.
    void loadStyle_clicked();
    //! Save a style when appriate button is pressed.
    void saveStyleAs_clicked();
    //! Restore dialog modality and focus, usually after a pixel clicked to pick transparency color
    void restoreWindowModality();


    //! Load a saved metadata file.
    void loadMetadata();
    //! Save a metadata.
    void saveMetadataAs();
    //! Save the default metadata.
    void saveDefaultMetadata();
    //! Load the default metadata.
    void loadDefaultMetadata();

    //! Help button
    void showHelp();

    //! Slot to reset all color rendering options to default
    void mResetColorRenderingBtn_clicked();

    //! Enable or disable Build pyramids button depending on selection in pyramids list
    void toggleBuildPyramidsButton();

    //! Enable or disable saturation controls depending on choice of grayscale mode
    void toggleSaturationControls( int grayscaleMode );

    //! Enable or disable colorize controls depending on checkbox
    void toggleColorizeControls( bool colorizeEnabled );

    //! Transparency cell changed
    void transparencyCellTextEdited( const QString &text );

    void aboutToShowStyleMenu();

    //! Make GUI reflect the layer's state
    void syncToLayer();

    void urlClicked( const QUrl &url );

  private:
    QPushButton *mBtnStyle = nullptr;
    QPushButton *mBtnMetadata = nullptr;
    QAction *mActionLoadMetadata = nullptr;
    QAction *mActionSaveMetadataAs = nullptr;

    QStandardItemModel *mMetadataUrlModel = nullptr;

    //! A list of additional pages provided by plugins
    QList<QgsMapLayerConfigWidget *> mLayerPropertiesPages;

    //! \brief  A constant that signals property not used
    const QString TRSTRING_NOT_SET;

    //! \brief Default contrast enhancement algorithm
    QString mDefaultContrastEnhancementAlgorithm;

    //! \brief default standard deviation
    double mDefaultStandardDeviation;

    //! \brief Default band combination
    int mDefaultRedBand;
    int mDefaultGreenBand;
    int mDefaultBlueBand;

    //! \brief Flag to indicate if Gray minimum maximum values are actual minimum maximum values
    bool mGrayMinimumMaximumEstimated;

    //! \brief Flag to indicate if RGB minimum maximum values are actual minimum maximum values
    bool mRGBMinimumMaximumEstimated;

    //! \brief Pointer to the raster layer that this property dilog changes the behavior of.
    QgsRasterLayer *mRasterLayer = nullptr;

    QgsRasterRendererWidget *mRendererWidget = nullptr;
    QgsMetadataWidget *mMetadataWidget = nullptr;

    QgsRasterTransparencyWidget *mRasterTransparencyWidget = nullptr;

    /**
     * Widget with temporal inputs, to be used by temporal based raster layers.
     */
    QgsRasterLayerTemporalPropertiesWidget *mTemporalWidget = nullptr;

    bool rasterIsMultiBandColor();

    /**
     * Updates the information tab by reloading metadata
     */
    void updateInformationContent();

    void setTransparencyCell( int row, int column, double value );
    void setTransparencyCellValue( int row, int column, double value );
    double transparencyCellValue( int row, int column );
    void setTransparencyToEdited( int row );
    void adjustTransparencyCellWidth( int row, int column );

    void setRendererWidget( const QString &rendererName );

    /**
     * Setup or update the raster attribute table options page.
     */
    void updateRasterAttributeTableOptionsPage();

    //TODO: we should move these gradient generators somewhere more generic
    //so they can be used generically throughout the app
    QLinearGradient greenGradient();
    QLinearGradient redGradient();
    QLinearGradient blueGradient();
    QLinearGradient grayGradient();
    QLinearGradient highlightGradient();
    qreal mGradientHeight;
    qreal mGradientWidth;

    QgsMapCanvas *mMapCanvas = nullptr;

    QgsRasterHistogramWidget *mHistogramWidget = nullptr;

    QVector<bool> mTransparencyToEdited;

    /**
     * Previous layer style. Used to reset style to previous state if new style
     * was loaded but dialog is canceled.
    */
    QgsMapLayerStyle mOldStyle;

    bool mDisableRenderTypeComboBoxCurrentIndexChanged = false;

    bool mMetadataFilled;

    //! Synchronize state with associated raster layer
    void sync();

    QgsResamplingUtils mResamplingUtils;

    QgsProviderSourceWidget *mSourceWidget = nullptr;

    QgsWebView *mMetadataViewer = nullptr;

    QgsExpressionContext mContext;

    friend class QgsAppScreenShots;

    QgsCoordinateReferenceSystem mBackupCrs;

    QgsRasterAttributeTableWidget *mRasterAttributeTableWidget = nullptr;
};
#endif
