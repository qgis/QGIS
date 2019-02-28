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
#include <QList>
class QDomDocument;
class QDomElement;

/**
 * \ingroup core
 * Defines the list of pixel values to be considered as transparent or semi
 * transparent when rendering rasters.
 */
class CORE_EXPORT QgsRasterTransparency
{

  public:

    /**
     * Constructor for QgsRasterTransparency.
     */
    QgsRasterTransparency() = default;

    //
    // Structs to hold transparent pixel vlaues
    //
    struct TransparentThreeValuePixel
    {
      double red;
      double green;
      double blue;
      double percentTransparent;
    };

    struct TransparentSingleValuePixel
    {
      double min;
      double max;
      double percentTransparent;
    };

    //
    // Initializer, Accessor and mutator for transparency tables.
    //

    /**
     * Returns the transparent single value pixel list.
     * \see setTransparentSingleValuePixelList()
     */
    QList<QgsRasterTransparency::TransparentSingleValuePixel> transparentSingleValuePixelList() const;

    /**
     * Returns the transparent three value pixel list.
     * \see setTransparentThreeValuePixelList()
     */
    QList<QgsRasterTransparency::TransparentThreeValuePixel> transparentThreeValuePixelList() const;

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
    void setTransparentSingleValuePixelList( const QList<QgsRasterTransparency::TransparentSingleValuePixel> &newList );

    /**
     * Sets the transparent three value pixel list, replacing the whole existing list.
     * \see transparentThreeValuePixelList()
     */
    void setTransparentThreeValuePixelList( const QList<QgsRasterTransparency::TransparentThreeValuePixel> &newList );

    /**
     * Returns the transparency value for a single \a value pixel.
     *
     * Searches through the transparency list, and if a match is found, the global transparency value is scaled
     * by the stored transparency value.
     *
     * \param value the needle to search for in the transparency hay stack
     * \param globalTransparency the overall transparency level for the layer
    */
    int alphaValue( double value, int globalTransparency = 255 ) const;

    //! \brief

    /**
     * Returns the transparency value for a RGB pixel.
     *
     * Searches through the transparency list, if a match is found, the global transparency value is scaled
     * by the stored transparency value.
     * \param redValue the red portion of the needle to search for in the transparency hay stack
     * \param greenValue  the green portion of the needle to search for in the transparency hay stack
     * \param blueValue the green portion of the needle to search for in the transparency hay stack
     * \param globalTransparency the overall transparency level for the layer
    */
    int alphaValue( double redValue, double greenValue, double blueValue, int globalTransparency = 255 ) const;

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
    QList<QgsRasterTransparency::TransparentThreeValuePixel> mTransparentThreeValuePixelList;

    //! \brief The list to hold transparency values for single value pixel layers
    QList<QgsRasterTransparency::TransparentSingleValuePixel> mTransparentSingleValuePixelList;

};
#endif
