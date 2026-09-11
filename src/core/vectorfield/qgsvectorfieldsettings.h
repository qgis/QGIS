/***************************************************************************
    qgsvectorfieldsettings.h
    ---------------------
    begin                : September 2026
    copyright            : (C) 2026 by Stefanos Natsis
    email                : uclaros at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSVECTORFIELDSETTINGS_H
#define QGSVECTORFIELDSETTINGS_H

#include "qgis.h"
#include "qgis_core.h"
#include "qgscolorrampshader.h"
#include "qgsinterpolatedlinerenderer.h"
#include "qgsvectorfieldarrowsettings.h"
#include "qgsvectorfieldstreamlinesettings.h"
#include "qgsvectorfieldtracessettings.h"
#include "qgsvectorfieldwindbarbsettings.h"

#include <QColor>
#include <QDomElement>

/**
 * \ingroup core
 *
 * \brief Represents a renderer settings for vector datasets.
 *
 * \note The API is considered EXPERIMENTAL and can be changed without a notice
 *
 * \since QGIS 4.4
 */
class CORE_EXPORT QgsVectorFieldSettings
{
  public:
    /**
     * Defines the symbology of vector rendering
     * \since QGIS 3.12
     */
    enum class Symbology
    {
      Arrows = 0,  //!< Displaying vector dataset with arrows
      Streamlines, //!< Displaying vector dataset with streamlines
      Traces,      //!< Displaying vector dataset with particle traces
      WindBarbs    //!< Displaying vector dataset with wind barbs
    };

    //! Returns line width of the arrow (in millimeters)
    double lineWidth() const;
    //! Sets line width of the arrow in pixels (in millimeters)
    void setLineWidth( double lineWidth );

    //! Returns color used for drawing arrows
    QColor color() const;
    //! Sets color used for drawing arrows
    void setColor( const QColor &color );

    /**
     * Returns filter value for vector magnitudes.
     *
     * If magnitude of the vector is lower than this value, the vector is not
     * drawn. -1 represents that filtering is not active.
     */
    double filterMin() const;

    /**
     * Sets filter value for vector magnitudes.
     * \see filterMin()
     */
    void setFilterMin( double filterMin );

    /**
     * Returns filter value for vector magnitudes.
     *
     * If magnitude of the vector is higher than this value, the vector is not
     * drawn. -1 represents that filtering is not active.
     */
    double filterMax() const;

    /**
     * Sets filter value for vector magnitudes.
     * \see filterMax()
     */
    void setFilterMax( double filterMax );

    //! Returns whether vectors are drawn on user-defined grid
    bool isOnUserDefinedGrid() const;
    //! Toggles drawing of vectors on user defined grid
    void setOnUserDefinedGrid( bool enabled );
    //! Returns width in pixels of user grid cell
    int userGridCellWidth() const;
    //! Sets width of user grid cell (in pixels)
    void setUserGridCellWidth( int width );
    //! Returns height in pixels of user grid cell
    int userGridCellHeight() const;
    //! Sets height of user grid cell (in pixels)
    void setUserGridCellHeight( int height );

    /**
    * Returns the displaying method used to render vector datasets
    */
    Symbology symbology() const;

    /**
     * Sets the displaying method used to render vector datasets
     */
    void setSymbology( const Symbology &symbology );

    /**
     * Returns the coloring method used to render vector datasets
     */
    QgsInterpolatedLineColor::ColoringMethod coloringMethod() const;

    /**
     * Sets the coloring method used to render vector datasets
     */
    void setColoringMethod( const QgsInterpolatedLineColor::ColoringMethod &coloringMethod );

    /**
     * Sets the color ramp shader used to render vector datasets
     */
    QgsColorRampShader colorRampShader() const;

    /**
     * Returns the color ramp shader used to render vector datasets
     */
    void setColorRampShader( const QgsColorRampShader &colorRampShader );

    /**
     * Returns the stroke coloring used to render vector datasets
     */
    QgsInterpolatedLineColor vectorStrokeColoring() const;

    /**
    * Returns settings for vector rendered with arrows
    */
    QgsVectorFieldArrowSettings arrowSettings() const;

    /**
     * Sets settings for vector rendered with arrows
     */
    void setArrowsSettings( const QgsVectorFieldArrowSettings &arrowSettings );

    /**
     * Returns settings for vector rendered with streamlines
     */
    QgsVectorFieldStreamlineSettings streamLinesSettings() const;

    /**
     * Sets settings for vector rendered with streamlines
     */
    void setStreamLinesSettings( const QgsVectorFieldStreamlineSettings &streamLinesSettings );

    /**
     * Returns settings for vector rendered with traces
     */
    QgsVectorFieldTracesSettings tracesSettings() const;

    /**
     * Sets settings for vector rendered with traces
     */
    void setTracesSettings( const QgsVectorFieldTracesSettings &tracesSettings );

    /**
    * Returns settings for vector rendered with wind barbs
    */
    QgsVectorFieldWindBarbSettings windBarbSettings() const;

    /**
     * Sets settings for vector rendered with wind barbs
     */
    void setWindBarbSettings( const QgsVectorFieldWindBarbSettings &windBarbSettings );

    //! Writes configuration to a new DOM element
    QDomElement writeXml( QDomDocument &doc, const QgsReadWriteContext &context = QgsReadWriteContext() ) const;
    //! Reads configuration from the given DOM element
    void readXml( const QDomElement &elem, const QgsReadWriteContext &context = QgsReadWriteContext() );

  private:
    Symbology mDisplayingMethod = Symbology::Arrows;

    double mLineWidth = Qgis::DEFAULT_LINE_WIDTH; //in millimeters
    QgsColorRampShader mColorRampShader;
    QColor mColor = Qt::black;
    QgsInterpolatedLineColor::ColoringMethod mColoringMethod = QgsInterpolatedLineColor::SingleColor;
    double mFilterMin = -1;       //disabled
    double mFilterMax = -1;       //disabled
    int mUserGridCellWidth = 10;  // in pixels
    int mUserGridCellHeight = 10; // in pixels
    bool mOnUserDefinedGrid = false;

    QgsVectorFieldArrowSettings mArrowsSettings;
    QgsVectorFieldStreamlineSettings mStreamLinesSettings;
    QgsVectorFieldTracesSettings mTracesSettings;
    QgsVectorFieldWindBarbSettings mWindBarbSettings;
};

#endif // QGSVECTORFIELDSETTINGS_H
