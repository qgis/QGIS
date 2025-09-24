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

#include "qgis_3d.h"
#include <vector>
#include <QRect>
#include <QImage>

///@cond PRIVATE
class QgsTextureRect;
///@endcond

/**
 * \ingroup qgis_3d
 * \brief Generates texture atlases by efficient packing of multiple input rectangles/images.
 *
 * QgsTextureAtlasGenerator can be used to pack either images or raw rectangles. The associated
 * method (appendRect() or appendImage()) should be called multiple times, adding all the
 * required objects to pack. The generateAtlas() method should then be called to perform
 * the packing, before the solution methods like rect() or atlasTexture() can be
 * called.
 *
 * \since QGIS 4.0
 */
class _3D_EXPORT QgsTextureAtlasGenerator
{
  public:
    QgsTextureAtlasGenerator();
    ~QgsTextureAtlasGenerator();

#ifndef SIP_RUN
    QgsTextureAtlasGenerator( const QgsTextureAtlasGenerator &other ) = delete;
    QgsTextureAtlasGenerator &operator=( const QgsTextureAtlasGenerator &other ) = delete;
#endif

    /**
     * Appends a rectangle to the atlas.
     *
     * This method should be used when the generator is used to pack rectangle shapes only.
     * No image will be associated with the rectangle, and the associated rectangle
     * will be empty in the atlasTexture().
     *
     * \returns a unique ID which can be used to retrieve the calculated packed position
     * of the rectangle after generating the atlas.
     *
     * \see appendImage()
     */
    int appendRect( const QRect &rect );

    /**
     * Appends an \a image to the atlas.
     *
     * \returns a unique ID which can be used to retrieve the calculated packed position
     * of the image after generating the atlas.
     *
     * \see appendRect()
     */
    int appendImage( const QImage &image );

    /**
     * Generates the packing solution for all stored rectangles and images.
     *
     * The \a maxSide argument specifies the maximum permitted side size for the atlas.
     * The calculated solution can only be less than or equal to this size - if it cannot fit,
     * then algorithm will gracefully fail and return FALSE.
     *
     * \note This method must be called before retrieving rect(), atlasSize() or atlasTexture().
     */
    bool generateAtlas( int maxSide = 1000 );

    /**
     * Returns the total size required for the atlas, i.e. the calculated
     * size for the packed images and rectangles.
     *
     * \warning generateAtlas() must be called before this method can be used.
     */
    QSize atlasSize() const { return mAtlasSize; }

    /**
     * Returns the calculated packed rectangle for the rectangle or image with the specified \a id.
     *
     * \warning generateAtlas() must be called before this method can be used.
     */
    QRect rect( int id ) const;

    /**
     * Renders the combined texture atlas, containing all images added via appendImage().
     *
     * \warning generateAtlas() must be called before this method can be used.
     */
    QImage atlasTexture() const;

    /**
     * Renders a debug texture.
     *
     * The debug texture renders all packed rectangles with a unique color, and can be used
     * to visualize the solution.
     *
     * \warning generateAtlas() must be called before this method can be used.
     */
    QImage debugTexture() const;

  private:
#ifdef SIP_RUN
    QgsTextureAtlasGenerator( const QgsTextureAtlasGenerator &other );
#endif

    std::vector< QgsTextureRect > mRects;
    QHash< int, int > mIdToIndex;
    QSize mAtlasSize;
};

#endif // QGSTEXTUREATLASGENERATOR_H
