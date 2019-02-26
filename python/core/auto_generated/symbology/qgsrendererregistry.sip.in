/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/symbology/qgsrendererregistry.h                             *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/



class QgsRendererWidget /External/;

class QgsRendererAbstractMetadata
{
%Docstring
Stores metadata about one renderer class.

.. note::

   It's necessary to implement createRenderer() function.
   In C++ you can use QgsRendererMetadata convenience class.
%End

%TypeHeaderCode
#include "qgsrendererregistry.h"
%End
  public:

    enum LayerType
    {
      PointLayer,
      LineLayer,
      PolygonLayer,
      All,
    };
    typedef QFlags<QgsRendererAbstractMetadata::LayerType> LayerTypes;


    QgsRendererAbstractMetadata( const QString &name, const QString &visibleName, const QIcon &icon = QIcon() );
    virtual ~QgsRendererAbstractMetadata();

    QString name() const;
    QString visibleName() const;

    QIcon icon() const;
    void setIcon( const QIcon &icon );

    virtual QgsRendererAbstractMetadata::LayerTypes compatibleLayerTypes() const;
%Docstring
Returns flags indicating the types of layer the renderer is compatible with.

.. versionadded:: 2.16
%End

    virtual QgsFeatureRenderer *createRenderer( QDomElement &elem, const QgsReadWriteContext &context ) = 0 /Factory/;
%Docstring
Returns new instance of the renderer given the DOM element. Returns ``None`` on error.
Pure virtual function: must be implemented in derived classes.  *
%End

    virtual QgsRendererWidget *createRendererWidget( QgsVectorLayer *layer, QgsStyle *style, QgsFeatureRenderer *oldRenderer ) /Factory/;
%Docstring
Returns new instance of settings widget for the renderer. Returns ``None`` on error.

The ``oldRenderer`` argument may refer to previously used renderer (or it is ``None``).
If not ``None``, it may be used to initialize GUI of the widget from the previous settings.
The old renderer does not have to be of the same type as returned by createRenderer().
When using ``oldRenderer`` make sure to make a copy of it - it will be deleted afterwards.
%End

    virtual QgsFeatureRenderer *createRendererFromSld( QDomElement &elem, QgsWkbTypes::GeometryType geomType ) /Factory/;

  protected:
};


QFlags<QgsRendererAbstractMetadata::LayerType> operator|(QgsRendererAbstractMetadata::LayerType f1, QFlags<QgsRendererAbstractMetadata::LayerType> f2);




class QgsRendererMetadata : QgsRendererAbstractMetadata
{
%Docstring
Convenience metadata class that uses static functions to create renderer and its widget.
%End

%TypeHeaderCode
#include "qgsrendererregistry.h"
%End
  public:



    virtual QgsFeatureRenderer *createRenderer( QDomElement &elem, const QgsReadWriteContext &context ) /Factory/;
    virtual QgsRendererWidget *createRendererWidget( QgsVectorLayer *layer, QgsStyle *style, QgsFeatureRenderer *renderer ) /Factory/;
    virtual QgsFeatureRenderer *createRendererFromSld( QDomElement &elem, QgsWkbTypes::GeometryType geomType ) /Factory/;



    virtual QgsRendererAbstractMetadata::LayerTypes compatibleLayerTypes() const;

  protected:

  private:
    QgsRendererMetadata();
};



class QgsRendererRegistry
{
%Docstring
Registry of renderers.

QgsRendererRegistry is not usually directly created, but rather accessed through
:py:func:`QgsApplication.rendererRegistry()`
%End

%TypeHeaderCode
#include "qgsrendererregistry.h"
%End
  public:

    QgsRendererRegistry();
    ~QgsRendererRegistry();


    bool addRenderer( QgsRendererAbstractMetadata *metadata /Transfer/ );
%Docstring
Adds a renderer to the registry. Takes ownership of the metadata object.

:param metadata: renderer metadata

:return: ``True`` if renderer was added successfully, or ``False`` if renderer could not
         be added (e.g., a renderer with a duplicate name already exists)
%End

    bool removeRenderer( const QString &rendererName );
%Docstring
Removes a renderer from registry.

:param rendererName: name of renderer to remove from registry

:return: ``True`` if renderer was successfully removed, or ``False`` if matching
         renderer could not be found
%End

    QgsRendererAbstractMetadata *rendererMetadata( const QString &rendererName );
%Docstring
Returns the metadata for a specified renderer. Returns ``None`` if a matching
renderer was not found in the registry.
%End

    QStringList renderersList( QgsRendererAbstractMetadata::LayerTypes layerTypes = QgsRendererAbstractMetadata::All ) const;
%Docstring
Returns a list of available renderers.

:param layerTypes: flags to filter the renderers by compatible layer types
%End

    QStringList renderersList( const QgsVectorLayer *layer ) const;
%Docstring
Returns a list of available renderers which are compatible with a specified layer.

:param layer: vector layer

.. versionadded:: 2.16
%End

  private:
    QgsRendererRegistry( const QgsRendererRegistry &rh );
};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/symbology/qgsrendererregistry.h                             *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
