/***************************************************************************
    qgsstacobject.h
    ---------------------
    begin                : August 2024
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

#ifndef QGSSTACOBJECT_H
#define QGSSTACOBJECT_H

#include "qgis.h"
#include "qgis_core.h"
#include "qgsstaclink.h"

#include <QString>
#include <QStringList>
#include <QObject>

/**
 * \ingroup core
 * \brief Abstract base class for storing STAC objects.
 *
 * \since QGIS 3.44
 */
class CORE_EXPORT QgsStacObject
{
    //SIP_TYPEHEADER_INCLUDE( "qgsstacitem.h" );
    //SIP_TYPEHEADER_INCLUDE( "qgsstaccollection.h" );
    //SIP_TYPEHEADER_INCLUDE( "qgsstaccatalog.h" );

#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE
    if ( QgsStacItem *item = dynamic_cast< QgsStacItem * >( sipCpp ) )
    {
      sipType = sipType_QgsStacItem;
    }
    else if ( QgsStacCollection *item = dynamic_cast< QgsStacCollection * >( sipCpp ) )
    {
      sipType = sipType_QgsStacCollection;
    }
    else if ( QgsStacCatalog *item = dynamic_cast< QgsStacCatalog * >( sipCpp ) )
    {
      sipType = sipType_QgsStacCatalog;
    }
    else
    {
      sipType = NULL;
    }
    SIP_END
#endif

  public:

    //! Default constructor is used for creating invalid objects
    QgsStacObject() = delete;

    //! Constructor for valid objects
    QgsStacObject( const QString &id, const QString &version, const QVector< QgsStacLink > &links );

    //! Destructor
    virtual ~QgsStacObject() = default;

    //! Returns the \a Type of the STAC object
    virtual Qgis::StacObjectType type() const = 0;

    //! Returns an HTML representation of the STAC object
    virtual QString toHtml() const = 0;

    //! Returns the STAC version the object implements
    QString stacVersion() const;

    //! Sets the STAC version the object implements
    void setStacVersion( const QString &stacVersion );

    //! Returns the list of extensions the STAC object implements
    QStringList stacExtensions() const;

    //! Sets the list of \a stacExtensions the object implements
    void setStacExtensions( const QStringList &stacExtensions );

    //! Id of the STAC object
    QString id() const;

    //! Sets the \a id for the STAC object
    void setId( const QString &id );

    //! Returns the STAC links included in the object
    QVector< QgsStacLink > links() const;

    //! Sets the STAC links included in the object
    void setLinks( const QVector< QgsStacLink > &links );

    //! Return the url stored in the object's "self" link
    QString url() const;

    //! Return the url stored in the object's "root" link
    QString rootUrl() const;

    //! Return the url stored in the object's "parent" link
    QString parentUrl() const;


  protected:
    Qgis::StacObjectType mType = Qgis::StacObjectType::Unknown;
    QString mId;
    QString mStacVersion;
    QStringList mStacExtensions;
    QVector< QgsStacLink > mLinks;
};

// we'll store STAC object pointers in QVariants in item models
Q_DECLARE_METATYPE( QgsStacObject * )

#endif // QGSSTACOBJECT_H
