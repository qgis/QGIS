/***************************************************************************
                         qgspointcloudclassifiedrenderer.h
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

#ifndef QGSPOINTCLOUDCLASSIFIEDRENDERER_H
#define QGSPOINTCLOUDCLASSIFIEDRENDERER_H

#include "qgspointcloudrenderer.h"
#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgscolorrampshader.h"


/**
 * \ingroup core
 * \brief Represents an individual category (class) from a QgsPointCloudClassifiedRenderer.
 * \since QGIS 3.18
*/
class CORE_EXPORT QgsPointCloudCategory
{
  public:

    /**
     * Constructor for QgsPointCloudCategory.
     */
    QgsPointCloudCategory() = default;

    /**
    * Constructor for a new QgsPointCloudCategory, with the specified \a value and \a color.
    *
    * The \a label argument specifies the label used for this category in legends and the layer tree.
    *
    * The \a render argument indicates whether the category should initially be rendered and appear checked in the layer tree.
    */
    QgsPointCloudCategory( int value, const QColor &color, const QString &label, bool render = true );

    /**
     * Equality operator.
     * \since QGIS 3.26
     */
    bool operator==( const QgsPointCloudCategory &other ) const;

    /**
     * Returns the value corresponding to this category.
     *
     * \see setValue()
     */
    int value() const { return mValue; }

    /**
     * Returns the color which will be used to render this category.
     * \see setColor()
     */
    QColor color() const { return mColor; }

    /**
     * Returns the label for this category, which is used to represent the category within
     * legends and the layer tree.
     * \see setLabel()
     */
    QString label() const { return mLabel; }

    /**
     * Sets the \a value corresponding to this category.
     *
     * \see value()
     */
    void setValue( int value ) { mValue = value; }

    /**
     * Sets the \a color which will be used to render this category.
     *
     * \see color()
     */
    void setColor( const QColor &color ) { mColor = color; }

    /**
     * Sets the \a label for this category, which is used to represent the category within
     * legends and the layer tree.
     * \see label()
     */
    void setLabel( const QString &label ) { mLabel = label; }

    /**
     * Returns TRUE if the category is currently enabled and should be rendered.
     * \see setRenderState()
     */
    bool renderState() const { return mRender; }

    /**
     * Sets whether the category is currently enabled and should be rendered.
     * \see renderState()
     */
    void setRenderState( bool render ) { mRender = render; }

  protected:
    int mValue = 0;
    QColor mColor;
    QString mLabel;
    bool mRender = true;
};

typedef QList<QgsPointCloudCategory> QgsPointCloudCategoryList;

#ifndef SIP_RUN

/**
 * \ingroup core
 * \brief Prepared data container for QgsPointCloudClassifiedRenderer.
 *
 * \note Not available in Python bindings.
 *
 * \since QGIS 3.26
 */
class CORE_EXPORT QgsPointCloudClassifiedRendererPreparedData: public QgsPreparedPointCloudRendererData
{
  public:

    QSet< QString > usedAttributes() const override;
    bool prepareBlock( const QgsPointCloudBlock *block ) override;
    QColor pointColor( const QgsPointCloudBlock *block, int i, double z ) override SIP_SKIP;

    QgsPointCloudAttribute::DataType attributeType;
    QHash< int, QColor > colors;
    QString attributeName;
    int attributeOffset = 0;
};
#endif

/**
 * \ingroup core
 * \brief Renders point clouds by a classification attribute.
 *
 * \since QGIS 3.18
 */
class CORE_EXPORT QgsPointCloudClassifiedRenderer : public QgsPointCloudRenderer
{
  public:

    /**
     * Constructor for QgsPointCloudClassifiedRenderer.
     */
    QgsPointCloudClassifiedRenderer();

    QString type() const override;
    QgsPointCloudRenderer *clone() const override;
    void renderBlock( const QgsPointCloudBlock *block, QgsPointCloudRenderContext &context ) override;
    bool willRenderPoint( const QVariantMap &pointAttributes ) override;
    QDomElement save( QDomDocument &doc, const QgsReadWriteContext &context ) const override;
    QSet< QString > usedAttributes( const QgsPointCloudRenderContext &context ) const override;
    QList<QgsLayerTreeModelLegendNode *> createLegendNodes( QgsLayerTreeLayer *nodeLayer ) override SIP_FACTORY;
    QStringList legendRuleKeys() const override;
    bool legendItemChecked( const QString &key ) override;
    void checkLegendItem( const QString &key, bool state = true ) override;
    std::unique_ptr< QgsPreparedPointCloudRendererData > prepare() override SIP_SKIP;

    /**
     * Creates an RGB renderer from an XML \a element.
     */
    static QgsPointCloudRenderer *create( QDomElement &element, const QgsReadWriteContext &context ) SIP_FACTORY;

    /**
     * Returns the default list of categories.
     */
    static QgsPointCloudCategoryList defaultCategories();

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
     * Returns the classification categories used for rendering.
     *
     * \see setCategories()
     */
    QgsPointCloudCategoryList categories() const;

    /**
     * Sets the classification \a categories used for rendering.
     *
     * \see categories()
     */
    void setCategories( const QgsPointCloudCategoryList &categories );

    /**
     * Adds a \a category to the renderer.
     *
     * \see categories()
     */
    void addCategory( const QgsPointCloudCategory &category );

  private:

    QString mAttribute = QStringLiteral( "Classification" );

    QgsPointCloudCategoryList mCategories;
};

#endif // QGSPOINTCLOUDCLASSIFIEDRENDERER_H
