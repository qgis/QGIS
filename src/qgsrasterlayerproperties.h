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
#include "qgsrasterlayerpropertiesbase.uic.h"
#include "qgsmaplayer.h"
#include "qgsrasterlayer.h"


/**Property sheet for a raster map layer
  *@author Tim Sutton
  */

class QgsRasterLayerProperties : public QgsRasterLayerPropertiesBase  
{
  Q_OBJECT
    public:
        /** \brief Constructor
         * @param ml Map layer for which properties will be displayed
         */
        QgsRasterLayerProperties(QgsMapLayer *lyr);
        /** \brief Destructor */
        ~QgsRasterLayerProperties();
        /** \brief Applies the settings made in the dialog without closing the box */
        void apply();
        /** \bried Apply the settings made and close the dialog. */
        void accept();
        /** \brief slot executed when the transparency level changes. */ 
	void sliderTransparency_valueChanged( int );
        /** \brief slot executed when the max red level changes. */
        void sliderMaxRed_valueChanged( int );
        /** \brief slot executed when the min red level changes. */
        void sliderMinRed_valueChanged( int );
        /** \brief slot executed when the max blue level changes. */
        void sliderMaxBlue_valueChanged( int );
        /** \brief slot executed when the max blue level changes. */
        void sliderMinBlue_valueChanged( int );
        /** \brief slot executed when the max green level changes. */
        void sliderMaxGreen_valueChanged( int );
        /** \brief slot executed when the min green level changes. */
        void sliderMinGreen_valueChanged( int );
        /** \brief slot executed when the max gray level changes. */
        void sliderMaxGray_valueChanged( int );
        /** \brief slot executed when the min gray level changes. */
        void sliderMinGray_valueChanged( int );
        /** \brief slot executed when the single band radio button is pressed. */
        void rbtnSingleBand_toggled( bool );
        /** \brief slot executed when the three band radio button is pressed. */
        void rbtnThreeBand_toggled( bool );
    private:
        /** \brief Private function to populate the statistics table from the band stats. */
        void fillStatsTable();
        /** \brief This function makes a pixmap to display in the color box */
        void makeScalePreview(QString theColor);
        /** \brief Pointer to the raster layer that this property dilog changes the behaviour of. */
        QgsRasterLayer * rasterLayer;
};

#endif
