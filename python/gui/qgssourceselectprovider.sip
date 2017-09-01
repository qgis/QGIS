/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgssourceselectprovider.h                                    *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsSourceSelectProvider
{
%Docstring
.. seealso:: QgsDataSourceManagerDialog

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgssourceselectprovider.h"
%End
  public:
    virtual ~QgsSourceSelectProvider();

    virtual QString providerKey() const = 0;
%Docstring
Provider key
 :rtype: str
%End

    virtual QString text() const = 0;
%Docstring
Text for the menu item entry
 :rtype: str
%End

    virtual QIcon icon() const = 0 /Factory/;
%Docstring
Caller takes responsibility of deleting created.
 :rtype: QIcon
%End

    virtual QgsAbstractDataSourceWidget *createDataSourceWidget( ) = 0 /Factory/;
%Docstring
Caller takes responsibility of deleting created.
 :rtype: QgsAbstractDataSourceWidget
%End

};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgssourceselectprovider.h                                    *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
