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

    //! Try to load layer tree from <legend> tag from project files from QGIS 2.2 and below
    static bool readOldLegend( QgsLayerTreeGroup* root, const QDomElement& legendElem );
    //! Try to load custom layer order from <legend> tag from project files from QGIS 2.2 and below
    static bool readOldLegendLayerOrder( const QDomElement& legendElem, bool& hasCustomOrder, QStringList& order );

    static QString checkStateToXml( Qt::CheckState state );
    static Qt::CheckState checkStateFromXml( QString txt );

    static bool layersEditable( const QList<QgsLayerTreeLayer*>& layerNodes );
    static bool layersModified( const QList<QgsLayerTreeLayer*>& layerNodes );


  protected:
    static void addLegendGroupToTreeWidget( const QDomElement& groupElem, QgsLayerTreeGroup* parent );
    static void addLegendLayerToTreeWidget( const QDomElement& layerElem, QgsLayerTreeGroup* parent );

};

#endif // QGSLAYERTREEUTILS_H
