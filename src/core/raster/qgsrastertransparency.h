/* **************************************************************************
                qgsrastertransparency.h -  description
                       -------------------
begin                : Mon Nov 30 2007
copyright            : (C) 2007 by Peter J. Ersts
email                : ersts@amnh.org

****************************************************************************/

/* **************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSRASTERTRANSPARENCY_H
#define QGSRASTERTRANSPARENCY_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgis.h"
#include <QList>
class QDomDocument;
class QDomElement;

/**
 * \ingroup core
 * \brief Defines the list of pixel values to be considered as transparent or semi
 * transparent when rendering rasters.
 */
class CORE_EXPORT QgsRasterTransparency
{

  public:

    /**
     * Constructor for QgsRasterTransparency.
     */
    QgsRasterTransparency() = default;

    /**
     * \ingroup core
     * \brief Defines the transparency for a RGB pixel value.
     */
    struct TransparentThreeValuePixel
    {

      /**
      * Constructor for TransparentThreeValuePixel.
      * \param red red pixel value
      * \param green green pixel value
      * \param blue blue pixel value
      * \param opacity opacity for pixel, between 0 and 1.0
      * \since QGIS 3.38
      */
      TransparentThreeValuePixel( double red = 0, double green = 0, double blue = 0, double opacity = 0 )
        : red( red )
        , green( green )
        , blue( blue )
        , opacity( opacity )
      {}

      /**
       * Red pixel value.
       */
      double red;

      /**
      * Green pixel value.
      */
      double green;

      /**
       * Blue pixel value.
       */
      double blue;

      /**
      * Opacity for pixel, between 0 and 1.0.
      *
      * \since QGIS 3.38
      */
      double opacity = 0;

      bool operator==( const QgsRasterTransparency::TransparentThreeValuePixel &other ) const
      {
        return qgsDoubleNear( red, other.red )
               && qgsDoubleNear( green, other.green )
               && qgsDoubleNear( blue, other.blue )
               && qgsDoubleNear( opacity, other.opacity );
      }
      bool operator!=( const QgsRasterTransparency::TransparentThreeValuePixel &other ) const
      {
        return !( *this == other );
      }

#ifdef SIP_RUN
      SIP_PYOBJECT __repr__();
      % MethodCode
      const QString str = QStringLiteral( "<QgsRasterTransparency.TransparentThreeValuePixel: %1, %2, %3, %4>" ).arg( sipCpp->red ).arg( sipCpp->green ).arg( sipCpp->blue ).arg( sipCpp->opacity );
      sipRes = PyUnicode_FromString( str.toUtf8().constData() );
      % End
#endif
    };

    /**
     * \ingroup core
     * \brief Defines the transparency for a range of single-band pixel values.
     */
    struct TransparentSingleValuePixel
    {

      /**
       * Constructor for TransparentSingleValuePixel.
       * \param minimum minimum pixel value to include in range
       * \param maximum maximum pixel value to include in range
       * \param opacity opacity for pixel, between 0 and 1.0
       * \param includeMinimum whether the minimum value should be included in the range
       * \param includeMaximum whether the maximum value should be included in the range
       *
       * \since QGIS 3.38
       */
      TransparentSingleValuePixel( double minimum = 0, double maximum = 0, double opacity = 0, bool includeMinimum = true, bool includeMaximum = true )
        : min( minimum )
        , max( maximum )
        , opacity( opacity )
        , includeMinimum( includeMinimum )
        , includeMaximum( includeMaximum )
      {}

      /**
       * Minimum pixel value to include in range.
       */
      double min;

      /**
       * Maximum pixel value to include in range.
       */
      double max;

      /**
       * Opacity for pixel, between 0 and 1.0.
       *
       * \since QGIS 3.38
       */
      double opacity = 0;

      /**
       * TRUE if pixels matching the min value should be considered transparent,
       * or FALSE if only pixels greater than the min value should be transparent.
       *
       * \since QGIS 3.38
       */
      bool includeMinimum = true;

      /**
       * TRUE if pixels matching the max value should be considered transparent,
       * or FALSE if only pixels less than the max value should be transparent.
       *
       * \since QGIS 3.38
       */
      bool includeMaximum = true;

