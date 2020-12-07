/***************************************************************************
                         qgsbrightnesscontrastfilter.h
                         -------------------
    begin                : February 2013
    copyright            : (C) 2013 by Alexander Bruy
    email                : alexander dot bruy at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSBRIGHTNESSCONTRASTFILTER_H
#define QGSBRIGHTNESSCONTRASTFILTER_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsrasterinterface.h"

class QDomElement;

/**
 * \ingroup core
  * Brightness/contrast and gamma correction filter pipe for rasters.
  */
class CORE_EXPORT QgsBrightnessContrastFilter : public QgsRasterInterface
{
  public:
    QgsBrightnessContrastFilter( QgsRasterInterface *input = nullptr );

    //! Clone itself, create deep copy
    QgsBrightnessContrastFilter *clone() const override SIP_FACTORY;

    //! Gets number of bands
    int bandCount() const override;

    //! Returns data type for the band specified by number
    Qgis::DataType dataType( int bandNo ) const override;

    /**
     * Set input.
     * Returns TRUE if set correctly, FALSE if cannot use that input
     */
    bool setInput( QgsRasterInterface *input ) override;

    /**
     * Read block of data using given extent and size.
     *  Returns pointer to data.
     *  Caller is responsible to free the memory returned.
     * \param bandNo band number
     * \param extent extent of block
     * \param width pixel width of block
     * \param height pixel height of block
     * \param feedback optional raster feedback object for cancellation/preview. Added in QGIS 3.0.
     */
    QgsRasterBlock *block( int bandNo, const QgsRectangle &extent, int width, int height, QgsRasterBlockFeedback *feedback = nullptr ) override SIP_FACTORY;

    /**
     * Set brightness level. Acceptable value range is -255…255
     * \see brightness()
     */
    void setBrightness( int brightness ) { mBrightness = qBound( -255, brightness, 255 ); }

    /**
     * Returns current brightness level.
     * \see setBrightness()
     */
    int brightness() const { return mBrightness; }

    /**
     * Set contrast level. Acceptable value range is -100…100
     * \see contrast()
     */
    void setContrast( int contrast ) { mContrast = qBound( -100, contrast, 100 ); }

    /**
     * Returns current contrast level.
     * \see setContrast()
     */
    int contrast() const { return mContrast; }

    /**
     * Set gamma value. Acceptable value range is -0.1…10
     * \see gamma()
     *
     * \since QGIS 3.16
     */
    void setGamma( double gamma ) { mGamma = qBound( 0.1, gamma, 10.0 ); }

    /**
     * Returns current gamma value.
     * \see setGamma()
     *
     * \since QGIS 3.16
     */
    double gamma() const { return mGamma; }

    //! Write base class members to xml.
    void writeXml( QDomDocument &doc, QDomElement &parentElem ) const override;

    //! Sets base class members from xml. Usually called from create() methods of subclasses
    void readXml( const QDomElement &filterElem ) override;

  private:
    //! Adjusts a color component by the specified brightness, contrast factor and gamma correction
    int  adjustColorComponent( int colorComponent, int alpha, int brightness, double contrastFactor, double gammaCorrection ) const;

    //! Current brightness coefficient value. Default: 0. Range: -255...255
    int mBrightness = 0;

    //! Current contrast coefficient value. Default: 0. Range: -100...100
    int mContrast = 0;

    //! Current gamma value. Default: 1. Range: 0.1…10.0
    double mGamma = 1.0;

};

#endif // QGSBRIGHTNESSCONTRASTFILTER_H
