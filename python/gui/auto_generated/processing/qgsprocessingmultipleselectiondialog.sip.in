/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/processing/qgsprocessingmultipleselectiondialog.h            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsProcessingMultipleSelectionDialog : QDialog
{
%Docstring
Dialog for configuration of a matrix (fixed table) parameter.

.. note::

   Not stable API

.. versionadded:: 3.6
%End

%TypeHeaderCode
#include "qgsprocessingmultipleselectiondialog.h"
%End
  public:

    QgsProcessingMultipleSelectionDialog( const QVariantList &availableOptions = QVariantList(),
                                          const QVariantList &selectedOptions = QVariantList(),
                                          QWidget *parent /TransferThis/ = 0, Qt::WindowFlags flags = 0 );
%Docstring
Constructor for QgsProcessingMultipleSelectionDialog.

The ``availableOptions`` list specifies the list of standard known options for the parameter,
whilst the ``selectedOptions`` list specifies which options should be initially selected.

The ``selectedOptions`` list may contain extra options which are not present in ``availableOptions``,
in which case they will be also added as existing options within the dialog.
%End


    void setValueFormatter( SIP_PYCALLABLE );
%Docstring
Sets a callback function to use when encountering an invalid geometry and
%End
%MethodCode

    Py_BEGIN_ALLOW_THREADS

    sipCpp->setValueFormatter( [a0]( const QVariant &v )->QString
    {
      QString res;
      SIP_BLOCK_THREADS
      PyObject *s = sipCallMethod( NULL, a0, "D", &v, sipType_QVariant, NULL );
      int state;
      int sipIsError = 0;
      QString *t1 = reinterpret_cast<QString *>( sipConvertToType( s, sipType_QString, 0, SIP_NOT_NONE, &state, &sipIsError ) );
      if ( sipIsError == 0 )
      {
        res = QString( *t1 );
      }
      sipReleaseType( t1, sipType_QString, state );
      SIP_UNBLOCK_THREADS
      return res;
    } );

    Py_END_ALLOW_THREADS
%End


    QVariantList selectedOptions() const;
%Docstring
Returns the ordered list of selected options.
%End

};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/processing/qgsprocessingmultipleselectiondialog.h            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
