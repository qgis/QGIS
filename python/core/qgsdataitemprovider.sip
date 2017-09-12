/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsdataitemprovider.h                                       *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsDataItemProvider
{
%Docstring
 This is the interface for those who want to add custom data items to the browser tree.

 The method createDataItem() is ever called only if capabilities() return non-zero value.
 There are two occasions when createDataItem() is called:
 1. to create root items (passed path is empty, parent item is null).
 2. to create items in directory structure. For this capabilities have to return at least
    of the following: QgsDataProider.Dir or QgsDataProvider.File. Passed path is the file
    or directory being inspected, parent item is a valid QgsDirectoryItem

.. versionadded:: 2.10
%End

%TypeHeaderCode
#include "qgsdataitemprovider.h"
%End
  public:
    virtual ~QgsDataItemProvider();

    virtual QString name() = 0;
%Docstring
Human-readable name of the provider name
 :rtype: str
%End

    virtual int capabilities() = 0;
%Docstring
Return combination of flags from QgsDataProvider.DataCapabilities
 :rtype: int
%End

    virtual QgsDataItem *createDataItem( const QString &path, QgsDataItem *parentItem ) = 0 /Factory/;
%Docstring
Caller takes responsibility of deleting created items.
 :rtype: QgsDataItem
%End

    virtual QVector<QgsDataItem *> createDataItems( const QString &path, QgsDataItem *parentItem );
%Docstring
Caller takes responsibility of deleting created items.
 :rtype: list of QgsDataItem
%End
};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsdataitemprovider.h                                       *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
