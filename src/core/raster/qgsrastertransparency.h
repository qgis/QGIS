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
    //! \brief Accessor for transparentSingleValuePixelList
    QList<QgsRasterTransparency::TransparentSingleValuePixel> transparentSingleValuePixelList() const;

    //! \brief Accessor for transparentThreeValuePixelList
    QList<QgsRasterTransparency::TransparentThreeValuePixel> transparentThreeValuePixelList() const;

    //! \brief Reset to the transparency list to a single value
    void initializeTransparentPixelList( double );

    //! \brief Reset to the transparency list to a single value
    void initializeTransparentPixelList( double, double, double );

    //! \brief Mutator for transparentSingleValuePixelList
    void setTransparentSingleValuePixelList( const QList<QgsRasterTransparency::TransparentSingleValuePixel> &newList SIP_TRANSFER );

    //! \brief Mutator for transparentThreeValuePixelList
    void setTransparentThreeValuePixelList( const QList<QgsRasterTransparency::TransparentThreeValuePixel> &newList SIP_TRANSFER );

    //! \brief Returns the transparency value for a single value Pixel
    int alphaValue( double, int globalTransparency = 255 ) const;

    //! \brief Return the transparency value for a RGB Pixel
    int alphaValue( double, double, double, int globalTransparency = 255 ) const;

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
