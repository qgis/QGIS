/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgspluginlayerregistry.h                                    *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsPluginLayerType
{
%Docstring
class for creating plugin specific layers
%End

%TypeHeaderCode
#include "qgspluginlayerregistry.h"
%End
  public:

    QgsPluginLayerType( const QString &name );
    virtual ~QgsPluginLayerType();

    QString name();
%Docstring
 :rtype: str
%End

    virtual QgsPluginLayer *createLayer() /Factory/;
%Docstring
Return new layer of this type. Return NULL on error
 :rtype: QgsPluginLayer
%End

    virtual QgsPluginLayer *createLayer( const QString &uri ) /Factory/;
%Docstring
 Return new layer of this type, using layer URI (specific to this plugin layer type). Return NULL on error.
.. versionadded:: 2.10
 :rtype: QgsPluginLayer
%End

    virtual bool showLayerProperties( QgsPluginLayer *layer );
%Docstring
Show plugin layer properties dialog. Return false if the dialog cannot be shown.
 :rtype: bool
%End

  protected:
};


class QgsPluginLayerRegistry
{
%Docstring
 A registry of plugin layers types.

 QgsPluginLayerRegistry is not usually directly created, but rather accessed through
 QgsApplication.pluginLayerRegistry().
%End

%TypeHeaderCode
#include "qgspluginlayerregistry.h"
%End
  public:

    QgsPluginLayerRegistry();
%Docstring
 Constructor for QgsPluginLayerRegistry.
%End
    ~QgsPluginLayerRegistry();


    QStringList pluginLayerTypes();
%Docstring
 List all known layer types
  \since QGIS *
 :rtype: list of str
%End

    bool addPluginLayerType( QgsPluginLayerType *pluginLayerType /Transfer/ );
%Docstring
Add plugin layer type (take ownership) and return true on success
 :rtype: bool
%End

    bool removePluginLayerType( const QString &typeName );
%Docstring
Remove plugin layer type and return true on success
 :rtype: bool
%End

    QgsPluginLayerType *pluginLayerType( const QString &typeName );
%Docstring
Return plugin layer type metadata or NULL if doesn't exist
 :rtype: QgsPluginLayerType
%End

    QgsPluginLayer *createLayer( const QString &typeName, const QString &uri = QString() ) /Factory/;
%Docstring
 Returns new layer if corresponding plugin has been found else returns a None.
.. note::

   parameter uri has been added in QGIS 2.10
 :rtype: QgsPluginLayer
%End

  private:
    QgsPluginLayerRegistry( const QgsPluginLayerRegistry &rh );
};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgspluginlayerregistry.h                                    *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
