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
#include <QObject>
#include <QDomElement>

class QgsProject;
class QgsAnnotation;

class CORE_EXPORT QgsAnnotationManager : public QObject
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsAnnotationManager. The project will become the parent object for this
     * manager.
     */
    explicit QgsAnnotationManager( QgsProject* project = nullptr );

    ~QgsAnnotationManager();

    /**
     * Adds an annotation to the manager. Ownership of the annotation is transferred to the manager.
     * Returns true if the addition was successful, or false if the annotation could not be added.
     * @see removeAnnotation()
     * @see annotationAdded()
     */
    bool addAnnotation( QgsAnnotation* annotation );

    /**
     * Removes an annotation from the manager. The annotation is deleted.
     * Returns true if the removal was successful, or false if the removal failed (eg as a result
     * of removing an annotation which is not contained in the manager).
     * @see addAnnotation()
     * @see compositionRemoved()
     * @see compositionAboutToBeRemoved()
     * @see clear()
     */
    bool removeAnnotation( QgsAnnotation* annotation );

    /**
     * Removes and deletes all annotations from the manager.
     * @see removeAnnotation()
     */
    void clear();

    /**
     * Returns a list of all annotations contained in the manager.
     */
    QList< QgsAnnotation* > annotations() const;

    bool readXml( const QDomElement& element, const QDomDocument& doc );

  signals:

    //! Emitted when a annotation has been added to the manager
    void annotationAdded( QgsAnnotation* annotation );

    //! Emitted when an annotation was removed from the manager
    void annotationRemoved();

    //! Emitted when an annotation is about to be removed from the manager
    void annotationAboutToBeRemoved( QgsAnnotation* annotation );

  private:

    QgsProject* mProject = nullptr;

    QList< QgsAnnotation* > mAnnotations;

    QgsAnnotation* createAnnotationFromXml( const QDomElement& element, const QDomDocument& doc ) const;

};

#endif // QGSANNOTATIONMANAGER_H
