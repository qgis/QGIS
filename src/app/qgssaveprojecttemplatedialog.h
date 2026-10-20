/***************************************************************************
    qgssaveprojecttemplatedialog.h
    ------------------------------
    begin                : September 2026
    copyright            : (C) 2026 by Jacky Volpes
    email                : jacky dot volpes at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSSAVEPROJECTTEMPLATEDIALOG_H
#define QGSSAVEPROJECTTEMPLATEDIALOG_H

#include "qgis_app.h"

#include <QDialog>

class QComboBox;
class QDialogButtonBox;
class QLineEdit;

/**
 * \brief A dialog prompting for the name and the destination directory
 * of a project saved as template.
 *
 * \since QGIS 4.4
 */
class APP_EXPORT QgsSaveProjectTemplateDialog : public QDialog
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsSaveProjectTemplateDialog.
     * \param templateDirectories directories offered as destination
     * \param initialName initial template name
     * \param parent parent widget
     */
    QgsSaveProjectTemplateDialog( const QStringList &templateDirectories, const QString &initialName, QWidget *parent = nullptr );

    //! Returns the template name without extension
    QString templateName() const;

    //! Returns the destination directory
    QString templateDirectory() const;

  public slots:

    void accept() override;

  private:
    QComboBox *mDirectoryComboBox = nullptr;
    QLineEdit *mNameLineEdit = nullptr;
    QDialogButtonBox *mButtonBox = nullptr;
};

#endif // QGSSAVEPROJECTTEMPLATEDIALOG_H
