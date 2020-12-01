/***************************************************************************
                         qgspointcloudattributebyramprenderer.h
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

#ifndef QGSPOINTCLOUDATTRIBUTEBYRAMPRENDERER_H
#define QGSPOINTCLOUDATTRIBUTEBYRAMPRENDERER_H

#include "qgspointcloudrenderer.h"
#include "qgis_core.h"
#include "qgis_sip.h"

class QgsColorRamp;

/**
 * \ingroup core
 * An RGB renderer for 2d visualisation of point clouds using embedded red, green and blue attributes.
 *
 * \since QGIS 3.18
 */
class CORE_EXPORT QgsPointCloudAttributeByRampRenderer : public QgsPointCloudRenderer
{
  public:

    /**
     * Constructor for QgsPointCloudAttributeByRampRenderer.
     */
    QgsPointCloudAttributeByRampRenderer();

    QString type() const override;
    QgsPointCloudRenderer *clone() const override;
    void renderBlock( const QgsPointCloudBlock *block, QgsPointCloudRenderContext &context ) override;
    QDomElement save( QDomDocument &doc, const QgsReadWriteContext &context ) const override;
    void startRender( QgsPointCloudRenderContext &context ) override;
    void stopRender( QgsPointCloudRenderContext &context ) override;
    QSet< QString > usedAttributes( const QgsPointCloudRenderContext &context ) const override;

    /**
     * Creates an RGB renderer from an XML \a element.
     */
    static QgsPointCloudRenderer *create( QDomElement &element, const QgsReadWriteContext &context ) SIP_FACTORY;

    /**
     * Returns the attribute to use for the renderer.
     *
     * \see setAttribute()
     */
    QString attribute() const;

    /**
     * Sets the \a attribute to use for the renderer.
     *
     * \see attribute()
     */
    void setAttribute( const QString &attribute );

    /**
     * Returns the color ramp used for rendering the attribute.
     *
     * \see setColorRamp()
     */
    QgsColorRamp *colorRamp() const;

    /**
     * Sets the color \a ramp used for rendering the attribute.
     *
     * Ownership of \a ramp is transferrred to the renderer.
     *
     * \see colorRamp()
     */
    void setColorRamp( QgsColorRamp *ramp SIP_TRANSFER );

    //! Returns min
    double min() const;
    //! Sets min
    void setMin( double value );

    //! Returns max
    double max() const;

    //! Sets max
    void setMax( double value );

  private:

    int mPainterPenWidth = 1;

    double mMin = 0;
    double mMax = 100;

    QString mAttribute = QStringLiteral( "Intensity" );

    std::unique_ptr<QgsColorRamp> mColorRamp;

};

#endif // QGSPOINTCLOUDATTRIBUTEBYRAMPRENDERER_H
