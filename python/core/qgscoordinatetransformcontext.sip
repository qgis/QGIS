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


.. seealso:: :py:class:`QgsDatumTransform`

.. seealso:: :py:class:`QgsCoordinateTransform`


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

    ~QgsCoordinateTransformContext();

    QgsCoordinateTransformContext( const QgsCoordinateTransformContext &rhs );
%Docstring
Copy constructor
%End



    void clear();
%Docstring
Clears all stored transform information from the context.
%End



    QMap< QPair< QString, QString>, QgsDatumTransform::TransformPair > sourceDestinationDatumTransforms() const;
%Docstring
Returns the stored mapping for source to destination CRS pairs to associated datum transforms to use.
The map keys will be :py:func:`QgsCoordinateReferenceSystems.authid()`s.

If either the source transform ID or destination transform ID is -1, then no datum transform is
required for transformations for that source or destination.

.. warning::

   This method should not be used to calculate the corresponding datum transforms
to use for a coordinate transform. Instead, always use calculateDatumTransforms()
to determine this.

.. seealso:: :py:func:`addSourceDestinationDatumTransform`
%End

    bool addSourceDestinationDatumTransform( const QgsCoordinateReferenceSystem &sourceCrs,
        const QgsCoordinateReferenceSystem &destinationCrs,
        int sourceTransformId,
        int destinationTransformId );
%Docstring
Adds a new ``sourceTransform`` and ``destinationTransform`` to use when projecting coordinates
from the the specified ``sourceCrs`` to the specified ``destinationCrs``.

If either ``sourceTransformId`` or ``destinationTransformId`` is -1, then no datum transform is
required for transformations for that source or destination.

Returns true if the new transform pair was added successfully.

.. note::

   Transforms set using this method will override any specific source or destination
transforms set by addSourceDatumTransform() or addDestinationDatumTransform().

.. seealso:: :py:func:`sourceDestinationDatumTransforms`

.. seealso:: :py:func:`removeSourceDestinationDatumTransform`
%End

    void removeSourceDestinationDatumTransform( const QgsCoordinateReferenceSystem &sourceCrs,
        const QgsCoordinateReferenceSystem &destinationCrs );
%Docstring
Removes the source to destination datum transform pair for the specified ``sourceCrs`` and
``destinationCrs``.

.. seealso:: :py:func:`addSourceDestinationDatumTransform`
%End

    bool hasTransform( const QgsCoordinateReferenceSystem &source,
                       const QgsCoordinateReferenceSystem &destination ) const;
%Docstring
Returns true if the context has a valid datum transform to use
when transforming from the specified ``source`` CRS to ``destination`` CRS.

.. note::

   source and destination are reversible.
%End

    QgsDatumTransform::TransformPair calculateDatumTransforms( const QgsCoordinateReferenceSystem &source,
        const QgsCoordinateReferenceSystem &destination ) const;
%Docstring
Returns the pair of source and destination datum transforms to use
for a transform from the specified ``source`` CRS to ``destination`` CRS.

Returns an ID of -1 if a datum transform should not be used for the source or
destination.

.. note::

   source and destination are reversible.
%End

    bool readXml( const QDomElement &element, const QgsReadWriteContext &context, QStringList &missingTransforms /Out/ );
%Docstring
Reads the context's state from a DOM ``element``.

Returns false if transforms stored in the XML are not available. In this case ``missingTransforms`` will be
filled with missing datum transform strings.

.. seealso:: :py:func:`writeXml`
%End

    void writeXml( QDomElement &element, const QgsReadWriteContext &context ) const;
%Docstring
Writes the context's state to a DOM ``element``.

.. seealso:: :py:func:`readXml`
%End


    void readSettings();
%Docstring
Reads the context's state from application settings.

.. seealso:: :py:func:`readSettings`
%End

    void writeSettings();
%Docstring
Write the context's state to application settings.

.. seealso:: :py:func:`writeSettings`
%End


};





/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgscoordinatetransformcontext.h                             *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
