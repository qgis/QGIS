/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsnewnamedialog.h                                           *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsNewNameDialog : QgsDialog
{
%Docstring
New name, for example new layer name dialog. If existing names are provided,
the dialog warns users if an entered name already exists.

.. versionadded:: 2.10
%End

%TypeHeaderCode
#include "qgsnewnamedialog.h"
%End
  public:

    QgsNewNameDialog( const QString &source = QString(), const QString &initial = QString(),
                      const QStringList &extensions = QStringList(), const QStringList &existing = QStringList(),
                      const QRegExp &regexp = QRegExp(), Qt::CaseSensitivity cs = Qt::CaseSensitive,
                      QWidget *parent /TransferThis/ = 0, Qt::WindowFlags flags = QgsGuiUtils::ModalDialogFlags );
%Docstring
New dialog constructor.

:param source: original data source name, e.g. original layer name of the layer to be copied
:param initial: initial name
:param extensions: base name extensions, e.g. raster base name band extensions or vector layer type extensions
:param existing: existing names
:param regexp: regular expression to be used as validator, for example db tables should have "[A-Za-z_][A-Za-z0-9_]+"
:param cs: case sensitivity for new name to existing names comparison
:param parent:
:param flags:
%End

    void setHintString( const QString &hintString );
%Docstring
Sets the hint string for the dialog (the text shown above the name
input box).

:param hintString: hint text

.. seealso:: :py:func:`hintString`

.. versionadded:: 2.12
%End

    QString hintString() const;
%Docstring
Returns the hint string for the dialog (the text shown above the name
input box).

.. seealso:: :py:func:`setHintString`

.. versionadded:: 2.12
%End

    void setOverwriteEnabled( bool enabled );
%Docstring
Sets whether users are permitted to overwrite existing names. If true, then
the dialog will reflect that the new name will overwrite an existing name. If false,
then the dialog will not accept names which already exist.

.. versionadded:: 2.12

.. seealso:: :py:func:`overwriteEnabled`
%End

    bool overwriteEnabled() const;
%Docstring
Returns whether users are permitted to overwrite existing names.

.. versionadded:: 2.12

.. seealso:: :py:func:`setOverwriteEnabled`
%End

    void setConflictingNameWarning( const QString &string );
%Docstring
Sets the string used for warning users if a conflicting name exists.

:param string: warning string. If empty a default warning string will be used.

.. versionadded:: 2.12

.. seealso:: :py:func:`conflictingNameWarning`
%End

    QString conflictingNameWarning() const;
%Docstring
Returns the string used for warning users if a conflicting name exists.

.. versionadded:: 2.12

.. seealso:: :py:func:`setConflictingNameWarning`
%End

    QString name() const;
%Docstring
Name entered by user.

:return: new name
%End

    static bool exists( const QString &name, const QStringList &extensions,
                        const QStringList &existing, Qt::CaseSensitivity cs = Qt::CaseSensitive );
%Docstring
Test if name or name with at least one extension exists.

:param name: name or base name
:param extensions: base name extensions
:param existing: existing names
:param cs: case sensitivity for new name to existing names comparison

:return: true if name exists
%End
  public slots:
    void nameChanged();

  protected:

    QString highlightText( const QString &text );
%Docstring
List of names with extensions
%End
    static QStringList fullNames( const QString &name, const QStringList &extensions );
    static QStringList matching( const QStringList &newNames, const QStringList &existingNames,
                                 Qt::CaseSensitivity cs = Qt::CaseSensitive );
};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsnewnamedialog.h                                           *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
