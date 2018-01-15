/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgslayerdefinition.h                                        *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsLayerDefinition
{
%Docstring
 The QgsLayerDefinition class holds generic methods for loading/exporting QLR files.

QLR files are an export of the layer xml including the style and datasource location.  There is no link
to the QLR file once loaded.  Consider the QLR file a mini project file for layers and styles.  QLR
files also store the layer tree info for the exported layers, including group information.
%End

%TypeHeaderCode
#include "qgslayerdefinition.h"
%End
  public:
    static bool loadLayerDefinition( const QString &path, QgsProject *project, QgsLayerTreeGroup *rootGroup, QString &errorMessage /Out/ );
%Docstring
Loads the QLR at path into QGIS.  New layers are added to given project into layer tree specified by rootGroup
%End
    static bool loadLayerDefinition( QDomDocument doc,  QgsProject *project, QgsLayerTreeGroup *rootGroup, QString &errorMessage /Out/, const QgsReadWriteContext &context );
%Docstring
Loads the QLR from the XML document.  New layers are added to given project into layer tree specified by rootGroup
%End
    static bool exportLayerDefinition( QString path, const QList<QgsLayerTreeNode *> &selectedTreeNodes, QString &errorMessage /Out/ );
%Docstring
Export the selected layer tree nodes to a QLR file
%End
    static bool exportLayerDefinition( QDomDocument doc, const QList<QgsLayerTreeNode *> &selectedTreeNodes, QString &errorMessage /Out/, const QgsReadWriteContext &context );
%Docstring
Export the selected layer tree nodes to a QLR-XML document
%End

    static QDomDocument exportLayerDefinitionLayers( const QList<QgsMapLayer *> &layers, const QgsReadWriteContext &context );
%Docstring
Returns the given layer as a layer definition document
Layer definitions store the data source as well as styling and custom properties.

Layer definitions can be used to load a layer and styling all from a single file.

This is a low-level routine that does not write layer tree.

.. seealso:: :py:func:`exportLayerDefinition`
%End

    static QList<QgsMapLayer *> loadLayerDefinitionLayers( QDomDocument &document, const QgsReadWriteContext &context ) /Factory/;
%Docstring
Creates new layers from a layer definition document.
This is a low-level routine that does not resolve layer ID conflicts, dependencies and joins

.. seealso:: :py:func:`loadLayerDefinition`
%End

    static QList<QgsMapLayer *> loadLayerDefinitionLayers( const QString &qlrfile ) /Factory/;
%Docstring
Creates new layers from a layer definition file (.QLR)
This is a low-level routine that does not resolve layer ID conflicts, dependencies and joins

.. seealso:: :py:func:`loadLayerDefinition`
%End

    class DependencySorter
{
%Docstring
Class used to work with layer dependencies stored in a XML project or layer definition file
%End

%TypeHeaderCode
#include "qgslayerdefinition.h"
%End
      public:

        DependencySorter( const QDomDocument &doc );
%Docstring
Constructor

:param doc: The XML document containing maplayer elements
%End

        DependencySorter( const QString &fileName );
%Docstring
Constructor

:param fileName: The filename where the XML document is stored
%End

        QVector<QDomNode> sortedLayerNodes() const;
%Docstring
Get the layer nodes in an order where they can be loaded incrementally without dependency break
%End

        QStringList sortedLayerIds() const;
%Docstring
Get the layer IDs in an order where they can be loaded incrementally without dependency break
%End

        bool hasCycle() const;
%Docstring
Whether some cyclic dependency has been detected
%End

        bool hasMissingDependency() const;
%Docstring
Whether some dependency is missing
%End

    };
};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgslayerdefinition.h                                        *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
