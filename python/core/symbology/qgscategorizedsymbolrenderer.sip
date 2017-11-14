/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/symbology/qgscategorizedsymbolrenderer.h                    *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsRendererCategory
{
%Docstring
 categorized renderer
%End

%TypeHeaderCode
#include "qgscategorizedsymbolrenderer.h"
%End
  public:

    QgsRendererCategory();
%Docstring
 Constructor for QgsRendererCategory.
%End

    QgsRendererCategory( const QVariant &value, QgsSymbol *symbol /Transfer/, const QString &label, bool render = true );
%Docstring
takes ownership of symbol
%End

    QgsRendererCategory( const QgsRendererCategory &cat );
%Docstring
copy constructor
%End


    QVariant value() const;
%Docstring
 :rtype: QVariant
%End
    QgsSymbol *symbol() const;
%Docstring
 :rtype: QgsSymbol
%End
    QString label() const;
%Docstring
 :rtype: str
%End

    void setValue( const QVariant &value );
    void setSymbol( QgsSymbol *s /Transfer/ );
    void setLabel( const QString &label );

    bool renderState() const;
%Docstring
 :rtype: bool
%End
    void setRenderState( bool render );

    QString dump() const;
%Docstring
 :rtype: str
%End

    void toSld( QDomDocument &doc, QDomElement &element, QgsStringMap props ) const;

  protected:

    void swap( QgsRendererCategory &other );
};

typedef QList<QgsRendererCategory> QgsCategoryList;

class QgsCategorizedSymbolRenderer : QgsFeatureRenderer
{

%TypeHeaderCode
#include "qgscategorizedsymbolrenderer.h"
%End
  public:

    QgsCategorizedSymbolRenderer( const QString &attrName = QString(), const QgsCategoryList &categories = QgsCategoryList() );

    virtual QgsSymbol *symbolForFeature( QgsFeature &feature, QgsRenderContext &context );
    virtual QgsSymbol *originalSymbolForFeature( QgsFeature &feature, QgsRenderContext &context );
    virtual void startRender( QgsRenderContext &context, const QgsFields &fields );
    virtual void stopRender( QgsRenderContext &context );
    virtual QSet<QString> usedAttributes( const QgsRenderContext &context ) const;
    virtual QString dump() const;
    virtual QgsCategorizedSymbolRenderer *clone() const /Factory/;
    virtual void toSld( QDomDocument &doc, QDomElement &element, const QgsStringMap &props = QgsStringMap() ) const;
    virtual QgsFeatureRenderer::Capabilities capabilities();
    virtual QString filter( const QgsFields &fields = QgsFields() );
    virtual QgsSymbolList symbols( QgsRenderContext &context );

    void updateSymbols( QgsSymbol *sym );
%Docstring
 Update all the symbols but leave categories and colors. This method also sets the source
 symbol for the renderer.
 \param sym source symbol to use for categories. Ownership is not transferred.
.. seealso:: setSourceSymbol()
%End

    const QgsCategoryList &categories() const;
%Docstring
 :rtype: QgsCategoryList
%End

    int categoryIndexForValue( const QVariant &val );
%Docstring
return index of category with specified value (-1 if not found)
 :rtype: int
%End

    int categoryIndexForLabel( const QString &val );
%Docstring
 return index of category with specified label (-1 if not found or not unique)
.. versionadded:: 2.5
 :rtype: int
%End

    bool updateCategoryValue( int catIndex, const QVariant &value );
%Docstring
 :rtype: bool
%End
    bool updateCategorySymbol( int catIndex, QgsSymbol *symbol /Transfer/ );
%Docstring
 :rtype: bool
%End
    bool updateCategoryLabel( int catIndex, const QString &label );
%Docstring
 :rtype: bool
%End

    bool updateCategoryRenderState( int catIndex, bool render );
%Docstring
.. versionadded:: 2.5
 :rtype: bool
%End

    void addCategory( const QgsRendererCategory &category );
    bool deleteCategory( int catIndex );
%Docstring
 :rtype: bool
%End
    void deleteAllCategories();

    void moveCategory( int from, int to );
%Docstring
Moves the category at index position from to index position to.
%End

    void sortByValue( Qt::SortOrder order = Qt::AscendingOrder );
    void sortByLabel( Qt::SortOrder order = Qt::AscendingOrder );

