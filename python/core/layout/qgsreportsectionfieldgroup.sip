/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgsreportsectionfieldgroup.h                         *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsReportSectionFieldGroup : QgsAbstractReportSection
{
%Docstring
 A report section consisting of a features

.. warning::

   This is not considered stable API, and may change in future QGIS releases. It is
exposed to the Python bindings for unit testing purposes only.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsreportsectionfieldgroup.h"
%End
  public:

    QgsReportSectionFieldGroup( QgsAbstractReportSection *parent = 0 );
%Docstring
Constructor for QgsReportSectionFieldGroup, attached to the specified ``parent`` section.
Note that ownership is not transferred to ``parent``.
%End

    virtual QString type() const;

    QgsLayout *body();
%Docstring
Returns the body layout for the section.

.. seealso:: :py:func:`setBody()`
%End

    void setBody( QgsLayout *body /Transfer/ );
%Docstring
Sets the ``body`` layout for the section. Ownership of ``body``
is transferred to the report section.

.. seealso:: :py:func:`body()`
%End

    QgsVectorLayer *layer();
%Docstring
Returns the vector layer associated with this section.

.. seealso:: :py:func:`setLayer()`
%End

    void setLayer( QgsVectorLayer *layer );
%Docstring
Sets the vector ``layer`` associated with this section.

.. seealso:: :py:func:`layer()`
%End

    QString field() const;
%Docstring
Returns the field associated with this section.

.. seealso:: :py:func:`setField()`
%End

    void setField( const QString &field );
%Docstring
Sets the ``field`` associated with this section.

.. seealso:: :py:func:`field()`
%End

    virtual QgsReportSectionFieldGroup *clone() const /Factory/;

    virtual bool beginRender();

    virtual QgsLayout *nextBody( bool &ok );

    virtual void reset();

    virtual void setParentSection( QgsAbstractReportSection *parent );


  protected:

    virtual bool writePropertiesToElement( QDomElement &element, QDomDocument &document, const QgsReadWriteContext &context ) const;

    virtual bool readPropertiesFromElement( const QDomElement &element, const QDomDocument &document, const QgsReadWriteContext &context );


};



/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgsreportsectionfieldgroup.h                         *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
