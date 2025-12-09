/***************************************************************************
  qgstextureatlasgenerator.h
  --------------------------------------
  Date                 : September 2025
  Copyright            : (C) 2025 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSTEXTUREATLASGENERATOR_H
#define QGSTEXTUREATLASGENERATOR_H

#include <vector>

#include "qgis_3d.h"

#include <QImage>
#include <QRect>

///@cond PRIVATE
class QgsTextureRect;
///@endcond


/**
 * \ingroup qgis_3d
 * \brief Encapsulates a texture atlas.
 *
 * QgsTextureAtlas contains the packed regions for aggregated texture atlases, and optionally the packed
 * texture map.
 *
 * See QgsTextureAtlasGenerator for a class which automatically creates texture atlases.
 *
 * \since QGIS 4.0
 */
class _3D_EXPORT QgsTextureAtlas
{
  public:
    QgsTextureAtlas();
    ~QgsTextureAtlas();

    QgsTextureAtlas( const QgsTextureAtlas &other );
    QgsTextureAtlas &operator=( const QgsTextureAtlas &other );

    /**
     * Returns TRUE if the atlas is valid.
     */
    bool isValid() const { return mAtlasSize.isValid(); }

    /**
     * Returns the total size required for the atlas, i.e. the total
     * size for the packed images and rectangles.
     */
    QSize atlasSize() const { return mAtlasSize; }

#ifndef SIP_RUN
    /**
     * Returns the packed rectangle for the texture with the specified \a index.
     */
    QRect rect( int index ) const;
#else

    /**
     * Returns the packed rectangle for the texture with the specified \a index.
     *
     * \throws IndexError if no texture with the specified index exists.
     */
    QRect rect( int index ) const;
    //%MethodCode
    const int count = sipCpp->count();
    if ( a0 < 0 || a0 >= count )
    {
      PyErr_SetString( PyExc_IndexError, QByteArray::number( a0 ) );
      sipIsErr = 1;
    }
    else
    {
      return sipConvertFromNewType( new QRect( sipCpp->rect( a0 ) ), sipType_QRect, Py_None );
    }
    //%End
#endif

    /**
     * Renders the combined texture atlas, containing all source images.
     *
     * \note This may be a null image if the atlas was created with rectangles alone.
     */
    QImage renderAtlasTexture() const;

    /**
     * Renders a debug texture.
     *
     * The debug texture renders all packed rectangles with a unique color, and can be used
     * to visualize the solution.
     */
    QImage renderDebugTexture() const;

    /**
     * Returns the number of textures in the atlas.
     */
    int count() const;

#ifdef SIP_RUN
    int __len__() const;
    % Docstring
        Returns the number of textures in the atlas.
      % End
        //%MethodCode
        sipRes
      = sipCpp->count();
    //% End
#endif

  private:
    std::vector< QgsTextureRect >
      mRects;
    QSize mAtlasSize;

    friend class QgsTextureAtlasGenerator;
};

/**
 * \ingroup qgis_3d
 * \brief Generates texture atlases by efficient packing of multiple input rectangles/images.
 *
 * QgsTextureAtlasGenerator can be used to pack either images or raw rectangles. The
 * static createFromRects() or createFromImages() methods should be called with the
 * source images or rectangles, which will return a QgsTextureAtlas containing the results.
 *
 * \since QGIS 4.0
 */
class _3D_EXPORT QgsTextureAtlasGenerator
{
  public:
    /**
     * Creates a texture atlas for a set of \a rectangles.
     *
     * This method should be used when the generator is used to pack rectangle shapes only.
     * No image will be associated with the rectangle.
     *
     * The \a maxSide argument specifies the maximum permitted side size for the atlas.
     * The calculated solution can only be less than or equal to this size - if it cannot fit,
     * then algorithm will gracefully fail and return an invalid QgsTextureAtlas.
     *
     * \see createFromImages()
     */
    static QgsTextureAtlas createFromRects( const QVector< QRect > &rectangles, int maxSide = 1000 );

    /**
     * Creates a texture atlas for a set of \a images.
     *
     * The \a maxSide argument specifies the maximum permitted side size for the atlas.
     * The calculated solution can only be less than or equal to this size - if it cannot fit,
     * then algorithm will gracefully fail and return an invalid QgsTextureAtlas.
     *
     * \see createFromRects()
     */
    static QgsTextureAtlas createFromImages( const QVector< QImage > &images, int maxSide = 1000 );

  private:
    /**
     * Generates the packing solution for a set of texture \a rects.
     *
     * The \a maxSide argument specifies the maximum permitted side size for the atlas.
     * The calculated solution can only be less than or equal to this size - if it cannot fit,
     * then algorithm will gracefully fail and return an invalid QgsTextureAtlas.
     */
    static QgsTextureAtlas generateAtlas( std::vector< QgsTextureRect > rects, int maxSide );
};

#endif // QGSTEXTUREATLASGENERATOR_H
