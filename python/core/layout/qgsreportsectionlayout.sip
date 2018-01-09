/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgsreportsectionlayout.h                             *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsReportSectionLayout : QgsAbstractReportSection
{
%Docstring
 A report section consisting of a single QgsLayout body.

.. warning::

   This is not considered stable API, and may change in future QGIS releases. It is
exposed to the Python bindings for unit testing purposes only.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsreportsectionlayout.h"
%End
  public:

    QgsReportSectionLayout( QgsAbstractReportSection *parentSection = 0 );
%Docstring
Constructor for QgsReportSectionLayout, attached to the specified ``parent`` section.
Note that ownership is not transferred to ``parent``.
%End

    virtual QString type() const;
    virtual QString description() const;
    virtual QIcon icon() const;


    QgsLayout *body();
%Docstring
Returns the body layout for the section.

.. seealso:: :py:func:`setBody`

.. seealso:: :py:func:`bodyEnabled`

.. seealso:: :py:func:`setBodyEnabled`
%End

    void setBody( QgsLayout *body /Transfer/ );
%Docstring
Sets the ``body`` layout for the section. Ownership of ``body``
is transferred to the report section.

.. seealso:: :py:func:`body`

.. seealso:: :py:func:`bodyEnabled`

.. seealso:: :py:func:`setBodyEnabled`
%End

    bool bodyEnabled() const;
%Docstring
Returns true if the body for the section is enabled.

.. seealso:: :py:func:`setBodyEnabled`

.. seealso:: :py:func:`body`

.. seealso:: :py:func:`setBody`
%End

    void setBodyEnabled( bool enabled );
%Docstring
Sets whether the body for the section is ``enabled``.

.. seealso:: :py:func:`bodyEnabled`

.. seealso:: :py:func:`body`

.. seealso:: :py:func:`setBody`
%End

    virtual QgsReportSectionLayout *clone() const /Factory/;

    virtual bool beginRender();

    virtual QgsLayout *nextBody( bool &ok );


  protected:

    virtual bool writePropertiesToElement( QDomElement &element, QDomDocument &document, const QgsReadWriteContext &context ) const;

    virtual bool readPropertiesFromElement( const QDomElement &element, const QDomDocument &document, const QgsReadWriteContext &context );


};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgsreportsectionlayout.h                             *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
