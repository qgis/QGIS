/***************************************************************************
                             qgsdatacollectionitem.h
                             -------------------
    begin                : 2011-04-01
    copyright            : (C) 2011 Radim Blazek
    email                : radim dot blazek at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSDATACOLLECTIONITEM_H
#define QGSDATACOLLECTIONITEM_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsdataitem.h"

/**
 * \ingroup core
 * \brief A browser item for collections of data.
 *
 * These represent logical collection of layers or subcollections, e.g. GRASS location/mapset, database? wms source?
*/
class CORE_EXPORT QgsDataCollectionItem : public QgsDataItem
{
    Q_OBJECT
  public:

    /**
     * Constructor for QgsDataCollectionItem, with the specified \a parent item.
     *
     * The \a name argument specifies the text to show in the model for the item. A translated string should
     * be used wherever appropriate.
     *
     * The \a path argument gives the item path in the browser tree. The \a path string can take any form,
     * but QgsDataCollectionItem items pointing to different logical locations should always use a different item \a path.
     *
     * The optional \a providerKey string can be used to specify the key for the QgsDataItemProvider that created this item.
     */
    QgsDataCollectionItem( QgsDataItem *parent SIP_TRANSFERTHIS, const QString &name, const QString &path = QString(), const QString &providerKey = QString() );

    ~QgsDataCollectionItem() override;

#ifdef SIP_RUN
    SIP_PYOBJECT __repr__();
    % MethodCode
    QString str = u"<QgsDataCollectionItem: \"%1\" %2>"_s.arg( sipCpp->name(), sipCpp->path() );
    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    % End
#endif

    void addChild( QgsDataItem *item SIP_TRANSFER ) { mChildren.append( item ); }

    /**
     * Returns the standard browser directory icon.
     *
     * Since QGIS 3.20 the optional \a fillColor and \a strokeColor arguments can be used to specify
     * a fill and stroke color for the icon.
     *
     * \see iconDataCollection()
     */
    static QIcon iconDir( const QColor &fillColor = QColor(), const QColor &strokeColor = QColor() );

    /**
     * Returns the standard browser data collection icon.
     * \see iconDir()
     */
    static QIcon iconDataCollection();

    QgsAbstractDatabaseProviderConnection *databaseConnection() const override;

  protected:

    /**
     * Shared open directory icon.
     *
     * Since QGIS 3.20 the optional \a fillColor and \a strokeColor arguments can be used to specify
     * a fill and stroke color for the icon.
     *
     * \since QGIS 3.4
     */
    static QIcon openDirIcon( const QColor &fillColor = QColor(), const QColor &strokeColor = QColor() );

    /**
     * Shared home directory icon.
     *
     * Since QGIS 3.20 the optional \a fillColor and \a strokeColor arguments can be used to specify
     * a fill and stroke color for the icon.
     *
     * \since QGIS 3.4
     */
    static QIcon homeDirIcon( const QColor &fillColor = QColor(), const QColor &strokeColor = QColor() );

};


#endif // QGSDATAITEM_H


