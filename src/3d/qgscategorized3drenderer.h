/***************************************************************************
  qgscategorized3drenderer.h
  --------------------------------------
  Date                 : November 2025
  Copyright            : (C) 2025 by Jean Felder
  Email                : jean dot felder at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCATEGORIZED3DRENDERER_H
#define QGSCATEGORIZED3DRENDERER_H

#include <memory>

#include "qgis_3d.h"
#include "qgis_sip.h"
#include "qgs3drendererregistry.h"
#include "qgsabstract3dsymbol.h"
#include "qgsabstractvectorlayer3drenderer.h"

class Qgs3DRenderContext;
class QgsFeature3DHandler;
class QgsExpression;


/**
 * \ingroup qgis_3d
 * \brief Metadata for Categorized 3D renderer to allow creation of its instances from XML.
 *
 * \warning This is not considered stable API, and may change in future QGIS releases. It is
 * exposed to the Python bindings as a tech preview only.
 *
 * \since QGIS 4.0
 */
class _3D_EXPORT QgsCategorized3DRendererMetadata : public Qgs3DRendererAbstractMetadata
{
  public:
    QgsCategorized3DRendererMetadata();

    //! Creates an instance of a 3D renderer Categorized on a DOM element with renderer configuration
    QgsAbstract3DRenderer *createRenderer( QDomElement &elem, const QgsReadWriteContext &context ) override SIP_FACTORY;
};


/**
 * \ingroup qgis_3d
 * \brief Represents an individual category (class) from a QgsCategorized3DRenderer.
 *
 * \warning This is not considered stable API, and may change in future QGIS releases. It is
 * exposed to the Python bindings as a tech preview only.
 *
 * \since QGIS 4.0
 */
class _3D_EXPORT Qgs3DRendererCategory
{
  public:
    Qgs3DRendererCategory() = default;

    /**
    * Constructor for a new QgsRendererCategory, with the specified \a value and \a symbol.
    *
    * If \a value is a list, then the category will match any of the values from this list.
    *
    * The ownership of \a symbol is transferred to the category.
    *
    * The \a render argument indicates whether the category should initially be rendered and appear checked in the layer tree.
    */
    Qgs3DRendererCategory( const QVariant &value, QgsAbstract3DSymbol *symbol SIP_TRANSFER, bool render = true );

    Qgs3DRendererCategory( const Qgs3DRendererCategory &other );
    Qgs3DRendererCategory &operator=( const Qgs3DRendererCategory &other );

    /**
     * Returns the value corresponding to this category.
     *
     * If the returned value is a list, then the category will match any of the values from this list.
     *
     * \see setValue()
     */
    const QVariant value() const { return mValue; }

    /**
     * Returns the symbol which will be used to render this category.
     * \see setSymbol()
     */
    QgsAbstract3DSymbol *symbol() const { return mSymbol.get(); }

    /**
     * Returns TRUE if the category is currently enabled and should be rendered.
     * \see setRenderState()
     */
    bool renderState() const { return mRender; }

    /**
     * Sets the \a value corresponding to this category.
     *
     * If \a value is a list, then the category will match any of the values from this list.
     *
     * \see value()
     */
    void setValue( const QVariant &value );

    /**
     * Sets the symbol which will be used to render this category.
     *
     * Ownership of the symbol is transferred to the category.
     *
     * \see symbol()
     */
    void setSymbol( QgsAbstract3DSymbol *symbol SIP_TRANSFER );

    /**
     * Sets whether the category is currently enabled and should be rendered.
     * \see renderState()
     */
    void setRenderState( bool render );

  private:
    QVariant mValue;
    std::unique_ptr<QgsAbstract3DSymbol> mSymbol;
    bool mRender = true;
};

using Qgs3DCategoryList = QList<Qgs3DRendererCategory>;


/**
 * \ingroup qgis_3d
 * \brief Categorized 3D renderer.
 *
 * Similar to Categorized 2D renderer and Categorized labeling, it allows specification of rules for 3D symbols.
 *
 * \warning This is not considered stable API, and may change in future QGIS releases. It is
 * exposed to the Python bindings as a tech preview only.
 *
 * \since QGIS 4.0
 */
class _3D_EXPORT QgsCategorized3DRenderer : public QgsAbstractVectorLayer3DRenderer
{
  public:
    //! Construct renderer with the given categories
    QgsCategorized3DRenderer( const QString &attributeName = QString(), const Qgs3DCategoryList &categories = Qgs3DCategoryList() );

    QString type() const override { return "categorized"; }
    QgsCategorized3DRenderer *clone() const override SIP_FACTORY;
    Qt3DCore::QEntity *createEntity( Qgs3DMapSettings *mapSettings ) const override SIP_SKIP;

    Qgs3DCategoryList categories() const
    {
      return mCategories;
    }

    /**
     * Returns the class attribute for the renderer, which is the field name
     * or expression string from the layer which will be matched against the
     * renderer categories.
     *
     * \see setClassAttribute()
     */
    QString classAttribute() const
    {
      return mAttributeName;
    }

    /**
     * Sets the class attribute for the renderer, which is the field name
     * or expression string from the layer which will be matched against the
     * renderer categories.
     *
     * \see classAttribute()
     */
    void setClassAttribute( QString attributeName );

    void writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const override;
    void readXml( const QDomElement &elem, const QgsReadWriteContext &context ) override;

  private:
    QString mAttributeName;
    Qgs3DCategoryList mCategories;
};

#endif // QGSCATEGORIZED3DRENDERER_H
