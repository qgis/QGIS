/***************************************************************************
  qgsrastercontourrenderer.h
  --------------------------------------
  Date                 : March 2020
  Copyright            : (C) 2020 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSRASTERCONTOURRENDERER_H
#define QGSRASTERCONTOURRENDERER_H


#include "qgsrasterrenderer.h"

class QgsLineSymbol;

/**
 * \ingroup core
 * Raster renderer that generates contours on the fly for a source raster band.
 *
 * \since QGIS 3.14
 */
class CORE_EXPORT QgsRasterContourRenderer : public QgsRasterRenderer
{
  public:
    //! Creates a contour renderer
    explicit QgsRasterContourRenderer( QgsRasterInterface *input );
    ~QgsRasterContourRenderer() override;

    //! QgsRasterContourRenderer cannot be copied. Use clone() instead.
    QgsRasterContourRenderer( const QgsRasterContourRenderer & ) = delete;
    //! QgsRasterContourRenderer cannot be copied. Use clone() instead.
    const QgsRasterContourRenderer &operator=( const QgsRasterContourRenderer & ) = delete;

    QgsRasterContourRenderer *clone() const override SIP_FACTORY;

    static QgsRasterRenderer *create( const QDomElement &elem, QgsRasterInterface *input ) SIP_FACTORY;

    void writeXml( QDomDocument &doc, QDomElement &parentElem ) const override;

    QgsRasterBlock *block( int bandNo, const QgsRectangle &extent, int width, int height, QgsRasterBlockFeedback *feedback = nullptr ) override SIP_FACTORY;

    QList<int> usesBands() const override;

    //

    //! Returns the number of the input raster band
    int inputBand() const { return mInputBand; }
    //! Sets the number of the input raster band
    void setInputBand( int band ) { mInputBand = band; }

    //! Returns the interval of contour lines generation
    double contourInterval() const { return mContourInterval; }
    //! Sets the interval of contour lines generation
    void setContourInterval( double interval ) { mContourInterval = interval; }

    //! Returns the symbol used for contour lines
    QgsLineSymbol *contourSymbol() const { return mContourSymbol.get(); }
    //! Sets the symbol used for contour lines. Takes ownership of the passed symbol
    void setContourSymbol( QgsLineSymbol *symbol SIP_TRANSFER );

    //! Returns the interval of index contour lines (index contour lines are typical further apart and with a wider line symbol)
    double contourIndexInterval() const { return mContourIndexInterval; }
    //! Sets the interval of index contour lines (index contour lines are typical further apart and with a wider line symbol)
    void setContourIndexInterval( double interval ) { mContourIndexInterval = interval; }

    //! Returns the symbol of index contour lines
    QgsLineSymbol *contourIndexSymbol() const { return mContourIndexSymbol.get(); }
    //! Sets the symbol of index contour lines
    void setContourIndexSymbol( QgsLineSymbol *symbol SIP_TRANSFER );

    /**
     * Returns by how much the renderer will scale down the request to the data provider.
     * For example, for a raster block 1000x500 with downscale 10, the renderer will request raster 100x50 from provider.
     * Higher downscale makes contour lines more simplified (at the expense of loosing some detail).
     * The value of one means there will be no downscaling.
     */
    double downscale() const { return mDownscale; }

    /**
     * Sets by how much the renderer will scale down the request to the data provider.
     * \see downscale()
     */
    void setDownscale( double scale ) { mDownscale = scale; }

  private:

#ifdef SIP_RUN
    QgsRasterContourRenderer( const QgsRasterContourRenderer & );
    const QgsRasterContourRenderer &operator=( const QgsRasterContourRenderer & );
#endif

    std::unique_ptr<QgsLineSymbol> mContourSymbol;   // should not be null
    std::unique_ptr<QgsLineSymbol> mContourIndexSymbol;  // may be null
    double mDownscale = 8.;
    double mContourInterval = 100.;
    double mContourIndexInterval = 0.;
    int mInputBand = 1;
};


#endif // QGSRASTERCONTOURRENDERER_H