      bool operator==( const QgsRasterTransparency::TransparentSingleValuePixel &other ) const
      {
        return qgsDoubleNear( min, other.min )
               && qgsDoubleNear( max, other.max )
               && qgsDoubleNear( opacity, other.opacity )
               && includeMinimum == other.includeMinimum && includeMaximum == other.includeMaximum;
      }
      bool operator!=( const QgsRasterTransparency::TransparentSingleValuePixel &other ) const
      {
        return !( *this == other );
      }

#ifdef SIP_RUN
      SIP_PYOBJECT __repr__();
      % MethodCode
      const QString str = QStringLiteral( "<QgsRasterTransparency.TransparentSingleValuePixel: %1, %2, %3>" ).arg( sipCpp->min ).arg( sipCpp->max ).arg( sipCpp->opacity );
      sipRes = PyUnicode_FromString( str.toUtf8().constData() );
      % End
#endif
    };

    /**
     * Returns the transparent single value pixel list.
     * \see setTransparentSingleValuePixelList()
     */
    QVector<QgsRasterTransparency::TransparentSingleValuePixel> transparentSingleValuePixelList() const;

    /**
     * Returns the transparent three value pixel list.
     * \see setTransparentThreeValuePixelList()
     */
    QVector<QgsRasterTransparency::TransparentThreeValuePixel> transparentThreeValuePixelList() const;

    /**
     * Resets the transparency list to a single \a value.
     */
    void initializeTransparentPixelList( double value );

    /**
     * Resets the transparency list to single red, green, and blue values.
     */
    void initializeTransparentPixelList( double redValue, double greenValue, double blueValue );

    /**
     * Sets the transparent single value pixel list, replacing the whole existing list.
     * \see transparentSingleValuePixelList()
     */
    void setTransparentSingleValuePixelList( const QVector<QgsRasterTransparency::TransparentSingleValuePixel> &newList );

    /**
     * Sets the transparent three value pixel list, replacing the whole existing list.
     * \see transparentThreeValuePixelList()
     */
    void setTransparentThreeValuePixelList( const QVector<QgsRasterTransparency::TransparentThreeValuePixel> &newList );

    /**
     * Returns the transparency value for a single \a value pixel.
     *
     * Searches through the transparency list, and if a match is found, the global transparency value is scaled
     * by the stored transparency value.
     *
     * \param value the needle to search for in the transparency hay stack
     * \param globalTransparency the overall transparency level for the layer
     *
     * \deprecated use opacityForValue() instead.
    */
    Q_DECL_DEPRECATED int alphaValue( double value, int globalTransparency = 255 ) const SIP_DEPRECATED;

    /**
     * Returns the opacity (as a value from 0 to 1) for a single \a value pixel.
     *
     * Searches through the transparency list, and if a match is found, returns
     * the opacity corresponding to the value. Returns 1 if no matches are found.
     *
     * \since QGIS 3.38
    */
    double opacityForValue( double value ) const;

    /**
     * Returns the transparency value for a RGB pixel.
     *
     * Searches through the transparency list, if a match is found, the global transparency value is scaled
     * by the stored transparency value.
     * \param redValue the red portion of the needle to search for in the transparency hay stack
     * \param greenValue  the green portion of the needle to search for in the transparency hay stack
     * \param blueValue the green portion of the needle to search for in the transparency hay stack
     * \param globalTransparency the overall transparency level for the layer
     *
     * \deprecated use opacityForRgbValues() instead.
    */
    Q_DECL_DEPRECATED int alphaValue( double redValue, double greenValue, double blueValue, int globalTransparency = 255 ) const SIP_DEPRECATED;

    /**
     * Returns the opacity (as a value from 0 to 1) for a set of RGB pixel values.
     *
     * Searches through the transparency list, and if a match is found, returns
     * the opacity corresponding to the values. Returns 1 if no matches are found.
     *
     * If any of the red, green or blue values are NaN, 0 will be returned.
     *
     * \since QGIS 3.38
    */
    double opacityForRgbValues( double redValue, double greenValue, double blueValue ) const;

    //! True if there are no entries in the pixel lists except the nodata value
    bool isEmpty() const;

    /**
     * Writes the transparency information to an XML document.
     */
    void writeXml( QDomDocument &doc, QDomElement &parentElem ) const;

    /**
     * Reads the transparency information from an XML document.
     */
    void readXml( const QDomElement &elem );

  private:
    //! \brief The list to hold transparency values for RGB layers
    QVector<QgsRasterTransparency::TransparentThreeValuePixel> mTransparentThreeValuePixelList;

    //! \brief The list to hold transparency values for single value pixel layers
    QVector<QgsRasterTransparency::TransparentSingleValuePixel> mTransparentSingleValuePixelList;

};
#endif
