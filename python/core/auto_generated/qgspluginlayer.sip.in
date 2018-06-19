/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgspluginlayer.h                                            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/


class QgsPluginLayer : QgsMapLayer
{
%Docstring
Base class for plugin layers. These can be implemented by plugins
and registered in QgsPluginLayerRegistry.

In order to be readable from project files, they should set these attributes in layer DOM node:
"type" = "plugin"
"name" = "your_layer_type"
%End

%TypeHeaderCode
#include "qgspluginlayer.h"
%End
  public:
    QgsPluginLayer( const QString &layerType, const QString &layerName = QString() );
    ~QgsPluginLayer();

    virtual QgsPluginLayer *clone() const = 0;

%Docstring
Returns a new instance equivalent to this one.

:return: a new layer instance

.. versionadded:: 3.0
%End

    QString pluginLayerType();
%Docstring
Returns plugin layer type (the same as used in QgsPluginLayerRegistry)
%End

    virtual void setExtent( const QgsRectangle &extent );

%Docstring
Sets extent of the layer
%End

    void setSource( const QString &source );
%Docstring
Set source string. This is used for example in layer tree to show tooltip.

.. versionadded:: 2.16
%End

    virtual QgsDataProvider *dataProvider();


  protected:
};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgspluginlayer.h                                            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
