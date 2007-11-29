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
class QgsMapLayer;
class QgsRasterLayer;


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
        QgsRasterLayerProperties(QgsMapLayer *lyr, QWidget *parent = 0, Qt::WFlags = QgisGui::ModalDialogFlags);
        /** \brief Destructor */
        ~QgsRasterLayerProperties();

        /** synchronize state with associated raster layer */
        void sync();

    public slots:
        /** \brief Applies the settings made in the dialog without closing the box */
        void apply();
        /** \brief slot executed when the transparency level changes. */ 
        void sliderTransparency_valueChanged( int );
        /** \brief slot executed when the max red level changes. */
        void on_rbtnSingleBand_toggled( bool );
        /** \brief slot executed when the three band radio button is pressed. */
        void on_rbtnThreeBand_toggled( bool );
        /** \brief this slot asks the rasterlayer to construct pyramids */
        void on_buttonBuildPyramids_clicked();
        /** \brief slot executed when user wishes to refresh raster histogram */
        void on_pbnHistRefresh_clicked();
        /** Override the SRS specified when the layer was loaded */
        void on_pbnChangeSpatialRefSys_clicked();
        /** Help button */
        void on_buttonBox_helpRequested();
        
  signals:
    
        /** emitted when changes to layer were saved to update legend */
        void refreshLegend(QString layerID, bool expandItem);


    private:
        /** \brief Pointer to the raster layer that this property dilog changes the behaviour of. */
        QgsRasterLayer * rasterLayer;

        /** \brief If the underlying raster layer is of GDAL type (i.e. non-provider)

            This variable is used to determine if various parts of the Properties UI are
            included or not
         */
        bool rasterLayerIsGdal;

        /** \brief If the underlying raster layer is of WMS type (i.e. WMS data provider)

            This variable is used to determine if various parts of the Properties UI are
            included or not
         */
        bool rasterLayerIsWms;

        /** Id for context help */
        static const int context_id = 394441851;
};

#endif
