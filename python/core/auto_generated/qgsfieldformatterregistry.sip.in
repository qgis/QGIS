/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsfieldformatterregistry.h                                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsFieldFormatterRegistry : QObject
{
%Docstring
The QgsFieldFormatterRegistry manages registered classes of QgsFieldFormatter.
A reference to the QgsFieldFormatterRegistry can be obtained from
:py:func:`QgsApplication.fieldFormatterRegistry()`

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsfieldformatterregistry.h"
%End
  public:

    explicit QgsFieldFormatterRegistry( QObject *parent /TransferThis/ = 0 );
%Docstring
You should not normally need to create your own field formatter registry.

Use the one provided by `QgsApplication.fieldFormatterRegistry()` instead.
%End
    ~QgsFieldFormatterRegistry();

    void addFieldFormatter( QgsFieldFormatter *formatter /Transfer/ );
%Docstring
They will take precedence in order of adding them.
The later they are added, the more weight they have.

Ownership is transferred to the registry.
%End

    void removeFieldFormatter( QgsFieldFormatter *formatter );
%Docstring
Remove a field formatter from the registry.
The field formatter will be deleted.
%End

    void removeFieldFormatter( const QString &id );
%Docstring
Remove the field formatter with the specified id.
%End

    QgsFieldFormatter *fieldFormatter( const QString &id ) const;
%Docstring
Gets a field formatter by its id. If there is no such id registered,
a default QgsFallbackFieldFormatter with a null id will be returned instead.
%End

    QgsFieldFormatter *fallbackFieldFormatter() const;
%Docstring
Returns a basic fallback field formatter which can be used
to represent any field in an unspectacular manner.
%End

  signals:

    void fieldFormatterAdded( QgsFieldFormatter *formatter );
%Docstring
Will be emitted after a new field formatter has been added.
%End

    void fieldFormatterRemoved( QgsFieldFormatter *formatter );
%Docstring
Will be emitted just before a field formatter is removed and deleted.
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsfieldformatterregistry.h                                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
