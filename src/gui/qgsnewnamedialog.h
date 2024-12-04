/***************************************************************************
    qgsnewnamedialog.h
                             -------------------
    begin                : May, 2015
    copyright            : (C) 2015 Radim Blazek
    email                : radim.blazek@gmail.com
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSNEWNAMEDIALOG_H
#define QGSNEWNAMEDIALOG_H

class QLabel;
class QLineEdit;

#include "qgsdialog.h"
#include "qgis_gui.h"
#include <QRegularExpression>

/**
 * \ingroup gui
 * \brief New name, for example new layer name dialog. If existing names are provided,
 * the dialog warns users if an entered name already exists.
 */
class GUI_EXPORT QgsNewNameDialog : public QgsDialog
{
    Q_OBJECT
  public:
    /**
     * New dialog constructor.
     * \param source original data source name, e.g. original layer name of the layer to be copied
     * \param initial initial name
     * \param extensions base name extensions, e.g. raster base name band extensions or vector layer type extensions
     * \param existing existing names
     * \param cs case sensitivity for new name to existing names comparison
     * \param parent parent widget
     * \param flags window flags
     * \note Earlier versions had a similar constructor but with extra arguments for \a regexp which were removed in QGIS 3.22 as they relied on the deprecated QRegExp class. Use setRegularExpression() instead.
     * \since QGIS 3.22
     */
    QgsNewNameDialog( const QString &source = QString(), const QString &initial = QString(), const QStringList &extensions = QStringList(), const QStringList &existing = QStringList(), Qt::CaseSensitivity cs = Qt::CaseSensitive, QWidget *parent SIP_TRANSFERTHIS = nullptr, Qt::WindowFlags flags = QgsGuiUtils::ModalDialogFlags );

    /**
     * Sets the hint string for the dialog (the text shown above the name
     * input box).
     * \param hintString hint text
     * \see hintString()
     */
    void setHintString( const QString &hintString );

    /**
     * Returns the hint string for the dialog (the text shown above the name
     * input box).
     * \see setHintString()
     */
    QString hintString() const;

    /**
     * Sets whether users are permitted to overwrite existing names. If TRUE, then
     * the dialog will reflect that the new name will overwrite an existing name. If FALSE,
     * then the dialog will not accept names which already exist.
     * \see overwriteEnabled()
     */
    void setOverwriteEnabled( bool enabled );

    /**
     * Returns whether users are permitted to overwrite existing names.
     * \see setOverwriteEnabled()
     */
    bool overwriteEnabled() const { return mOverwriteEnabled; }

    /**
     * Sets whether users are permitted to leave the widget empty.
     * If TRUE, the dialog will accept an empty name value.
     * \see allowEmptyName()
     * \since QGIS 3.14
     */
    void setAllowEmptyName( bool allowed );

    /**
     * Returns TRUE if the widget can be left empty (no name filled).
     * \see setAllowEmptyName()
     * \since QGIS 3.14
     */
    bool allowEmptyName() const { return mAllowEmptyName; }

    /**
     * Sets the string used for warning users if a conflicting name exists.
     * \param string warning string. If empty a default warning string will be used.
     * \see conflictingNameWarning()
     */
    void setConflictingNameWarning( const QString &string );

    /**
     * Returns the string used for warning users if a conflicting name exists.
     * \see setConflictingNameWarning()
     */
    QString conflictingNameWarning() const { return mConflictingNameWarning; }

    /**
     * Sets a regular \a expression to use for validating user-entered names in the dialog.
     *
     * \since QGIS 3.22
     */
    void setRegularExpression( const QString &expression );

    /**
     * Name entered by user.
     * \returns new name
     * \see newNameChanged()
     */
    QString name() const;

    /**
     * Test if name or name with at least one extension exists.
     * \param name name or base name
     * \param extensions base name extensions
     * \param existing existing names
     * \param cs case sensitivity for new name to existing names comparison
     * \returns TRUE if name exists
     */
    static bool exists( const QString &name, const QStringList &extensions, const QStringList &existing, Qt::CaseSensitivity cs = Qt::CaseSensitive );
  signals:

    // TODO QGIS 4.0 - rename to nameChanged

    /**
     * Emitted when the name is changed in the dialog.
     * \since QGIS 3.2
     */
    void newNameChanged();

  public slots:
    // TODO QGIS 4.0 - rename to onNameChanged
    void nameChanged();

  protected:
    QStringList mExiting;
    QStringList mExtensions;
    Qt::CaseSensitivity mCaseSensitivity = Qt::CaseSensitive;
    QLabel *mHintLabel = nullptr;
    QLineEdit *mLineEdit = nullptr;
    //! List of names with extensions
    QLabel *mNamesLabel = nullptr;
    QLabel *mErrorLabel = nullptr;
    QString mOkString;
    QRegularExpression mRegularExpression;
    bool mOverwriteEnabled = true;
    bool mAllowEmptyName = false;
    QString mConflictingNameWarning;

    QString highlightText( const QString &text );
    static QStringList fullNames( const QString &name, const QStringList &extensions );
    // get list of existing names
    static QStringList matching( const QStringList &newNames, const QStringList &existingNames, Qt::CaseSensitivity cs = Qt::CaseSensitive );
};

#endif // QGSNEWNAMEDIALOG_H
