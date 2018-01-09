/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgsreport.h                                          *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsReport : QObject, QgsAbstractReportSection, QgsMasterLayoutInterface
{
%Docstring
 Represents a report for use with the QgsLayout engine.

Reports consist of multiple sections, represented by QgsAbstractReportSection
subclasses.

.. warning::

   This is not considered stable API, and may change in future QGIS releases. It is
exposed to the Python bindings for unit testing purposes only.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsreport.h"
%End
  public:

    QgsReport( QgsProject *project );
%Docstring
Constructor for QgsReport, associated with the specified
``project``.

Note that ownership is not transferred to ``project``.
%End

    virtual QgsMasterLayoutInterface::Type layoutType() const;

    virtual QString type() const;
    virtual QString description() const;
    virtual QIcon icon() const;

    virtual QgsProject *layoutProject() const;
    virtual QgsReport *clone() const /Factory/;

    virtual QString name() const;
    virtual void setName( const QString &name );

    virtual QDomElement writeLayoutXml( QDomDocument &document, const QgsReadWriteContext &context ) const;

    virtual bool readLayoutXml( const QDomElement &layoutElement, const QDomDocument &document, const QgsReadWriteContext &context );


  signals:

    void nameChanged( const QString &name );
%Docstring
Emitted when the report's name is changed.

.. seealso:: :py:func:`setName`
%End

};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgsreport.h                                          *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
