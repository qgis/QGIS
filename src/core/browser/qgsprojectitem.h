/***************************************************************************
                             qgsprojectitem.h
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
#ifndef QGSPROJECTITEM_H
#define QGSPROJECTITEM_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsdataitem.h"

/**
 * \ingroup core
 * \brief Data item that can be used to represent QGIS projects.
 */
class CORE_EXPORT QgsProjectItem : public QgsDataItem
{
    Q_OBJECT
  public:

    /**
     * \brief A data item holding a reference to a QGIS project file.
     * \param parent The parent data item.
     * \param name The name of the of the project. Displayed to the user.
     * \param path The full path to the project.
     * \param providerKey key of the provider that created this item
     */
    QgsProjectItem( QgsDataItem *parent, const QString &name, const QString &path, const QString &providerKey = QString() );

#ifdef SIP_RUN
    SIP_PYOBJECT __repr__();
    % MethodCode
    QString str = u"<QgsProjectItem: \"%1\" %2>"_s.arg( sipCpp->name(), sipCpp->path() );
    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    % End
#endif

    bool hasDragEnabled() const override { return true; }

    QgsMimeDataUtils::UriList mimeUris() const override;

};

#endif // QGSPROJECTITEM_H


