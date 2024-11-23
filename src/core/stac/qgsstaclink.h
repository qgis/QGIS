/***************************************************************************
    qgsstaclink.h
    ---------------------
    begin                : October 2024
    copyright            : (C) 2024 by Stefanos Natsis
    email                : uclaros at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSTACLINK_H
#define QGSSTACLINK_H

#define SIP_NO_FILE

#include "qgis_core.h"

#include <QString>

/**
 * \ingroup core
 * \brief Class for storing data associated with a STAC link
 *
 * \note Not available in python bindings
 * \since QGIS 3.40
 */
class CORE_EXPORT QgsStacLink
{
  public:

    /**
     * Constructor
     * \param href The actual link in the format of an URL.
     * \param relation Relationship between the parent document and the linked document.
     * \param mediaType Media type of the referenced entity
     * \param title A human readable title to be used in rendered displays of the link.
     */
    QgsStacLink( const QString &href, const QString &relation, const QString &mediaType, const QString &title );

    //! Returns the actual link in the format of an URL.
    QString href() const;

    //! Returns the relationship between the parent document and the linked document.
    QString relation() const;

    //! Returns the Media type of the referenced entity
    QString mediaType() const;

    //! Returns a human readable title to be used in rendered displays of the link.
    QString title() const;

  private:
    QString mHref;
    QString mRelation;
    QString mMediaType;
    QString mTitle;
};

#endif // QGSSTACLINK_H
