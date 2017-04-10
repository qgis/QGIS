/***************************************************************************
                             qgslayermetadata.h
                             -------------------
    begin                : April 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLAYERMETADATA_H
#define QGSLAYERMETADATA_H

#include "qgis.h"
#include "qgis_core.h"

class CORE_EXPORT QgsLayerMetadata
{
  public:

    QgsLayerMetadata() = default;

    virtual ~QgsLayerMetadata() = default;


    QString identifier() const;
    void setIdentifier( const QString &identifier );

    /**
     * Returns an empty string if no parent identifier is set.
     */
    QString parentIdentifier() const;


    void setParentIdentifier( const QString &parentIdentifier );

    QString type() const;
    void setType( const QString &type );

    QString title() const;
    void setTitle( const QString &title );

    QString abstract() const;
    void setAbstract( const QString &abstract );

    /**
     * Returns an empty string if no fees are set.
     */
    QString fees() const;
    void setFees( const QString &fees );

    QStringList constraints() const;
    void setConstraints( const QStringList &constraints );

    QStringList rights() const;
    void setRights( const QStringList &rights );

    /**
     * Returns an empty string if no encoding is set.
     */
    QString encoding() const;
    void setEncoding( const QString &encoding );

  private:

    QString mIdentifier;
    QString mParentIdentifier;
    QString mType;
    QString mTitle;
    QString mAbstract;
    QString mFees;
    QStringList mConstraints;
    QStringList mRights;
    QString mEncoding;



};

#endif // QGSLAYERMETADATA_H
