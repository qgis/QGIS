/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/annotations/qgsannotationmanager.h                          *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsAnnotationManager : QObject
{
%Docstring
 \since QGIS 3.0

 Manages storage of a set of QgsAnnotation annotation objects.

 QgsAnnotationManager handles the storage, serializing and deserializing
 of QgsAnnotations. Usually this class is not constructed directly, but
 rather accessed through a QgsProject via QgsProject::annotationManager().

 QgsAnnotationManager retains ownership of all the annotations contained
 in the manager.
%End

%TypeHeaderCode
#include "qgsannotationmanager.h"
%End

  public:

    explicit QgsAnnotationManager( QgsProject *project /TransferThis/ = 0 );
%Docstring
 Constructor for QgsAnnotationManager. The project will become the parent object for this
 manager.
%End

    ~QgsAnnotationManager();

    bool addAnnotation( QgsAnnotation *annotation /Transfer/ );
%Docstring
 Adds an annotation to the manager. Ownership of the annotation is transferred to the manager.
 Returns true if the addition was successful, or false if the annotation could not be added.
 @see removeAnnotation()
 @see annotationAdded()
%End

    bool removeAnnotation( QgsAnnotation *annotation );
%Docstring
 Removes an annotation from the manager. The annotation is deleted.
 Returns true if the removal was successful, or false if the removal failed (eg as a result
 of removing an annotation which is not contained in the manager).
 @see addAnnotation()
 @see compositionRemoved()
 @see compositionAboutToBeRemoved()
 @see clear()
%End

    void clear();
%Docstring
 Removes and deletes all annotations from the manager.
 @see removeAnnotation()
%End

    QList< QgsAnnotation * > annotations() const;
%Docstring
 Returns a list of all annotations contained in the manager.
%End

    bool readXml( const QDomElement &element, const QDomDocument &doc );
%Docstring
 Reads the manager's state from a DOM element, restoring all annotations
 present in the XML document.
 @see writeXml()
%End

    QDomElement writeXml( QDomDocument &doc ) const;
%Docstring
 Returns a DOM element representing the state of the manager.
 @see readXml()
%End

  signals:

    void annotationAdded( QgsAnnotation *annotation );
%Docstring
Emitted when a annotation has been added to the manager
%End

    void annotationRemoved();
%Docstring
Emitted when an annotation was removed from the manager
%End

    void annotationAboutToBeRemoved( QgsAnnotation *annotation );
%Docstring
Emitted when an annotation is about to be removed from the manager
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/annotations/qgsannotationmanager.h                          *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