    QString classAttribute() const;
%Docstring
 :rtype: str
%End
    void setClassAttribute( const QString &attr );

    static QgsFeatureRenderer *create( QDomElement &element, const QgsReadWriteContext &context ) /Factory/;
%Docstring
create renderer from XML element
 :rtype: QgsFeatureRenderer
%End

    virtual QDomElement save( QDomDocument &doc, const QgsReadWriteContext &context );
    virtual QgsLegendSymbolList legendSymbolItems() const;

    virtual QSet< QString > legendKeysForFeature( QgsFeature &feature, QgsRenderContext &context );

    QgsSymbol *sourceSymbol();
%Docstring
 Returns the renderer's source symbol, which is the base symbol used for the each categories' symbol before applying
 the categories' color.
.. seealso:: setSourceSymbol()
.. seealso:: sourceColorRamp()
 :rtype: QgsSymbol
%End

    void setSourceSymbol( QgsSymbol *sym /Transfer/ );
%Docstring
 Sets the source symbol for the renderer, which is the base symbol used for the each categories' symbol before applying
 the categories' color.
 \param sym source symbol, ownership is transferred to the renderer
.. seealso:: sourceSymbol()
.. seealso:: setSourceColorRamp()
%End

    QgsColorRamp *sourceColorRamp();
%Docstring
 Returns the source color ramp, from which each categories' color is derived.
.. seealso:: setSourceColorRamp()
.. seealso:: sourceSymbol()
 :rtype: QgsColorRamp
%End

    void setSourceColorRamp( QgsColorRamp *ramp /Transfer/ );
%Docstring
 Sets the source color ramp.
 \param ramp color ramp. Ownership is transferred to the renderer
.. seealso:: sourceColorRamp()
.. seealso:: setSourceSymbol()
%End

    void updateColorRamp( QgsColorRamp *ramp /Transfer/ );
%Docstring
 Update the color ramp used and all symbols colors.
 \param ramp color ramp. Ownership is transferred to the renderer
.. versionadded:: 2.5
%End

    virtual bool legendSymbolItemsCheckable() const;
    virtual bool legendSymbolItemChecked( const QString &key );
    virtual void setLegendSymbolItem( const QString &key, QgsSymbol *symbol /Transfer/ );
    virtual void checkLegendSymbolItem( const QString &key, bool state = true );
    virtual QString legendClassificationAttribute() const;

    static QgsCategorizedSymbolRenderer *convertFromRenderer( const QgsFeatureRenderer *renderer ) /Factory/;
%Docstring
 creates a QgsCategorizedSymbolRenderer from an existing renderer.
.. versionadded:: 2.5
 :return: a new renderer if the conversion was possible, otherwise 0.
 :rtype: QgsCategorizedSymbolRenderer
%End

    void setDataDefinedSizeLegend( QgsDataDefinedSizeLegend *settings /Transfer/ );
%Docstring
 Configures appearance of legend when renderer is configured to use data-defined size for marker symbols.
 This allows configuring for which values (symbol sizes) should be shown in the legend, whether to display
 different symbol sizes collapsed in one legend node or separated across multiple legend nodes etc.

 When renderer does not use data-defined size or does not use marker symbols, these settings will be ignored.
 Takes ownership of the passed settings objects. Null pointer is a valid input that disables data-defined
 size legend.
.. versionadded:: 3.0
%End

    QgsDataDefinedSizeLegend *dataDefinedSizeLegend() const;
%Docstring
 Returns configuration of appearance of legend when using data-defined size for marker symbols.
 Will return null if the functionality is disabled.
.. versionadded:: 3.0
 :rtype: QgsDataDefinedSizeLegend
%End

  protected:




    void rebuildHash();
%Docstring
hashtable for faster access to symbols
%End

    QgsSymbol *skipRender();
%Docstring
 :rtype: QgsSymbol
%End

    QgsSymbol *symbolForValue( const QVariant &value );
%Docstring
 :rtype: QgsSymbol
%End

  private:
    QgsCategorizedSymbolRenderer( const QgsCategorizedSymbolRenderer & );
    QgsCategorizedSymbolRenderer &operator=( const QgsCategorizedSymbolRenderer & );
%Docstring
 :rtype: QgsCategorizedSymbolRenderer
%End
};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/symbology/qgscategorizedsymbolrenderer.h                    *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
