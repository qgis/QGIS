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
#include "qgscolorrampshader.h"

#ifndef SIP_RUN

/**
 * \ingroup core
 * \brief Prepared data container for QgsPointCloudAttributeByRampRenderer.
 *
 * \note Not available in Python bindings.
 *
 * \since QGIS 3.26
 */
class CORE_EXPORT QgsPointCloudAttributeByRampRendererPreparedData: public QgsPreparedPointCloudRendererData
{
  public:

    QSet< QString > usedAttributes() const override;
    bool prepareBlock( const QgsPointCloudBlock *block ) override;
    QColor pointColor( const QgsPointCloudBlock *block, int i, double z ) override SIP_SKIP;

    QString attributeName;
    QgsColorRampShader colorRampShader;
    int attributeOffset = 0;
    bool attributeIsX = false;
    bool attributeIsY = false;
    bool attributeIsZ = false;
    QgsPointCloudAttribute::DataType attributeType;
};
#endif


/**
 * \ingroup core
 * \brief An RGB renderer for 2d visualisation of point clouds using embedded red, green and blue attributes.
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
    QSet< QString > usedAttributes( const QgsPointCloudRenderContext &context ) const override;
    QList<QgsLayerTreeModelLegendNode *> createLegendNodes( QgsLayerTreeLayer *nodeLayer ) override SIP_FACTORY;
    std::unique_ptr< QgsPreparedPointCloudRendererData > prepare() override SIP_SKIP;

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
     * Returns the color ramp shader function used to visualize the attribute.
     *
     * \see setColorRampShader()
     */
    QgsColorRampShader colorRampShader() const;

    /**
     * Sets the color ramp \a shader function used to visualize the attribute.
     *
     * \see colorRampShader()
     */
    void setColorRampShader( const QgsColorRampShader &shader );

    /**
     * Returns the minimum value for attributes which will be used by the color ramp shader.
     *
     * \see setMinimum()
     * \see maximum()
     */
    double minimum() const;

    /**
     * Sets the \a minimum value for attributes which will be used by the color ramp shader.
     *
     * \see minimum()
     * \see setMaximum()
     */
    void setMinimum( double minimum );

    /**
     * Returns the maximum value for attributes which will be used by the color ramp shader.
     *
     * \see setMaximum()
     * \see minimum()
     */
    double maximum() const;

    /**
     * Sets the \a maximum value for attributes which will be used by the color ramp shader.
     *
     * \see maximum()
     * \see setMinimum()
     */
    void setMaximum( double maximum );

  private:

    double mMin = 0;
    double mMax = 100;

    QString mAttribute = QStringLiteral( "Intensity" );
    QgsColorRampShader mColorRampShader;

};

#endif // QGSPOINTCLOUDATTRIBUTEBYRAMPRENDERER_H
