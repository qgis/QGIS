/***************************************************************************
    qgsstacprovider.h
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

#ifndef QGSSTACPROVIDER_H
#define QGSSTACPROVIDER_H

#define SIP_NO_FILE

#include <QString>
#include <QStringList>

/**
 * \brief Class for storing a STAC collection's provider data
 * The object provides information about a provider. A provider is any of the organizations that captures or
 * processes the content of the Collection and therefore influences the data offered by this Collection. May
 * also include information about the final storage provider hosting the data.
 * \note Not available in python bindings
 * \since QGIS 3.40
 */
class QgsStacProvider
{
  public:

    /**
     * QgsStacProvider constructor
     * \param name The name of the organization or the individual.
     * \param description Multi-line description to add further provider information such as processing details
     *  for processors and producers, hosting details for hosts or basic contact information.
     *  CommonMark 0.29 syntax MAY be used for rich text representation.
     * \param roles Roles of the provider. Any of licensor, producer, processor or host.
     * \param url Homepage on which the provider describes the dataset and publishes contact information.
     */
    QgsStacProvider( const QString &name, const QString &description, const QStringList &roles, const QString &url );

    //! Returns the name of the organization or the individual.
    QString name() const;

    /**
     * Returns a multi-line description to add further provider information such as processing details
     * for processors and producers, hosting details for hosts or basic contact information.
     * CommonMark 0.29 syntax MAY be used for rich text representation.
     */
    QString description() const;

    //! Returns the roles of the provider. Any of licensor, producer, processor or host.
    QStringList roles() const;

    //! Returns the url of a homepage on which the provider describes the dataset and publishes contact information.
    QString url() const;

  private:
    QString mName;
    QString mDescription;
    QStringList mRoles;
    QString mUrl;
};

#endif // QGSSTACPROVIDER_H
