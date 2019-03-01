/***************************************************************************
    qgsannotationmanager.h
    ----------------------
    Date                 : January 2017
    Copyright            : (C) 2017 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSANNOTATIONMANAGER_H
#define QGSANNOTATIONMANAGER_H

#include "qgis_core.h"
#include "qgis.h"
#include <QObject>
#include <QDomElement>

class QgsReadWriteContext;
class QgsProject;
class QgsAnnotation;


/**
 * \ingroup core
 * \class QgsAnnotationManager
 *
 * \brief Manages storage of a set of QgsAnnotation annotation objects.
 *
 * QgsAnnotationManager handles the storage, serializing and deserializing
 * of QgsAnnotations. Usually this class is not constructed directly, but
 * rather accessed through a QgsProject via QgsProject::annotationManager().
 *
 * QgsAnnotationManager retains ownership of all the annotations contained
 * in the manager.
 *
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsAnnotationManager : public QObject
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsAnnotationManager. The project will become the parent object for this
     * manager.
     */
    explicit QgsAnnotationManager( QgsProject *project SIP_TRANSFERTHIS = nullptr );

    ~QgsAnnotationManager() override;

    /**
     * Adds an annotation to the manager. Ownership of the annotation is transferred to the manager.
     * Returns TRUE if the addition was successful, or FALSE if the annotation could not be added.
     * \see removeAnnotation()
     * \see annotationAdded()
     */
    bool addAnnotation( QgsAnnotation *annotation SIP_TRANSFER );

    /**
     * Removes an annotation from the manager. The annotation is deleted.
     * Returns TRUE if the removal was successful, or FALSE if the removal failed (eg as a result
     * of removing an annotation which is not contained in the manager).
     * \see addAnnotation()
     * \see annotationRemoved()
     * \see annotationAboutToBeRemoved()
     * \see clear()
     */
    bool removeAnnotation( QgsAnnotation *annotation );

    /**
     * Removes and deletes all annotations from the manager.
     * \see removeAnnotation()
     */
    void clear();

    /**
     * Returns a list of all annotations contained in the manager.
     * \see cloneAnnotations()
     */
    QList< QgsAnnotation * > annotations() const;

    /**
     * Returns a list containing clones of all annotations contained
     * in the manager. The caller takes responsibility for deleting
     * all returned annotations.
     * \see annotations()
     */
    QList< QgsAnnotation * > cloneAnnotations() const SIP_FACTORY;

    /**
     * Reads the manager's state from a DOM element, restoring all annotations
     * present in the XML document.
     * \see writeXml()
     */
    bool readXml( const QDomElement &element, const QgsReadWriteContext &context );

    /**
     * Returns a DOM element representing the state of the manager.
     * \see readXml()
     */
    QDomElement writeXml( QDomDocument &doc, const QgsReadWriteContext &context ) const;

  signals:

    //! Emitted when a annotation has been added to the manager
    void annotationAdded( QgsAnnotation *annotation );

    //! Emitted when an annotation was removed from the manager
    void annotationRemoved();

    //! Emitted when an annotation is about to be removed from the manager
    void annotationAboutToBeRemoved( QgsAnnotation *annotation );

  private:

    QgsProject *mProject = nullptr;

    QList< QgsAnnotation * > mAnnotations;

    void createAnnotationFromXml( const QDomElement &element, const QgsReadWriteContext &context );

};

#endif // QGSANNOTATIONMANAGER_H
