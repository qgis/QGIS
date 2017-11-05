/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgscoordinatetransformcontext.h                             *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsCoordinateTransformContext
{
%Docstring
 Contains information about the context in which a coordinate transform is executed.

 The context stores various information regarding which coordinate transforms should
 be used when transforming points from a source to destination coordinate reference
 system.

 The highest priority transforms are those set using addSourceDestinationDatumTransform()
 and which the transform has a matching source to destination CRS pair.

 Failing this, if the source CRS has a matching transform specified by
 addSourceDatumTransform() then this datum transform will be used. The same logic
 applies for destination CRS transforms set using addDestinationDatumTransform().

.. note::

   QgsCoordinateTransformContext objects are thread safe for read and write.

.. note::

   QgsCoordinateTransformContext objects are implicitly shared.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgscoordinatetransformcontext.h"
%End
  public:

    QgsCoordinateTransformContext();
%Docstring
 Constructor for QgsCoordinateTransformContext.
%End

    QgsCoordinateTransformContext( const QgsCoordinateTransformContext &rhs );
%Docstring
 Copy constructor
%End



    void clear();
%Docstring
 Clears all stored transform information from the context.
%End

    QMap<QString, int> sourceDatumTransforms() const;
%Docstring
 Returns the stored mapping for source CRS to associated datum transform to use.
 The map keys will be QgsCoordinateReferenceSystems.authid()s.

 \warning This method should not be used to calculate the corresponding datum transforms
 to use for a coordinate transform. Instead, always use calculateDatumTransforms()
 to determine this.

.. seealso:: addSourceDatumTransform()
.. seealso:: destinationDatumTransforms()
 :rtype: QMap<str, int>
%End

    bool addSourceDatumTransform( const QgsCoordinateReferenceSystem &crs, int transform );
%Docstring
 Adds a new ``transform`` to use when projecting coordinates from the specified source
 ``crs``.

 If ``transform`` is -1, then any existing source transform for the ``crs`` will be removed.

 Returns true if the new transform was added successfully.

 \warning Transforms set using this method may be overridden by specific source/destination
 transforms set by addSourceDestinationDatumTransform().

.. seealso:: sourceDatumTransforms()
.. seealso:: addDestinationDatumTransform()
 :rtype: bool
%End

    QMap< QString, int > destinationDatumTransforms() const;
%Docstring
 Returns the stored mapping for destination CRS to associated datum transform to use.
 The map keys will be QgsCoordinateReferenceSystems.authid()s.

 \warning This method should not be used to calculate the corresponding datum transforms
 to use for a coordinate transform. Instead, always use calculateDatumTransforms()
 to determine this.

.. seealso:: addDestinationDatumTransform()
.. seealso:: sourceDatumTransforms()
 :rtype: QMap< str, int >
%End

    bool addDestinationDatumTransform( const QgsCoordinateReferenceSystem &crs, int transform );
%Docstring
 Adds a new ``transform`` to use when projecting coordinates to the specified destination
 ``crs``.

 If ``transform`` is -1, then any existing destination transform for the ``crs`` will be removed.

 Returns true if the new transform was added successfully.

 \warning Transforms set using this method may be overridden by specific source/destination
 transforms set by addSourceDestinationDatumTransform().

.. seealso:: destinationDatumTransforms()
.. seealso:: addSourceDatumTransform()
 :rtype: bool
%End

    QMap< QPair< QString, QString>, QPair< int, int > > sourceDestinationDatumTransforms() const;
%Docstring
 Returns the stored mapping for source to destination CRS pairs to associated datum transforms to use.
 The map keys will be QgsCoordinateReferenceSystems.authid()s.

 \warning This method should not be used to calculate the corresponding datum transforms
 to use for a coordinate transform. Instead, always use calculateDatumTransforms()
 to determine this.

.. seealso:: addSourceDestinationDatumTransform()
 :rtype: QMap< QPair< str, QString>, QPair< int, int > >
%End

    bool addSourceDestinationDatumTransform( const QgsCoordinateReferenceSystem &sourceCrs,
        const QgsCoordinateReferenceSystem &destinationCrs,
        int sourceTransform,
        int destinationTransform );
%Docstring
 Adds a new ``sourceTransform`` and ``destinationTransform`` to use when projecting coordinates
 from the the specified ``sourceCrs`` to the specified ``destinationCrs``.

 If either ``sourceTransform`` or ``destinationTransform`` is -1, then any existing source to destination
 transform for the crs pair will be removed.

 Returns true if the new transform pair was added successfully.

.. note::

   Transforms set using this method will override any specific source or destination
 transforms set by addSourceDatumTransform() or addDestinationDatumTransform().

.. seealso:: sourceDestinationDatumTransforms()
 :rtype: bool
%End

    QPair< int, int > calculateDatumTransforms( const QgsCoordinateReferenceSystem &source,
        const QgsCoordinateReferenceSystem &destination ) const;
%Docstring
 Returns the pair of source and destination datum transforms to use
 for a transform from the specified ``source`` CRS to ``destination`` CRS.

 Returns -1 if a datum transform should not be used for the source or
 destination.
 :rtype: QPair< int, int >
%End

};





/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgscoordinatetransformcontext.h                             *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
