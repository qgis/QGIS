/***************************************************************************
    qgslayerdefinition.h
    ---------------------
    begin                : January 2015
    copyright            : (C) 2015 by Nathan Woodrow
    email                : woodrow dot nathan at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSLAYERDEFINITION_H
#define QGSLAYERDEFINITION_H


#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgis.h"
#include "qgslayertreeregistrybridge.h"

#include <QString>
#include <QVector>
#include <QDomNode>
#include <QSet>

class QDomDocument;

class QgsLayerTreeGroup;
class QgsLayerTreeNode;
class QgsMapLayer;
class QgsReadWriteContext;
class QgsProject;

/**
 * \ingroup core
 * \brief The QgsLayerDefinition class holds generic methods for loading/exporting QLR files.
 *
 * QLR files are an export of the layer xml including the style and datasource location.  There is no link
 * to the QLR file once loaded.  Consider the QLR file a mini project file for layers and styles.  QLR
 * files also store the layer tree info for the exported layers, including group information.
 */
class CORE_EXPORT QgsLayerDefinition
{
  public:

    /**
     * Loads the QLR at path into QGIS.  New layers are added to given project into layer tree specified by rootGroup
     * \param path file path to the qlr
     * \param project the current project
     * \param rootGroup the layer tree group to insert the qlr content
     * \param errorMessage the returned error message
     * \param insertMethod method for layer tree (since QGIS 3.38)
     * \param insertPoint describes where in rootGroup the qlr layers/groups shall be inserted (since QGIS 3.38)
     * \return true in case of success
    */
    static bool loadLayerDefinition( const QString &path, QgsProject *project, QgsLayerTreeGroup *rootGroup, QString &errorMessage SIP_OUT, Qgis::LayerTreeInsertionMethod insertMethod = Qgis::LayerTreeInsertionMethod::OptimalInInsertionGroup, const QgsLayerTreeRegistryBridge::InsertionPoint *insertPoint = nullptr );

    /**
     * Loads the QLR from the XML document.  New layers are added to given project into layer tree specified by rootGroup
     *  \param doc the xml document
     *  \param project the current project
     *  \param rootGroup the layer tree group to insert the qlr content
     *  \param errorMessage the returned error message
     *  \param context the read write context
     *  \param insertMethod method for layer tree (since QGIS 3.38)
     *  \param insertPoint describes where in rootGroup the qlr layers/groups shall be inserted (since QGIS 3.38)
     *  \return true in case of success
     */
    static bool loadLayerDefinition( QDomDocument doc,  QgsProject *project, QgsLayerTreeGroup *rootGroup, QString &errorMessage SIP_OUT, QgsReadWriteContext &context, Qgis::LayerTreeInsertionMethod insertMethod = Qgis::LayerTreeInsertionMethod::OptimalInInsertionGroup, const QgsLayerTreeRegistryBridge::InsertionPoint *insertPoint = nullptr );

    /**
     * Exports the selected layer tree nodes to a QLR file.
     *
     * This method uses the QgsProject.instance()'s file path setting to determine whether absolute
     * or relative paths are written. Use the variant with an explicit argument for file path type
     * for control over this setting.
     *
     * \param path file path for exported QLR file
     * \param selectedTreeNodes layer tree nodes to include in the QLR file
     * \param errorMessage will be set to any error messages generated during the export
     *
     * \returns TRUE if the export was successful
     */
    static bool exportLayerDefinition( const QString &path, const QList<QgsLayerTreeNode *> &selectedTreeNodes, QString &errorMessage SIP_OUT );

    /**
     * Exports the selected layer tree nodes to a QLR file.
     *
     * \param path file path for exported QLR file
     * \param selectedTreeNodes layer tree nodes to include in the QLR file
     * \param pathType specifies whether absolute or relative file paths should be used.
     * \param errorMessage will be set to any error messages generated during the export
     *
     * \returns TRUE if the export was successful
     *
     * \since QGIS 3.22
     */
    static bool exportLayerDefinition( const QString &path, const QList<QgsLayerTreeNode *> &selectedTreeNodes, Qgis::FilePathType pathType, QString &errorMessage SIP_OUT );

    /**
     * Exports the selected layer tree nodes to a QLR XML document.
     */
    static bool exportLayerDefinition( QDomDocument doc, const QList<QgsLayerTreeNode *> &selectedTreeNodes, QString &errorMessage SIP_OUT, const QgsReadWriteContext &context );

    /**
     * Returns the given layer as a layer definition document
     *  Layer definitions store the data source as well as styling and custom properties.
     *
     *  Layer definitions can be used to load a layer and styling all from a single file.
     *
     *  This is a low-level routine that does not write layer tree.
     *  \see exportLayerDefinition()
     */
    static QDomDocument exportLayerDefinitionLayers( const QList<QgsMapLayer *> &layers, const QgsReadWriteContext &context );

    /**
     * Creates new layers from a layer definition document.
     * This is a low-level routine that does not resolve layer ID conflicts, dependencies and joins
     * \see loadLayerDefinition()
     */
    static QList<QgsMapLayer *> loadLayerDefinitionLayers( QDomDocument &document, QgsReadWriteContext &context ) SIP_FACTORY;

    /**
     * Creates new layers from a layer definition file (.QLR)
     * This is a low-level routine that does not resolve layer ID conflicts, dependencies and joins
     * \see loadLayerDefinition()
     */
    static QList<QgsMapLayer *> loadLayerDefinitionLayers( const QString &qlrfile ) SIP_FACTORY;

    /**
     * \ingroup core
     * \brief Class used to work with layer dependencies stored in a XML project or layer definition file
     */
    class CORE_EXPORT DependencySorter
    {
      public:

        /**
         * Constructor
         * \param doc The XML document containing maplayer elements
         */
        explicit DependencySorter( const QDomDocument &doc );

        /**
         * Constructor
         * \param fileName The filename where the XML document is stored
         */
        explicit DependencySorter( const QString &fileName );

        //! Gets the layer nodes in an order where they can be loaded incrementally without dependency break
        QVector<QDomNode> sortedLayerNodes() const { return mSortedLayerNodes; }

        //! Gets the layer IDs in an order where they can be loaded incrementally without dependency break
        QStringList sortedLayerIds() const { return mSortedLayerIds; }

        //! Whether some cyclic dependency has been detected
        bool hasCycle() const { return mHasCycle; }

        //! Whether some dependency is missing
        bool hasMissingDependency() const { return mHasMissingDependency; }

        /**
         * Returns whether the layer associated with the\a layerId is dependent from another layer
         *
         * \since QGIS 3.32
         */
        bool isLayerDependent( const QString &layerId ) const;

      private:
        QVector<QDomNode> mSortedLayerNodes;
        QStringList mSortedLayerIds;
        QSet<QString> mDependentLayerIds;
        bool mHasCycle;
        bool mHasMissingDependency;
        void init( const QDomDocument &doc );
    };

  private:

    static QList<QgsMapLayer *> loadLayerDefinitionLayersInternal( QDomDocument &document, QgsReadWriteContext &context, QString &errorMessage );


};

#endif // QGSLAYERDEFINITION_H
