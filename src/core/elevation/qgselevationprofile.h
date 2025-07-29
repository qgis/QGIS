/***************************************************************************
    qgselevationprofile.h
    ------------------
    Date                 : July 2025
    Copyright            : (C) 2025 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSELEVATIONPROFILE_H
#define QGSELEVATIONPROFILE_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include <QObject>
#include <QPointer>

class QgsProject;
class QgsReadWriteContext;
class QDomDocument;
class QDomElement;

/**
 * \ingroup core
 * \class QgsElevationProfile
 *
 * \brief Represents an elevation profile attached to a project.
 *
 * \since QGIS 4.0
 */
class CORE_EXPORT QgsElevationProfile : public QObject
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsElevationProfile.
     */
    explicit QgsElevationProfile( QgsProject *project );

    /**
     * Returns the profile's unique name.
     *
     * \see setName()
     * \see nameChanged()
     */
    QString name() const { return mName; }

    /**
     * Returns the profiles's state encapsulated in a DOM element.
     * \see readXml()
     */
    QDomElement writeXml( QDomDocument &document, const QgsReadWriteContext &context ) const;

    /**
     * Sets the profiles's state from a DOM element.
     *
     * \a element is the DOM node corresponding to the profile.
     *
     * \see writeXml()
     */
    bool readXml( const QDomElement &element, const QDomDocument &document, const QgsReadWriteContext &context );

  public slots:

    /**
     * Sets the profile's unique \a name.
     *
     * \see name()
     * \see nameChanged()
     */
    void setName( const QString &name );

  signals:

    /**
     * Emitted when the profile is renamed.
     *
     * \see name()
     * \see setName()
     */
    void nameChanged( const QString &newName );

  private:

    QPointer< QgsProject > mProject;
    QString mName;

};

#endif // QGSELEVATIONPROFILE_H
