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

#include "qgslayertreegroup.h"

/** \ingroup core
 * @brief The QgsLayerDefinition class holds generic methods for loading/exporting QLR files.
 *
 * QLR files are an export of the layer xml including the style and datasource location.  There is no link
 * to the QLR file once loaded.  Consider the QLR file a mini project file for layers and styles.  QLR
 * files also store the layer tree info for the exported layers, including group information.
 */
class CORE_EXPORT QgsLayerDefinition
{
  public:
    /** Loads the QLR at path into QGIS.  New layers are added to rootGroup and the map layer registry*/
    static bool loadLayerDefinition( const QString & path, QgsLayerTreeGroup* rootGroup, QString &errorMessage );
    /** Loads the QLR from the XML document.  New layers are added to rootGroup and the map layer registry */
    static bool loadLayerDefinition( QDomDocument doc, QgsLayerTreeGroup* rootGroup, QString &errorMessage );
    /** Export the selected layer tree nodes to a QLR file */
    static bool exportLayerDefinition( QString path, const QList<QgsLayerTreeNode*>& selectedTreeNodes, QString &errorMessage );
    /** Export the selected layer tree nodes to a QLR-XML document */
    static bool exportLayerDefinition( QDomDocument doc, const QList<QgsLayerTreeNode*>& selectedTreeNodes, QString &errorMessage, const QString& relativeBasePath = QString::null );

    /**
     * \ingroup core
     * Class used to work with layer dependencies stored in a XML project or layer definition file
     */
    class CORE_EXPORT DependencySorter
    {
      public:
        /** Constructor
         * @param doc The XML document containing maplayer elements
         */
        DependencySorter( const QDomDocument& doc );

        /** Constructor
         * @param fileName The filename where the XML document is stored
         */
        DependencySorter( const QString& fileName );

        /** Get the layer nodes in an order where they can be loaded incrementally without dependency break */
        QVector<QDomNode> sortedLayerNodes() const { return mSortedLayerNodes; }

        /** Get the layer IDs in an order where they can be loaded incrementally without dependency break */
        QStringList sortedLayerIds() const { return mSortedLayerIds; }

        /** Whether some cyclic dependency has been detected */
        bool hasCycle() const { return mHasCycle; }

        /** Whether some dependency is missing */
        bool hasMissingDependency() const { return mHasMissingDependency; }

      private:
        QVector<QDomNode> mSortedLayerNodes;
        QStringList mSortedLayerIds;
        bool mHasCycle;
        bool mHasMissingDependency;
        void init( const QDomDocument& doc );
    };
};

#endif // QGSLAYERDEFINITION_H
