/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/layertree/qgslayertreeembeddedwidgetregistry.h               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsLayerTreeEmbeddedWidgetProvider
{
%Docstring
 Provider interface to be implemented in order to introduce new kinds of embedded widgets for use in layer tree.
 Embedded widgets are assigned per individual map layers and they are shown before any legend entries.
.. seealso:: QgsLayerTreeEmbeddedWidgetRegistry
.. versionadded:: 2.16
%End

%TypeHeaderCode
#include "qgslayertreeembeddedwidgetregistry.h"
%End
  public:
    virtual ~QgsLayerTreeEmbeddedWidgetProvider();

    virtual QString id() const = 0;
%Docstring
Unique name of the provider (among other providers)
 :rtype: str
%End

    virtual QString name() const = 0;
%Docstring
Human readable name - may be translatable with tr()
 :rtype: str
%End

    virtual QWidget *createWidget( QgsMapLayer *layer, int widgetIndex ) = 0 /Factory/;
%Docstring
 Factory to create widgets. The returned widget is owned by the caller.
 The widgetIndex argument may be used to identify which widget is being
 created (useful when using multiple widgets from the same provider for one layer).
 :rtype: QWidget
%End

    virtual bool supportsLayer( QgsMapLayer *layer ) = 0;
%Docstring
Whether it makes sense to use this widget for a particular layer
 :rtype: bool
%End
};

class QgsLayerTreeEmbeddedWidgetRegistry
{
%Docstring
 Registry of widgets that may be embedded into layer tree view.
 Embedded widgets are assigned per individual map layers and they are shown before any legend entries.
 Layer tree must have UseEmbeddedWidgets flag enabled in order to show assigned widgets.

 QgsLayerTreeEmbeddedWidgetRegistry is not usually directly created, but rather accessed through
 QgsGui.layerTreeEmbeddedWidgetRegistry().

.. seealso:: QgsLayerTreeEmbeddedWidgetRegistry
.. versionadded:: 2.16
%End

%TypeHeaderCode
#include "qgslayertreeembeddedwidgetregistry.h"
%End
  public:

    QgsLayerTreeEmbeddedWidgetRegistry();
%Docstring
 Constructor for QgsLayerTreeEmbeddedWidgetRegistry

 QgsLayerTreeEmbeddedWidgetRegistry is not usually directly created, but rather accessed through
 QgsGui.layerTreeEmbeddedWidgetRegistry().
%End

    ~QgsLayerTreeEmbeddedWidgetRegistry();


    QStringList providers() const;
%Docstring
Return list of all registered providers
 :rtype: list of str
%End

    QgsLayerTreeEmbeddedWidgetProvider *provider( const QString &providerId ) const;
%Docstring
Get provider object from the provider's ID
 :rtype: QgsLayerTreeEmbeddedWidgetProvider
%End

    bool addProvider( QgsLayerTreeEmbeddedWidgetProvider *provider /Transfer/ );
%Docstring
 Register a provider, takes ownership of the object.
 Returns true on success, false if the provider is already registered. *
 :rtype: bool
%End

    bool removeProvider( const QString &providerId );
%Docstring
 Unregister a provider, the provider object is deleted.
 Returns true on success, false if the provider was not registered. *
 :rtype: bool
%End

  protected:


  private:
    QgsLayerTreeEmbeddedWidgetRegistry( const QgsLayerTreeEmbeddedWidgetRegistry &other );
};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/layertree/qgslayertreeembeddedwidgetregistry.h               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
