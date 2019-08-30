/***************************************************************************
    qgsclassificationmethodregistry.h
    ---------------------
    begin                : September 2019
    copyright            : (C) 2019 by Denis Rouzaud
    email                : denis@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef QGSCLASSIFICATIONMETHODREGISTRY_H
#define QGSCLASSIFICATIONMETHODREGISTRY_H

#include <QMap>

#include "qgis_core.h"
#include "qgis_sip.h"

class QgsClassificationMethod;

/**
 * \ingroup gui
 * This class manages all known classification methods
 *
 * QgsClassificationMethodRegistry is not usually directly created, but rather accessed through
 * QgsApplication::classificationMethodRegistry().
 *
 * \since QGIS 3.10
 */
class CORE_EXPORT QgsClassificationMethodRegistry
{
  public:
    QgsClassificationMethodRegistry();

    //! Adds a method to the registry
    void addMethod( QgsClassificationMethod *method SIP_TRANSFER );

    //! Return a new instance of the method for the given id
    QgsClassificationMethod *method( const QString &id );

    //! Returns a map <id, name> of all registered methods
    QMap<QString, QString> methodNames() const;

  private:

    QMap<QString, QgsClassificationMethod *> mMethods;
};

#endif // QGSCLASSIFICATIONMETHODREGISTRY_H
