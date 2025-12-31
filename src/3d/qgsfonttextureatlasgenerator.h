/***************************************************************************
  qgsfonttextureatlasgenerator.h
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

#ifndef QGSFONTTEXTUREATLASGENERATOR_H
#define QGSFONTTEXTUREATLASGENERATOR_H

#include <vector>

#include "qgis_3d.h"
#include "qgstextformat.h"

#include <QImage>
#include <QRect>

///@cond PRIVATE
class QgsCharTextureRect;
///@endcond


/**
 * \ingroup qgis_3d
 * \brief Encapsulates a font texture atlas.
 *
 * QgsFontTextureAtlas contains the packed texture atlas for a font, along with the associated
 * text metrics for rendering.
 *
 * See QgsFontTextureAtlasGenerator for a class which automatically creates texture atlases.
 *
 * \since QGIS 4.0
 */
class _3D_EXPORT QgsFontTextureAtlas
{
  public:
    QgsFontTextureAtlas();
    ~QgsFontTextureAtlas();

    QgsFontTextureAtlas( const QgsFontTextureAtlas &other );
    QgsFontTextureAtlas &operator=( const QgsFontTextureAtlas &other );

    /**
     * Returns TRUE if the atlas is valid.
     */
    bool isValid() const { return mAtlasSize.isValid(); }

    /**
     * Returns the total size required for the atlas, i.e. the total
     * size for the texture.
     */
    QSize atlasSize() const { return mAtlasSize; }

#ifndef SIP_RUN
    /**
     * Returns the packed rectangle for the texture for the specified \a grapheme.
     */
    QRect rect( const QString &grapheme ) const;
#else

    /**
     * Returns the packed rectangle for the texture for the specified \a grapheme.
     *
     * \throws KeyError if no texture for the specified grapheme exists.
     */
    QRect rect( const QString &grapheme ) const;
    //%MethodCode
    const QRect res = sipCpp->rect( *a0 );
    if ( res.isNull() )
    {
      PyErr_SetString( PyExc_KeyError, u"No rectangle for character %1 exists."_s.arg( *a0 ).toUtf8().constData() );
      sipIsErr = 1;
    }
    else
    {
      return sipConvertFromNewType( new QRect( res ), sipType_QRect, Py_None );
    }
    //%End
#endif

    /**
     * Returns the number of graphemes to render for a given \a string.
     *
     * The \a string must match one of the strings passed to QgsFontTextureAtlasGenerator when
     * creating the texture atlas.
     */
    int graphemeCount( const QString &string ) const;

    /**
     * Returns the total width (in pixels) required for a given \a string.
     *
     * The \a string must match one of the strings passed to QgsFontTextureAtlasGenerator when
     * creating the texture atlas.
     */
    int totalWidth( const QString &string ) const;

    /**
     * Returns the pixel offset at which the texture for the matching grapheme should be placed.
     *
     * The \a string must match one of the strings passed to QgsFontTextureAtlasGenerator when
     * creating the texture atlas.
     */
    QPoint pixelOffsetForGrapheme( const QString &string, int graphemeIndex ) const;

    /**
     * Returns the packed rectangle for the texture for the matching grapheme.
     *
     * The \a string must match one of the strings passed to QgsFontTextureAtlasGenerator when
     * creating the texture atlas.
     */
    QRect textureRectForGrapheme( const QString &string, int graphemeIndex ) const;

    /**
     * Renders the combined texture atlas, containing all required characters.
     */
    QImage renderAtlasTexture() const;

    /**
     * Renders a debug texture.
     *
     * The debug texture renders all packed character rectangles with a unique color, and can be used
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
    struct GraphemeMetric
    {
        GraphemeMetric( int horizontalAdvance = 0, const QString &grapheme = QString() )
          : horizontalAdvance( horizontalAdvance )
          , grapheme( grapheme )
        {}

        int horizontalAdvance = 0;
        QString grapheme;
    };

    struct StringMetrics
    {
        int totalWidth = 0;
        QVector< GraphemeMetric > graphemeMetrics;
    };

    QgsTextFormat mFormat;
    std::vector< QgsCharTextureRect > mRects;
    QSize mAtlasSize;
    QHash< QString, int > mGraphemeIndices;
    QMap< QString, StringMetrics > mStringMetrics;
    int mTexturePaddingPixels = 0;

    friend class QgsFontTextureAtlasGenerator;
};

/**
 * \ingroup qgis_3d
 * \brief Generates texture atlases for a font by efficiently packing the characters required for a set of strings.
 *
 * \since QGIS 4.0
 */
class _3D_EXPORT QgsFontTextureAtlasGenerator
{
  public:
    /**
     * Creates the texture atlas for a set of \a strings, using the specified text \a format.
     */
    static QgsFontTextureAtlas create( const QgsTextFormat &format, const QStringList &strings );
};

#endif // QGSFONTTEXTUREATLASGENERATOR_H
