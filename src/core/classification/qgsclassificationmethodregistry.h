/***************************************************************************
    qgsclassificationmethodregistry.h
    ---------------------
    begin                : September 2019
    copyright            : (C) 2019 by Denis Rouzaud
    email                : denis@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/


#ifndef QGSCLASSIFICATIONMETHODREGISTRY_H
#define QGSCLASSIFICATIONMETHODREGISTRY_H

#include <QMap>

#include "qgis_core.h"
#include "qgis_sip.h"

class QIcon;

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
    ~QgsClassificationMethodRegistry();

    /**
     * Adds a method to the registry
     * Returns FALSE if a method with same id already exists.
     */
    bool addMethod( QgsClassificationMethod *method SIP_TRANSFER );

    //! Returns a new instance of the method for the given id
    QgsClassificationMethod *method( const QString &id ) SIP_FACTORY;

    //! Returns a map <name, id> of all registered methods.
    QMap<QString, QString> methodNames() const;

    //! Returns the icon for a given method id
    QIcon icon( const QString &id ) const;

  private:

    QMap<QString, QgsClassificationMethod *> mMethods;
};

#endif // QGSCLASSIFICATIONMETHODREGISTRY_H
