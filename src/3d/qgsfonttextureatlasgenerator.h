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

#include "qgis_3d.h"
#include "qgstextformat.h"
#include <vector>
#include <QRect>
#include <QImage>

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
     * Returns the packed rectangle for the texture for the specified \a character.
     */
    QRect rect( const QChar &character ) const;
#else
    /**
     * Returns the packed rectangle for the texture for the specified \a character.
     *
     * \throws KeyError if no texture for the specified character exists.
     */
    QRect rect( const QChar &character ) const;
    //%MethodCode
    const QRect res = sipCpp->rect( *a0 );
    if ( res.isNull() )
    {
      PyErr_SetString( PyExc_KeyError, QStringLiteral( "No rectangle for character %1 exists." ).arg( QString( *a0 ) ).toUtf8().constData() );
      sipIsErr = 1;
    }
    else
    {
      return sipConvertFromNewType( new QRect( res ), sipType_QRect, Py_None );
    }
    //%End
#endif

    /**
     * Returns the pixel offset at which the texture for the matching character should be placed.
     *
     * The \a string must match one of the strings passed to QgsFontTextureAtlasGenerator when
     * creating the texture atlas.
     */
    QPoint pixelOffsetForCharacter( const QString &string, int characterIndex ) const;

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
    QgsTextFormat mFormat;
    std::vector< QgsCharTextureRect > mRects;
    QSize mAtlasSize;
    QHash< QChar, int > mCharIndices;
    QMap< QString, QVector< int > > mHorizontalAdvancesForStrings;

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
