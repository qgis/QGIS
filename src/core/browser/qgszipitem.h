/***************************************************************************
                             qgszipitem.h
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
#ifndef QGSZIPITEM_H
#define QGSZIPITEM_H

#include "qgis_sip.h"
#include "qgis_core.h"
#include "qgis.h"
#include "qgsdatacollectionitem.h"

/**
 * \ingroup core
 * \brief A zip file: contains layers, using GDAL/OGR VSIFILE mechanism
*/
class CORE_EXPORT QgsZipItem : public QgsDataCollectionItem
{
    Q_OBJECT

  protected:
    QString mFilePath;
    QString mVsiPrefix;
    QStringList mZipFileList;

  public:
    //! Constructor
    QgsZipItem( QgsDataItem *parent, const QString &name, const QString &path );

    //! Constructor
    QgsZipItem( QgsDataItem *parent, const QString &name, const QString &filePath, const QString &path, const QString &providerKey = QString() );

#ifdef SIP_RUN
    SIP_PYOBJECT __repr__();
    % MethodCode
    QString str = QStringLiteral( "<QgsZipItem: \"%1\" %2>" ).arg( sipCpp->name(), sipCpp->path() );
    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    % End
#endif

    QVector<QgsDataItem *> createChildren() override;
    QStringList getZipFileList();

    bool hasDragEnabled() const override;
    QgsMimeDataUtils::UriList mimeUris() const override;

    //! \note not available via Python bindings
    static QVector<dataItem_t *> sDataItemPtr SIP_SKIP;
    static QStringList sProviderNames;

    static QString vsiPrefix( const QString &uri ) { return qgsVsiPrefix( uri ); }

    /**
     * Creates a new data item from the specified path.
     */
    static QgsDataItem *itemFromPath( QgsDataItem *parent, const QString &path, const QString &name ) SIP_FACTORY;

    /**
    * Creates a new data item from the specified path.
    * \note available in Python as itemFromFilePath
    */
    static QgsDataItem *itemFromPath( QgsDataItem *parent, const QString &filePath, const QString &name, const QString &path ) SIP_FACTORY SIP_PYNAME( itemFromFilePath );

    static QIcon iconZip();

  private:
    void init();
};



#endif // QGSZIPITEM_H


