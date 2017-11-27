/***************************************************************************
    qgscategorizedsymbolrenderer.h
    ---------------------
    begin                : November 2009
    copyright            : (C) 2009 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSCATEGORIZEDSYMBOLRENDERERV2_H
#define QGSCATEGORIZEDSYMBOLRENDERERV2_H

#include "qgis_core.h"
#include "qgis.h"
#include "qgssymbol.h"
#include "qgsrenderer.h"
#include "qgsexpression.h"
#include "qgscolorramp.h"
#include "qgsdatadefinedsizelegend.h"

#include <QHash>

class QgsVectorLayer;

/**
 * \ingroup core
 * \brief categorized renderer
*/
class CORE_EXPORT QgsRendererCategory
{
  public:

    /**
     * Constructor for QgsRendererCategory.
     */
    QgsRendererCategory() = default;

    //! takes ownership of symbol
    QgsRendererCategory( const QVariant &value, QgsSymbol *symbol SIP_TRANSFER, const QString &label, bool render = true );

    //! copy constructor
    QgsRendererCategory( const QgsRendererCategory &cat );

    QgsRendererCategory &operator=( QgsRendererCategory cat );

    QVariant value() const;
    QgsSymbol *symbol() const;
    QString label() const;

    void setValue( const QVariant &value );
    void setSymbol( QgsSymbol *s SIP_TRANSFER );
    void setLabel( const QString &label );

    // \since QGIS 2.5
    bool renderState() const;
    void setRenderState( bool render );

    // debugging
    QString dump() const;

    void toSld( QDomDocument &doc, QDomElement &element, QgsStringMap props ) const;

  protected:
    QVariant mValue;
    std::unique_ptr<QgsSymbol> mSymbol;
    QString mLabel;
    bool mRender = true;

    void swap( QgsRendererCategory &other );
};

typedef QList<QgsRendererCategory> QgsCategoryList;

/**
 * \ingroup core
 * \class QgsCategorizedSymbolRenderer
 */
class CORE_EXPORT QgsCategorizedSymbolRenderer : public QgsFeatureRenderer
{
  public:

    QgsCategorizedSymbolRenderer( const QString &attrName = QString(), const QgsCategoryList &categories = QgsCategoryList() );

    virtual QgsSymbol *symbolForFeature( QgsFeature &feature, QgsRenderContext &context ) override;
    virtual QgsSymbol *originalSymbolForFeature( QgsFeature &feature, QgsRenderContext &context ) override;
    virtual void startRender( QgsRenderContext &context, const QgsFields &fields ) override;
    virtual void stopRender( QgsRenderContext &context ) override;
    virtual QSet<QString> usedAttributes( const QgsRenderContext &context ) const override;
    virtual QString dump() const override;
    virtual QgsCategorizedSymbolRenderer *clone() const override SIP_FACTORY;
    virtual void toSld( QDomDocument &doc, QDomElement &element, const QgsStringMap &props = QgsStringMap() ) const override;
    virtual QgsFeatureRenderer::Capabilities capabilities() override { return SymbolLevels | Filter; }
    virtual QString filter( const QgsFields &fields = QgsFields() ) override;
    virtual QgsSymbolList symbols( QgsRenderContext &context ) override;

    /**
     * Update all the symbols but leave categories and colors. This method also sets the source
     * symbol for the renderer.
     * \param sym source symbol to use for categories. Ownership is not transferred.
     * \see setSourceSymbol()
     */
    void updateSymbols( QgsSymbol *sym );

    const QgsCategoryList &categories() const { return mCategories; }

    //! return index of category with specified value (-1 if not found)
    int categoryIndexForValue( const QVariant &val );

    /**
     * return index of category with specified label (-1 if not found or not unique)
     * \since QGIS 2.5
     */
    int categoryIndexForLabel( const QString &val );

    bool updateCategoryValue( int catIndex, const QVariant &value );
    bool updateCategorySymbol( int catIndex, QgsSymbol *symbol SIP_TRANSFER );
    bool updateCategoryLabel( int catIndex, const QString &label );

    //! \since QGIS 2.5
    bool updateCategoryRenderState( int catIndex, bool render );

    void addCategory( const QgsRendererCategory &category );
    bool deleteCategory( int catIndex );
    void deleteAllCategories();

    //! Moves the category at index position from to index position to.
    void moveCategory( int from, int to );

    void sortByValue( Qt::SortOrder order = Qt::AscendingOrder );
    void sortByLabel( Qt::SortOrder order = Qt::AscendingOrder );

    QString classAttribute() const { return mAttrName; }
    void setClassAttribute( const QString &attr ) { mAttrName = attr; }

