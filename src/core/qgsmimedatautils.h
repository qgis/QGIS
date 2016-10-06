/***************************************************************************
    qgsmimedatautils.h
    ---------------------
    begin                : November 2011
    copyright            : (C) 2011 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSMIMEDATAUTILS_H
#define QGSMIMEDATAUTILS_H

#include <QMimeData>
#include <QStringList>

class QgsLayerItem;
class QgsLayerTreeNode;

/** \ingroup core
 * \class QgsMimeDataUtils
 */
class CORE_EXPORT QgsMimeDataUtils
{
  public:

    struct CORE_EXPORT Uri
    {
      //! Constructs invalid URI
      Uri();
      //! Constructs URI from encoded data
      explicit Uri( QString& encData );

      //! Returns whether the object contains valid data
      //! @note added in 3.0
      bool isValid() const { return !layerType.isEmpty(); }

      //! Returns encoded representation of the object
      QString data() const;

      //! Type of URI. Recognized types: "vector" / "raster" / "plugin" / "custom"
      QString layerType;
      //! For "vector" / "raster" type: provider id.
      //! For "plugin" type: plugin layer type name.
      //! For "custom" type: key of its QgsCustomDropHandler
      QString providerKey;
      //! Human readable name to be used e.g. in layer tree
      QString name;
      //! Identifier of the data source recognized by its providerKey
      QString uri;
      QStringList supportedCrs;
      QStringList supportedFormats;
    };
    typedef QList<Uri> UriList;

    static QMimeData* encodeUriList( const UriList& layers );

    static bool isUriList( const QMimeData* data );

    static UriList decodeUriList( const QMimeData* data );

    /**
     * Returns encoded URI list from a list of layer tree nodes.
     * @note added in QGIS 3.0
     */
    static QByteArray layerTreeNodesToUriList( const QList<QgsLayerTreeNode*>& nodes );

  private:
    static QString encode( const QStringList& items );
    static QStringList decode( const QString& encoded );
    static QByteArray uriListToByteArray( const UriList& layers );

};

#endif // QGSMIMEDATAUTILS_H

