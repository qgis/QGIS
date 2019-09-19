/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/validity/qgsvaliditycheckcontext.h                          *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/



class QgsValidityCheckContext
{
%Docstring
Base class for validity check contexts.

QgsAbstractValidityCheck subclasses are passed a QgsValidityCheckContext subclass which
encapsulates the context around that particular check type. For instance, a QgsAbstractValidityCheck
of the QgsAbstractValidityCheck.TypeLayoutCheck type will be passed a QgsLayoutValidityCheckContext
context, containing a reference to the QgsLayout to be checked.

.. versionadded:: 3.6
%End

%TypeHeaderCode
#include "qgsvaliditycheckcontext.h"
%End
%ConvertToSubClassCode
    if ( dynamic_cast<QgsLayoutValidityCheckContext *>( sipCpp ) != NULL )
      sipType = sipType_QgsLayoutValidityCheckContext;
    else
      sipType = 0;
%End
  public:

    enum ContextType
    {
      TypeLayoutContext,
      TypeUserContext,
    };

    virtual int type() const = 0;
%Docstring
Returns the context type.
%End

    virtual ~QgsValidityCheckContext();

};

class QgsLayoutValidityCheckContext : QgsValidityCheckContext
{
%Docstring
Validity check context for print layout validation.

QgsLayoutValidityCheckContext are passed to QgsAbstractValidityCheck subclasses which
indicate they are of the QgsAbstractValidityCheck.TypeLayoutCheck type.

.. versionadded:: 3.6
%End

%TypeHeaderCode
#include "qgsvaliditycheckcontext.h"
%End
  public:

    QgsLayoutValidityCheckContext( QgsLayout *layout );
%Docstring
Constructor for QgsLayoutValidityCheckContext for the specified ``layout``.
%End

    virtual int type() const;

    QgsLayout *layout;

};
/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/validity/qgsvaliditycheckcontext.h                          *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
