/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgsreport.h                                          *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsReport : QgsAbstractReportSection
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

    QgsProject *project();
%Docstring
Returns the associated project.
%End

    virtual QgsReport *clone() const /Factory/;


};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgsreport.h                                          *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
