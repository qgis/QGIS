/***************************************************************************
  qgslayertreeutils.h
  --------------------------------------
  Date                 : May 2014
  Copyright            : (C) 2014 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLAYERTREEUTILS_H
#define QGSLAYERTREEUTILS_H

#include <qnamespace.h>
#include <QList>

class QDomElement;
class QDomDocument;
class QStringList;

class QgsLayerTreeGroup;
class QgsLayerTreeLayer;

/**
 * Assorted functions for dealing with layer trees.
 *
 * @note added in 2.4
 */
class CORE_EXPORT QgsLayerTreeUtils
{
  public:

    //! Try to load layer tree from \verbatim <legend> \endverbatim tag from project files from QGIS 2.2 and below
    static bool readOldLegend( QgsLayerTreeGroup* root, const QDomElement& legendElem );
    //! Try to load custom layer order from \verbatim <legend> \endverbatim tag from project files from QGIS 2.2 and below
    static bool readOldLegendLayerOrder( const QDomElement& legendElem, bool& hasCustomOrder, QStringList& order );
    //! Return \verbatim <legend> \endverbatim tag used in QGIS 2.2 and below
    static QDomElement writeOldLegend( QDomDocument& doc, QgsLayerTreeGroup* root, bool hasCustomOrder, const QStringList& order );

    //! Convert Qt::CheckState to QString
    static QString checkStateToXml( Qt::CheckState state );
    //! Convert QString to Qt::CheckState
    static Qt::CheckState checkStateFromXml( QString txt );

    //! Return true if any of the layers is editable
    static bool layersEditable( const QList<QgsLayerTreeLayer*>& layerNodes );
    //! Return true if any of the layers is modified
    static bool layersModified( const QList<QgsLayerTreeLayer*>& layerNodes );

    //! Remove layer nodes that refer to invalid layers
    static void removeInvalidLayers( QgsLayerTreeGroup* group );

    //! Remove subtree of embedded groups. Useful when saving layer tree
    static void removeChildrenOfEmbeddedGroups( QgsLayerTreeGroup* group );

};

#endif // QGSLAYERTREEUTILS_H
