/***************************************************************************
  qgsqgisprofileimportdialog.h
  ----------------------------
  begin                : June 2026
  copyright            : (C) 2026 by Francesco Mazzi
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSQGISPROFILEIMPORTDIALOG_H
#define QGSQGISPROFILEIMPORTDIALOG_H

#include "qgis_app.h"
#include "qgsqgisprofileimporter.h"

#include <QDialog>
#include <QList>
#include <QString>

class QDialogButtonBox;
class QLineEdit;
class QListWidget;
class QListWidgetItem;
class QPushButton;

class APP_EXPORT QgsQgisProfileImportDialog : public QDialog
{
  public:
    enum class Mode
    {
      FirstRun,
      Manual
    };

    QgsQgisProfileImportDialog( const QList<QgsQgisProfileImporter::Candidate> &candidates, const QString &targetRootProfileFolder, Mode mode, QWidget *parent = nullptr );

    QList<QgsQgisProfileImporter::Candidate> selectedCandidates() const;
    QString targetProfileName() const;

  private:
    void addCandidate( const QgsQgisProfileImporter::Candidate &candidate, bool checked = true );
    void addCustomProfile();
    void updateImportButton();
    void updateSuggestedTargetName();

    QList<QgsQgisProfileImporter::Candidate> mCandidates;
    QString mTargetRootProfileFolder;
    Mode mMode = Mode::FirstRun;
    QListWidget *mProfileList = nullptr;
    QLineEdit *mTargetProfileName = nullptr;
    QPushButton *mImportButton = nullptr;
};

#endif // QGSQGISPROFILEIMPORTDIALOG_H