    //! create renderer from XML element
    static QgsFeatureRenderer *create( QDomElement &element, const QgsReadWriteContext &context ) SIP_FACTORY;

    virtual QDomElement save( QDomDocument &doc, const QgsReadWriteContext &context ) override;
    QgsLegendSymbolList legendSymbolItems() const override;
    virtual QSet< QString > legendKeysForFeature( QgsFeature &feature, QgsRenderContext &context ) override;

    /**
     * Returns the renderer's source symbol, which is the base symbol used for the each categories' symbol before applying
     * the categories' color.
     * \see setSourceSymbol()
     * \see sourceColorRamp()
     */
    QgsSymbol *sourceSymbol();

    /**
     * Sets the source symbol for the renderer, which is the base symbol used for the each categories' symbol before applying
     * the categories' color.
     * \param sym source symbol, ownership is transferred to the renderer
     * \see sourceSymbol()
     * \see setSourceColorRamp()
     */
    void setSourceSymbol( QgsSymbol *sym SIP_TRANSFER );

    /**
     * Returns the source color ramp, from which each categories' color is derived.
     * \see setSourceColorRamp()
     * \see sourceSymbol()
     */
    QgsColorRamp *sourceColorRamp();

    /**
     * Sets the source color ramp.
      * \param ramp color ramp. Ownership is transferred to the renderer
      * \see sourceColorRamp()
      * \see setSourceSymbol()
      */
    void setSourceColorRamp( QgsColorRamp *ramp SIP_TRANSFER );

    /**
     * Update the color ramp used and all symbols colors.
      * \param ramp color ramp. Ownership is transferred to the renderer
      * \since QGIS 2.5
      */
    void updateColorRamp( QgsColorRamp *ramp SIP_TRANSFER );

    virtual bool legendSymbolItemsCheckable() const override;
    virtual bool legendSymbolItemChecked( const QString &key ) override;
    virtual void setLegendSymbolItem( const QString &key, QgsSymbol *symbol SIP_TRANSFER ) override;
    virtual void checkLegendSymbolItem( const QString &key, bool state = true ) override;
    virtual QString legendClassificationAttribute() const override { return classAttribute(); }

    /**
     * creates a QgsCategorizedSymbolRenderer from an existing renderer.
     * \since QGIS 2.5
     * \returns a new renderer if the conversion was possible, otherwise 0.
     */
    static QgsCategorizedSymbolRenderer *convertFromRenderer( const QgsFeatureRenderer *renderer ) SIP_FACTORY;

    /**
     * Configures appearance of legend when renderer is configured to use data-defined size for marker symbols.
     * This allows configuring for which values (symbol sizes) should be shown in the legend, whether to display
     * different symbol sizes collapsed in one legend node or separated across multiple legend nodes etc.
     *
     * When renderer does not use data-defined size or does not use marker symbols, these settings will be ignored.
     * Takes ownership of the passed settings objects. Null pointer is a valid input that disables data-defined
     * size legend.
     * \since QGIS 3.0
     */
    void setDataDefinedSizeLegend( QgsDataDefinedSizeLegend *settings SIP_TRANSFER );

    /**
     * Returns configuration of appearance of legend when using data-defined size for marker symbols.
     * Will return null if the functionality is disabled.
     * \since QGIS 3.0
     */
    QgsDataDefinedSizeLegend *dataDefinedSizeLegend() const;

  protected:
    QString mAttrName;
    QgsCategoryList mCategories;
    std::unique_ptr<QgsSymbol> mSourceSymbol;
    std::unique_ptr<QgsColorRamp> mSourceColorRamp;
    std::unique_ptr<QgsExpression> mExpression;

    std::unique_ptr<QgsDataDefinedSizeLegend> mDataDefinedSizeLegend;

    //! attribute index (derived from attribute name in startRender)
    int mAttrNum = -1;

    //! hashtable for faster access to symbols
    QHash<QString, QgsSymbol *> mSymbolHash;
    bool mCounting = false;

    void rebuildHash();

    QgsSymbol *skipRender();

    QgsSymbol *symbolForValue( const QVariant &value );

  private:
#ifdef SIP_RUN
    QgsCategorizedSymbolRenderer( const QgsCategorizedSymbolRenderer & );
    QgsCategorizedSymbolRenderer &operator=( const QgsCategorizedSymbolRenderer & );
#endif

    //! Returns calculated classification value for a feature
    QVariant valueForFeature( QgsFeature &feature, QgsRenderContext &context ) const;

    //! Returns list of legend symbol items from individual categories
    QgsLegendSymbolList baseLegendSymbolItems() const;
};

#endif // QGSCATEGORIZEDSYMBOLRENDERERV2_H
