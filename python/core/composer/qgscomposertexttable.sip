/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/composer/qgscomposertexttable.h                             *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsComposerTextTableV2 : QgsComposerTableV2
{
%Docstring
A text table item that reads text from string lists

.. versionadded:: 2.10
%End

%TypeHeaderCode
#include "qgscomposertexttable.h"
%End
  public:
    QgsComposerTextTableV2( QgsComposition *c /TransferThis/, bool createUndoCommands );

    void addRow( const QStringList &row );
%Docstring
Adds a row to the table
:param row: list of strings to use for each cell's value in the newly added row

.. note::

   If row is shorter than the number of columns in the table than blank cells
will be inserted at the end of the row. If row contains more strings then the number
of columns in the table then these extra strings will be ignored.

.. note::

   if adding many rows, setContents() is much faster
%End

    void setContents( const QList< QStringList > &contents );
%Docstring
Sets the contents of the text table.
:param contents: list of table rows

.. seealso:: :py:func:`addRow`
%End

    virtual bool getTableContents( QgsComposerTableContents &contents );


    virtual void addFrame( QgsComposerFrame *frame, bool recalcFrameSizes = true );


};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/composer/qgscomposertexttable.h                             *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
