/***************************************************************************
                         qgspalettedrasterrenderer.h
                         ---------------------------
    begin                : December 2011
    copyright            : (C) 2011 by Marco Hugentobler
    email                : marco at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPALETTEDRASTERRENDERER_H
#define QGSPALETTEDRASTERRENDERER_H

#include "qgis_core.h"
#include <QVector>

#include "qgsrasterrenderer.h"

class QColor;
class QDomElement;

/** \ingroup core
  * Renderer for paletted raster images.
*/
class CORE_EXPORT QgsPalettedRasterRenderer: public QgsRasterRenderer
{
  public:

    //! Properties of a single value class
    struct Class
    {
      //! Constructor for Class
      Class( const QColor &color = QColor(), const QString &label = QString() )
        : color( color )
        , label( label )
      {}

      //! Color to render value
      QColor color;
      //! Label for value
      QString label;
    };

    //! Map of value to class properties
    typedef QMap< int, Class > ClassData;

    /**
     * Constructor for QgsPalettedRasterRenderer.
     */
    QgsPalettedRasterRenderer( QgsRasterInterface *input, int bandNumber, const ClassData &classes );
    ~QgsPalettedRasterRenderer();

    //! QgsPalettedRasterRenderer cannot be copied. Use clone() instead.
    QgsPalettedRasterRenderer( const QgsPalettedRasterRenderer & ) = delete;
    //! QgsPalettedRasterRenderer cannot be copied. Use clone() instead.
    const QgsPalettedRasterRenderer &operator=( const QgsPalettedRasterRenderer & ) = delete;

    QgsPalettedRasterRenderer *clone() const override;
    static QgsRasterRenderer *create( const QDomElement &elem, QgsRasterInterface *input );

    QgsRasterBlock *block( int bandNo, const QgsRectangle &extent, int width, int height, QgsRasterBlockFeedback *feedback = nullptr ) override;

    //! Returns number of colors
    int nColors() const { return mClassData.size(); }

    /**
     * Returns a map of value to classes (colors) used by the renderer.
     */
    ClassData classes() const;

    /** Return optional category label
     * \since QGIS 2.1 */
    QString label( int idx ) const;

    /** Set category label
     *  \since QGIS 2.1 */
    void setLabel( int idx, const QString &label );

    void writeXml( QDomDocument &doc, QDomElement &parentElem ) const override;

    void legendSymbologyItems( QList< QPair< QString, QColor > > &symbolItems ) const override;

    QList<int> usesBands() const override;

  private:

    int mBand;
    int mMaxColorIndex = -INT_MAX;
    ClassData mClassData;


    //! Premultiplied color array
    QRgb *mColors = nullptr;
    bool *mIsNoData = nullptr;
    void updateArrays();


};

#endif // QGSPALETTEDRASTERRENDERER_H
