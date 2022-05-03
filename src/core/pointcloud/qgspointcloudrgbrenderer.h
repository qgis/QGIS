/***************************************************************************
                         qgspointcloudrgbrenderer.h
                         --------------------
    begin                : October 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPOINTCLOUDRGBRENDERER_H
#define QGSPOINTCLOUDRGBRENDERER_H

#include "qgspointcloudrenderer.h"
#include "qgis_core.h"
#include "qgis_sip.h"

#include "qgscontrastenhancement.h"


#ifndef SIP_RUN

/**
 * \ingroup core
 * \brief Prepared data container for QgsPointCloudRgbRenderer.
 *
 * \note Not available in Python bindings.
 *
 * \since QGIS 3.26
 */
class CORE_EXPORT QgsPointCloudRgbRendererPreparedData: public QgsPreparedPointCloudRendererData
{
  public:

    QSet< QString > usedAttributes() const override;
    bool prepareBlock( const QgsPointCloudBlock *block ) override;
    QColor pointColor( const QgsPointCloudBlock *block, int i, double z ) override;

    QString redAttribute = QStringLiteral( "Red" );
    QString greenAttribute = QStringLiteral( "Green" );
    QString blueAttribute = QStringLiteral( "Blue" );
    std::unique_ptr< QgsContrastEnhancement > redContrastEnhancement;
    std::unique_ptr< QgsContrastEnhancement > greenContrastEnhancement;
    std::unique_ptr< QgsContrastEnhancement > blueContrastEnhancement;

    int redOffset = 0;
    QgsPointCloudAttribute::DataType redType;
    bool useRedContrastEnhancement = false;
    int greenOffset = 0;
    QgsPointCloudAttribute::DataType greenType;
    bool useGreenContrastEnhancement = false;
    int blueOffset = 0;
    QgsPointCloudAttribute::DataType blueType;
    bool useBlueContrastEnhancement = false;
};
#endif

/**
 * \ingroup core
 * \brief An RGB renderer for 2d visualisation of point clouds using embedded red, green and blue attributes.
 *
 * \since QGIS 3.18
 */
class CORE_EXPORT QgsPointCloudRgbRenderer : public QgsPointCloudRenderer
{
  public:

    /**
     * Constructor for QgsPointCloudRgbRenderer.
     */
    QgsPointCloudRgbRenderer();

    QString type() const override;
    QgsPointCloudRenderer *clone() const override;
    void renderBlock( const QgsPointCloudBlock *block, QgsPointCloudRenderContext &context ) override;
    QDomElement save( QDomDocument &doc, const QgsReadWriteContext &context ) const override;
    QSet< QString > usedAttributes( const QgsPointCloudRenderContext &context ) const override;
    std::unique_ptr< QgsPreparedPointCloudRendererData > prepare() override SIP_SKIP;

    /**
     * Creates an RGB renderer from an XML \a element.
     */
    static QgsPointCloudRenderer *create( QDomElement &element, const QgsReadWriteContext &context ) SIP_FACTORY;

    /**
     * Returns the attribute to use for the red channel.
     *
     * \see greenAttribute()
     * \see blueAttribute()
     * \see setRedAttribute()
     */
    QString redAttribute() const;

    /**
     * Sets the \a attribute to use for the red channel.
     *
     * \see setGreenAttribute()
     * \see setBlueAttribute()
     * \see redAttribute()
     */
    void setRedAttribute( const QString &attribute );

    /**
     * Returns the attribute to use for the green channel.
     *
     * \see redAttribute()
     * \see blueAttribute()
     * \see setGreenAttribute()
     */
    QString greenAttribute() const;

    /**
     * Sets the \a attribute to use for the green channel.
     *
     * \see setRedAttribute()
     * \see setBlueAttribute()
     * \see greenAttribute()
     */
    void setGreenAttribute( const QString &attribute );

    /**
     * Returns the attribute to use for the blue channel.
     *
     * \see greenAttribute()
     * \see redAttribute()
     * \see setBlueAttribute()
     */
    QString blueAttribute() const;

    /**
     * Sets the \a attribute to use for the blue channel.
     *
     * \see setRedAttribute()
     * \see setGreenAttribute()
     * \see blueAttribute()
     */
    void setBlueAttribute( const QString &attribute );

    /**
     * Returns the contrast enhancement to use for the red channel.
     *
     * \see setRedContrastEnhancement()
     * \see greenContrastEnhancement()
     * \see blueContrastEnhancement()
     */
    const QgsContrastEnhancement *redContrastEnhancement() const;

    /**
     * Sets the contrast \a enhancement to use for the red channel.
     *
     * Ownership of \a enhancement is transferred.
     *
     * \see redContrastEnhancement()
     * \see setGreenContrastEnhancement()
     * \see setBlueContrastEnhancement()
     */
    void setRedContrastEnhancement( QgsContrastEnhancement *enhancement SIP_TRANSFER );

    /**
     * Returns the contrast enhancement to use for the green channel.
     *
     * \see setGreenContrastEnhancement()
     * \see redContrastEnhancement()
     * \see blueContrastEnhancement()
     */
    const QgsContrastEnhancement *greenContrastEnhancement() const;

    /**
     * Sets the contrast \a enhancement to use for the green channel.
     *
     * Ownership of \a enhancement is transferred.
     *
     * \see greenContrastEnhancement()
     * \see setRedContrastEnhancement()
     * \see setBlueContrastEnhancement()
     */
    void setGreenContrastEnhancement( QgsContrastEnhancement *enhancement SIP_TRANSFER );

    /**
     * Returns the contrast enhancement to use for the blue channel.
     *
     * \see setBlueContrastEnhancement()
     * \see redContrastEnhancement()
     * \see greenContrastEnhancement()
     */
    const QgsContrastEnhancement *blueContrastEnhancement() const;

    /**
     * Sets the contrast \a enhancement to use for the blue channel.
     *
     * Ownership of \a enhancement is transferred.
     *
     * \see blueContrastEnhancement()
     * \see setRedContrastEnhancement()
     * \see setGreenContrastEnhancement()
     */
    void setBlueContrastEnhancement( QgsContrastEnhancement *enhancement SIP_TRANSFER );

  private:

    QString mRedAttribute = QStringLiteral( "Red" );
    QString mGreenAttribute = QStringLiteral( "Green" );
    QString mBlueAttribute = QStringLiteral( "Blue" );

    std::unique_ptr< QgsContrastEnhancement > mRedContrastEnhancement;
    std::unique_ptr< QgsContrastEnhancement > mGreenContrastEnhancement;
    std::unique_ptr< QgsContrastEnhancement > mBlueContrastEnhancement;

};

#endif // QGSPOINTCLOUDRGBRENDERER_H
