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
    QgsSymbol *symbol() const;
    QString label() const;

    void setValue( const QVariant &value );
    void setSymbol( QgsSymbol *s /Transfer/ );
    void setLabel( const QString &label );

    bool renderState() const;
    void setRenderState( bool render );

    QString dump() const;

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

:param sym: source symbol to use for categories. Ownership is not transferred.

.. seealso:: :py:func:`setSourceSymbol`
%End

    const QgsCategoryList &categories() const;

    int categoryIndexForValue( const QVariant &val );
%Docstring
return index of category with specified value (-1 if not found)
%End

    int categoryIndexForLabel( const QString &val );
%Docstring
return index of category with specified label (-1 if not found or not unique)

.. versionadded:: 2.5
%End

    bool updateCategoryValue( int catIndex, const QVariant &value );
    bool updateCategorySymbol( int catIndex, QgsSymbol *symbol /Transfer/ );
    bool updateCategoryLabel( int catIndex, const QString &label );

    bool updateCategoryRenderState( int catIndex, bool render );
%Docstring

.. versionadded:: 2.5
%End

    void addCategory( const QgsRendererCategory &category );
    bool deleteCategory( int catIndex );
    void deleteAllCategories();

    void moveCategory( int from, int to );
%Docstring
Moves the category at index position from to index position to.
%End

    void sortByValue( Qt::SortOrder order = Qt::AscendingOrder );
    void sortByLabel( Qt::SortOrder order = Qt::AscendingOrder );

    QString classAttribute() const;
    void setClassAttribute( const QString &attr );

    static QgsFeatureRenderer *create( QDomElement &element, const QgsReadWriteContext &context ) /Factory/;
%Docstring
create renderer from XML element
%End

    virtual QDomElement save( QDomDocument &doc, const QgsReadWriteContext &context );

    virtual QgsLegendSymbolList legendSymbolItems() const;

    virtual QSet< QString > legendKeysForFeature( QgsFeature &feature, QgsRenderContext &context );


    QgsSymbol *sourceSymbol();
%Docstring
Returns the renderer's source symbol, which is the base symbol used for the each categories' symbol before applying
the categories' color.

.. seealso:: :py:func:`setSourceSymbol`

.. seealso:: :py:func:`sourceColorRamp`
%End

    void setSourceSymbol( QgsSymbol *sym /Transfer/ );
%Docstring
Sets the source symbol for the renderer, which is the base symbol used for the each categories' symbol before applying
the categories' color.

:param sym: source symbol, ownership is transferred to the renderer

.. seealso:: :py:func:`sourceSymbol`

.. seealso:: :py:func:`setSourceColorRamp`
%End

    QgsColorRamp *sourceColorRamp();
%Docstring
Returns the source color ramp, from which each categories' color is derived.

.. seealso:: :py:func:`setSourceColorRamp`

.. seealso:: :py:func:`sourceSymbol`
%End

    void setSourceColorRamp( QgsColorRamp *ramp /Transfer/ );
%Docstring
Sets the source color ramp.

:param ramp: color ramp. Ownership is transferred to the renderer

.. seealso:: :py:func:`sourceColorRamp`

.. seealso:: :py:func:`setSourceSymbol`
%End

    void updateColorRamp( QgsColorRamp *ramp /Transfer/ );
%Docstring
Update the color ramp used and all symbols colors.

:param ramp: color ramp. Ownership is transferred to the renderer

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

:return: a new renderer if the conversion was possible, otherwise 0.

.. versionadded:: 2.5
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
%End

  protected:




    void rebuildHash();
%Docstring
hashtable for faster access to symbols
%End

    QgsSymbol *skipRender();

    QgsSymbol *symbolForValue( const QVariant &value );

  private:
    QgsCategorizedSymbolRenderer( const QgsCategorizedSymbolRenderer & );
    QgsCategorizedSymbolRenderer &operator=( const QgsCategorizedSymbolRenderer & );
};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/symbology/qgscategorizedsymbolrenderer.h                    *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
