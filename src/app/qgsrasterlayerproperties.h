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
class QgsMapToolEmitPoint;
class QgsRasterRendererWidget;

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
    /** \brief slot executed when the reset null value to file default icon is selected */
    void on_btnResetNull_clicked( );

    void pixelSelected( const QgsPoint& );
    /** \brief slot executed when the transparency level changes. */
    void sliderTransparency_valueChanged( int );

  private slots:
    void on_mRenderTypeComboBox_currentIndexChanged( int index );
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
    /**Enable or disable Build pyramids button depending on selection in pyramids list*/
    void toggleBuildPyramidsButton();

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

    /** \brief Flag to indicate if Gray minimum maximum values are actual minimum maximum values */
    bool mGrayMinimumMaximumEstimated;

    /** \brief Flag to indicate if RGB minimum maximum values are actual minimum maximum values */
    bool mRGBMinimumMaximumEstimated;

    /** \brief Pointer to the raster layer that this property dilog changes the behaviour of. */
    QgsRasterLayer * mRasterLayer;

    /** \brief If the underlying raster layer doesn't have a provider

        This variable is used to determine if various parts of the Properties UI are
        included or not
     */
    //bool mRasterLayerIsInternal;

    QgsRasterRendererWidget* mRendererWidget;

    /** \brief Clear the current transparency table and populate the table with the correct types for current drawing mode and data type*/
    void populateTransparencyTable();

    void setRendererWidget( const QString& rendererName );

    //@TODO we should move these gradient generators somewhere more generic
    //so they can be used generically throughut the app
    QLinearGradient greenGradient();
    QLinearGradient redGradient();
    QLinearGradient blueGradient();
    QLinearGradient grayGradient();
    QLinearGradient highlightGradient();
    qreal mGradientHeight;
    qreal mGradientWidth;

    /** Update pipe tab - filters list */
    void updatePipeList();

    QgsMapCanvas* mMapCanvas;
    QgsMapToolEmitPoint* mPixelSelectorTool;
};
#endif
