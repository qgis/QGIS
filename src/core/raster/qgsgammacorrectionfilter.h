/***************************************************************************
                         qgsgammacorrectionfilter.h
                         -------------------
    begin                : June 2020
    copyright            : (C) 2020 by Alexander Bruy
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

#ifndef QGSGAMMACORRECTIONFILTER_H
#define QGSGAMMACORRECTIONFILTER_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsrasterinterface.h"

class QDomElement;

/**
 * \ingroup core
  * Gamma correction filter pipe for rasters.
  * \since QGIS 3.16
  */
class CORE_EXPORT QgsGammaCorrectionFilter : public QgsRasterInterface
{
  public:
    QgsGammaCorrectionFilter( QgsRasterInterface *input = nullptr );

    QgsGammaCorrectionFilter *clone() const override SIP_FACTORY;

    int bandCount() const override;

    Qgis::DataType dataType( int bandNo ) const override;

    bool setInput( QgsRasterInterface *input ) override;

    QgsRasterBlock *block( int bandNo, const QgsRectangle &extent, int width, int height, QgsRasterBlockFeedback *feedback = nullptr ) override SIP_FACTORY;

    void setGamma( double gamma ) { mGamma = qBound( 0.1, gamma, 10.0 ); }
    double gamma() const { return mGamma; }

    void writeXml( QDomDocument &doc, QDomElement &parentElem ) const override;

    //! Sets base class members from xml. Usually called from create() methods of subclasses
    void readXml( const QDomElement &filterElem ) override;

  private:
    //! Adjusts a color component by the specified gamma correction value
    int  adjustColorComponent( int colorComponent, int alpha, double gammaCorrection ) const;

    //! Current gamma value. Default: 1. Range: 0.1…10.0
    double mGamma = 1.0;
};

#endif // QGSGAMMACORRECTIONFILTER_H
