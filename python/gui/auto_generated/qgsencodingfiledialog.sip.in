/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsencodingfiledialog.h                                      *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsEncodingFileDialog: QFileDialog
{
%Docstring
A file dialog which lets the user select the preferred encoding type for a data provider.
*
%End

%TypeHeaderCode
#include "qgsencodingfiledialog.h"
%End
  public:

    QgsEncodingFileDialog( QWidget *parent /TransferThis/ = 0,
                           const QString &caption = QString(), const QString &directory = QString(),
                           const QString &filter = QString(), const QString &encoding = QString() );
%Docstring
Constructor for QgsEncodingFileDialog
%End
    QString encoding() const;
%Docstring
Returns a string describing the chosen encoding
%End
    void addCancelAll();
%Docstring
Adds a 'Cancel All' button for the user to click
%End
    bool cancelAll();
%Docstring
Returns true if the user clicked 'Cancel All'
%End

  public slots:
    void saveUsedEncoding();

    void pbnCancelAll_clicked();

};

class QgsEncodingSelectionDialog: QDialog
{
%Docstring
A dialog which presents the user with a choice of file encodings.

.. versionadded:: 3.0
*
%End

%TypeHeaderCode
#include "qgsencodingfiledialog.h"
%End
  public:

    QgsEncodingSelectionDialog( QWidget *parent /TransferThis/ = 0,
                                const QString &caption = QString(), const QString &encoding = QString(),
                                Qt::WindowFlags flags = Qt::WindowFlags() );
%Docstring
Constructor for QgsEncodingSelectionDialog.

If ``caption`` is set, it will be used as the caption within the dialog.

The ``encoding`` argument can be used to specify the encoding initially selected in the dialog.
%End

    QString encoding() const;
%Docstring
Returns the encoding selected within the dialog.

.. seealso:: :py:func:`setEncoding`
%End

    void setEncoding( const QString &encoding );
%Docstring
Sets the ``encoding`` selected within the dialog.
see encoding()
%End

};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsencodingfiledialog.h                                      *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
