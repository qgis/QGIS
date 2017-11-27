/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/symbology/qgssymbollayerregistry.h                          *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/



class QgsSymbolLayerWidget /External/;

class QgsSymbolLayerAbstractMetadata
{
%Docstring
Stores metadata about one symbol layer class.

.. note::

   It's necessary to implement createSymbolLayer() function.
In C++ you can use QgsSymbolLayerMetadata convenience class.
%End

%TypeHeaderCode
#include "qgssymbollayerregistry.h"
%End
  public:
    QgsSymbolLayerAbstractMetadata( const QString &name, const QString &visibleName, QgsSymbol::SymbolType type );

    virtual ~QgsSymbolLayerAbstractMetadata();

    QString name() const;
%Docstring
 :rtype: str
%End
    QString visibleName() const;
%Docstring
 :rtype: str
%End
    QgsSymbol::SymbolType type() const;
%Docstring
 :rtype: QgsSymbol.SymbolType
%End

    virtual QgsSymbolLayer *createSymbolLayer( const QgsStringMap &map ) = 0 /Factory/;
%Docstring
Create a symbol layer of this type given the map of properties.
 :rtype: QgsSymbolLayer
%End
    virtual QgsSymbolLayerWidget *createSymbolLayerWidget( QgsVectorLayer * ) /Factory/;
%Docstring
Create widget for symbol layer of this type. Can return NULL if there's no GUI
 :rtype: QgsSymbolLayerWidget
%End
    virtual QgsSymbolLayer *createSymbolLayerFromSld( QDomElement & ) /Factory/;
%Docstring
Create a symbol layer of this type given the map of properties.
 :rtype: QgsSymbolLayer
%End

    virtual void resolvePaths( QgsStringMap &properties, const QgsPathResolver &pathResolver, bool saving );
%Docstring
 Resolve paths in symbol layer's properties (if there are any paths).
 When saving is true, paths are converted from absolute to relative,
 when saving is false, paths are converted from relative to absolute.
 This ensures that paths in project files can be relative, but in symbol layer
 instances the paths are always absolute
.. versionadded:: 3.0
%End

  protected:
};


class QgsSymbolLayerMetadata : QgsSymbolLayerAbstractMetadata
{
%Docstring
Convenience metadata class that uses static functions to create symbol layer and its widget.
%End

%TypeHeaderCode
#include "qgssymbollayerregistry.h"
%End
  public:



    virtual QgsSymbolLayer *createSymbolLayer( const QgsStringMap &map ) /Factory/;
    virtual QgsSymbolLayerWidget *createSymbolLayerWidget( QgsVectorLayer *vl ) /Factory/;
    virtual QgsSymbolLayer *createSymbolLayerFromSld( QDomElement &elem ) /Factory/;
    virtual void resolvePaths( QgsStringMap &properties, const QgsPathResolver &pathResolver, bool saving );

  protected:

  private:
    QgsSymbolLayerMetadata();
};


class QgsSymbolLayerRegistry
{
%Docstring
 Registry of available symbol layer classes.

 QgsSymbolLayerRegistry is not usually directly created, but rather accessed through
 QgsApplication.symbolLayerRegistry().
%End

%TypeHeaderCode
#include "qgssymbollayerregistry.h"
%End
  public:

    QgsSymbolLayerRegistry();
    ~QgsSymbolLayerRegistry();


    QgsSymbolLayerAbstractMetadata *symbolLayerMetadata( const QString &name ) const;
%Docstring
return metadata for specified symbol layer. Returns NULL if not found
 :rtype: QgsSymbolLayerAbstractMetadata
%End

    bool addSymbolLayerType( QgsSymbolLayerAbstractMetadata *metadata /Transfer/ );
%Docstring
register a new symbol layer type. Takes ownership of the metadata instance.
 :rtype: bool
%End

    QgsSymbolLayer *createSymbolLayer( const QString &name, const QgsStringMap &properties = QgsStringMap() ) const /Factory/;
%Docstring
create a new instance of symbol layer given symbol layer name and properties
 :rtype: QgsSymbolLayer
%End

    QgsSymbolLayer *createSymbolLayerFromSld( const QString &name, QDomElement &element ) const /Factory/;
%Docstring
create a new instance of symbol layer given symbol layer name and SLD
 :rtype: QgsSymbolLayer
%End

    void resolvePaths( const QString &name, QgsStringMap &properties, const QgsPathResolver &pathResolver, bool saving ) const;
%Docstring
 Resolve paths in properties of a particular symbol layer.
 This normally means converting relative paths to absolute paths when loading
 and converting absolute paths to relative paths when saving.
.. versionadded:: 3.0
%End

    QStringList symbolLayersForType( QgsSymbol::SymbolType type );
%Docstring
return a list of available symbol layers for a specified symbol type
 :rtype: list of str
%End

    static QgsSymbolLayer *defaultSymbolLayer( QgsSymbol::SymbolType type ) /Factory/;
%Docstring
create a new instance of symbol layer for specified symbol type with default settings
 :rtype: QgsSymbolLayer
%End

  private:
    QgsSymbolLayerRegistry( const QgsSymbolLayerRegistry &rh );
};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/symbology/qgssymbollayerregistry.h                          *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
