/** \brief The qgsrasterlayerproperties class is used to set up how raster layers are displayed.
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
/* $Id$ */
#ifndef QGSRASTERLAYERPROPERTIES_H
#define QGSRASTERLAYERPROPERTIES_H

#include "ui_qgsrasterlayerpropertiesbase.h"
#include "qgisgui.h"
#include "qgsmaptool.h"
#include "qgscolorrampshader.h"
#include "qgscontexthelp.h"

class QgsMapLayer;
class QgsMapCanvas;
class QgsRasterLayer;
class QgsPixelSelectorTool;

/**Property sheet for a raster map layer
  *@author Tim Sutton
  */

class QgsRasterLayerProperties : public QDialog, private Ui::QgsRasterLayerPropertiesBase
{
    Q_OBJECT

  public:
    /** \brief Constructor
     * @param ml Map layer for which properties will be displayed
     */
    QgsRasterLayerProperties( QgsMapLayer *lyr, QgsMapCanvas* theCanvas, QWidget *parent = 0, Qt::WFlags = QgisGui::ModalDialogFlags );
    /** \brief Destructor */
    ~QgsRasterLayerProperties();

    /** synchronize state with associated raster layer */
    void sync();

  public slots:
    //TODO: Verify that these all need to be public
    /** \brief Applies the settings made in the dialog without closing the box */
    void apply();
    /** \brief this slot asks the rasterlayer to construct pyramids */
    void on_buttonBuildPyramids_clicked();
    /** \brief slot executed when user presses "Add Values From Display" button on the transparency page */
    void on_pbnAddValuesFromDisplay_clicked();
    /** \brief slot executed when user presses "Add Values Manually" button on the transparency page */
    void on_pbnAddValuesManually_clicked();
    /** Override the CRS specified when the layer was loaded */
    void on_pbnChangeSpatialRefSys_clicked();
    /** \brief slot executed when user wishes to reset noNoDataValue and transparencyTable to default value */
    void on_pbnDefaultValues_clicked();
    /** \brief slot executed when user wishes to export transparency values */
    void on_pbnExportTransparentPixelValues_clicked();
    /** \brief auto slot executed when the active page in the main widget stack is changed */
    void on_tabBar_currentChanged( int theTab );
    /** \brief slot executed when user wishes to refresh raster histogram */
    void refreshHistogram();
    /** \brief slow executed when user wishes to import transparency values */
    void on_pbnImportTransparentPixelValues_clicked();
    /** \brief slot executed when user presses "Remove Selected Row" button on the transparency page */
    void on_pbnRemoveSelectedRow_clicked();
    /** \brief slot executed when the single band radio button is pressed. */
    void on_rbtnSingleBand_toggled( bool );
    /** \brief slot executed when the single band min max radio button is pressed. */
    void on_rbtnSingleBandMinMax_toggled( bool );
    /** \brief slot executed when the single band standard deviation radio button is pressed. */
    void on_rbtnSingleBandStdDev_toggled( bool );
    /** \brief slot executed when the three band radio button is pressed. */
    void on_rbtnThreeBand_toggled( bool );
    /** \brief slot executed when the three band min max radio button is pressed. */
    void on_rbtnThreeBandMinMax_toggled( bool );
    /** \brief slot executed when the three band standard deviation radio button is pressed. */
    void on_rbtnThreeBandStdDev_toggled( bool );

    void pixelSelected( int x, int y );
    /** \brief this slot clears min max values from gui */
    void sboxSingleBandStdDev_valueChanged( double );
    /** \brief this slot clears min max values from gui */
    void sboxThreeBandStdDev_valueChanged( double );
    /** \brief slot executed when the transparency level changes. */
    void sliderTransparency_valueChanged( int );
    /** \brief this slot sets StdDev switch box to 0.00 when user enters min max values */
    void userDefinedMinMax_textEdited( QString );

  private slots:
    /** This slow handles necessary interface modifications (i.e. loading min max values) */
    void on_cboBlue_currentIndexChanged( const QString& );
    /** This slow handles necessary interface modifications (i.e. loading min max values) */
    void on_cboGray_currentIndexChanged( const QString& );
    /** This slow handles necessary interface modifications (i.e. loading min max values) */
    void on_cboGreen_currentIndexChanged( const QString& );
    /** This slow handles necessary interface modifications (i.e. loading min max values) */
    void on_cboRed_currentIndexChanged( const QString& );
    /** This slot handles necessary interface modifications based when color map selected changes */
    void on_cboxColorMap_currentIndexChanged( const QString& );
    /** This slot calculates classification values and colors for the tree widget on the colormap tab */
    void on_mClassifyButton_clicked();
    /** This slot deletes the current class from the tree widget on the colormap tab */
    void on_mDeleteEntryButton_clicked();
    /** Callback for double clicks on the colormap entry widget */
    void handleColormapTreeWidgetDoubleClick( QTreeWidgetItem* item, int column );
    /** This slot adds a new row to the color map table */
    void on_pbtnAddColorMapEntry_clicked();
    /** This slots saves the current color map to a file */
    void on_pbtnExportColorMapToFile_clicked();
    /** This slots loads the current color map from a band */
    void on_pbtnLoadColorMapFromBand_clicked();
    /** This slots loads the current color map from a file */
    void on_pbtnLoadColorMapFromFile_clicked();
    /** This slot loads the minimum and maximum values from the raster band and updates the gui */
    void on_pbtnLoadMinMax_clicked();
    /** This slot sets the default band combination variable to current band combination */
    void on_pbtnMakeBandCombinationDefault_clicked();
    /** This slot sets the default contrast enhancement variable  to current contrast enhancement algorithm */
    void on_pbtnMakeContrastEnhancementAlgorithmDefault_clicked();
    /** This slot sets the standard deviation default */
    void on_pbtnMakeStandardDeviationDefault_clicked();
    /** This slot will sort the color map in ascending order */
    void on_pbtnSortColorMap_clicked();
    /** Load the default style when appropriate button is pressed. */
    void on_pbnLoadDefaultStyle_clicked();
    /** Save the default style when appropriate button is pressed. */
    void on_pbnSaveDefaultStyle_clicked();
    /** Load a saved style when appropriate button is pressed. */
    void on_pbnLoadStyle_clicked();
    /** Save a style when appriate button is pressed. */
    void on_pbnSaveStyleAs_clicked();
    /** Help button */
    void on_buttonBox_helpRequested() { QgsContextHelp::run( metaObject()->className() ); }
    /** This slot lets you save the histogram as an image to disk */
    void on_mSaveAsImageButton_clicked();

  signals:

    /** emitted when changes to layer were saved to update legend */
    void refreshLegend( QString layerID, bool expandItem );


  private:
    /** \brief  A constant that signals property not used */
    const QString TRSTRING_NOT_SET;

    /** \brief Default contrast enhancement algorithm */
    QString mDefaultContrastEnhancementAlgorithm;

    /** \brief default standard deviation */
    double mDefaultStandardDeviation;

    /** \brief Default band combination */
    int mDefaultRedBand;
    int mDefaultGreenBand;
    int mDefaultBlueBand;

    /** \brief Internal flag used to short circuit signal loop between min max field and stdDev spin box */
    bool ignoreSpinBoxEvent;

    /** \brief Flag to indicate if Gray minimum maximum values are actual minimum maximum values */
    bool mGrayMinimumMaximumEstimated;

    /** \brief Flag to indicate if RGB minimum maximum values are actual minimum maximum values */
    bool mRGBMinimumMaximumEstimated;

    /** \brief Pointer to the raster layer that this property dilog changes the behaviour of. */
    QgsRasterLayer * mRasterLayer;

    /** \brief If the underlying raster layer is of GDAL type (i.e. non-provider)

        This variable is used to determine if various parts of the Properties UI are
        included or not
     */
    bool mRasterLayerIsGdal;

    /** \brief If the underlying raster layer is of WMS type (i.e. WMS data provider)

        This variable is used to determine if various parts of the Properties UI are
        included or not
     */
    bool mRasterLayerIsWms;

    /** \brief Clear current color map table and population with values from new list */
    void populateColorMapTable( const QList<QgsColorRampShader::ColorRampItem>& );

    /** \brief Clear the current transparency table and populate the table with the correct types for current drawing mode and data type*/
    void populateTransparencyTable();

    /** \brief Set the message indicating if any min max values are estimates */
    void setMinimumMaximumEstimateWarning();

    /**Restores the state of the colormap tab*/
    void syncColormapTab();

    /** \brief Verify values in custom min max line edits */
    bool validUserDefinedMinMax();

    //@TODO we should move these gradient generators somewhere more generic
    //so they can be used generically throughut the app
    QLinearGradient greenGradient();
    QLinearGradient redGradient();
    QLinearGradient blueGradient();
    QLinearGradient grayGradient();
    QLinearGradient highlightGradient();
    qreal mGradientHeight;
    qreal mGradientWidth;

    QgsMapCanvas* mMapCanvas;
    QgsPixelSelectorTool* mPixelSelectorTool;
};

/**
  *Simple map tool for selecting pixels, specific to QgsRasterLayerProperties
  */
class QgsPixelSelectorTool: public QgsMapTool
{
    Q_OBJECT

  public:
    QgsPixelSelectorTool( QgsMapCanvas* );
    ~QgsPixelSelectorTool( );

    /** \brief Method to handle mouse release, i.e., select, event */
    void canvasReleaseEvent( QMouseEvent* theMouseEvent );

  signals:
    /** \brief Alter the listener ( raster properties dialog ) that a mouse click was registered */
    void pixelSelected( int x, int y );

  private:
    QgsMapCanvas * mMapCanvas;
};

#endif
