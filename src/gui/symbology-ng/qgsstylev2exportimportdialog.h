/***************************************************************************
    qgsstylev2exportimportdialog.h
    ---------------------
    begin                : Jan 2011
    copyright            : (C) 2011 by Alexander Bruy
    email                : alexander dot bruy at gmail dot com

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSTYLEV2EXPORTIMPORTDIALOG_H
#define QGSSTYLEV2EXPORTIMPORTDIALOG_H

#include <QDialog>
#include <QUrl>
#include <QProgressDialog>
#include <QTemporaryFile>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QStandardItem>

#include "qgsstylev2groupselectiondialog.h"

#include "ui_qgsstylev2exportimportdialogbase.h"

class QgsStyleV2;

/** \ingroup gui
 * \class QgsStyleV2ExportImportDialog
 */
class GUI_EXPORT QgsStyleV2ExportImportDialog : public QDialog, private Ui::QgsStyleV2ExportImportDialogBase
{
    Q_OBJECT

  public:
    enum Mode
    {
      Export,
      Import
    };

    // constructor
    // mode argument must be 0 for saving and 1 for loading
    QgsStyleV2ExportImportDialog( QgsStyleV2* style, QWidget *parent = nullptr, Mode mode = Export );
    ~QgsStyleV2ExportImportDialog();

    /**
     * @brief selectSymbols select symbols by name
     * @param symbolNames list of symbol names
     */
    void selectSymbols( const QStringList& symbolNames );
    /**
     * @brief deselectSymbols deselect symbols by name
     * @param symbolNames list of symbol names
     */
    void deselectSymbols( const QStringList& symbolNames );

  public slots:
    void doExportImport();
    /**
     * @brief selectByGroup open select by group dialog
     */
    void selectByGroup();
    /**
     * @brief selectAll selects all symbols
     */
    void selectAll();
    /**
     * @brief clearSelection deselects all symbols
     */
    void clearSelection();
    /**
     * Select the symbols belonging to the given group
     * @param groupName the name of the group to be selected
     */
    void selectGroup( const QString& groupName );
    /**
     * Deselect the symbols belonging to the given group
     * @param groupName the name of the group to be deselected
     */
    void deselectGroup( const QString& groupName );
    /**
     * @brief selectSmartgroup selects all symbols from a smart group
     * @param groupName
     */
    void selectSmartgroup( const QString& groupName );
    /**
     * @brief deselectSmartgroup deselects all symbols from a smart group
     * @param groupName
     */
    void deselectSmartgroup( const QString& groupName );

    void importTypeChanged( int );
    void browse();

  private slots:
    void httpFinished();
    void fileReadyRead();
    void updateProgress( qint64, qint64 );
    void downloadCanceled();
    void selectionChanged( const QItemSelection & selected, const QItemSelection & deselected );

  private:
    void downloadStyleXML( const QUrl& url );
    bool populateStyles( QgsStyleV2* style );
    void moveStyles( QModelIndexList* selection, QgsStyleV2* src, QgsStyleV2* dst );

    QProgressDialog *mProgressDlg;
    QgsStyleV2GroupSelectionDialog *mGroupSelectionDlg;
    QTemporaryFile *mTempFile;
    QNetworkAccessManager *mNetManager;
    QNetworkReply *mNetReply;

    QString mFileName;
    Mode mDialogMode;

    QgsStyleV2* mQgisStyle;
    QgsStyleV2* mTempStyle;
};

#endif // QGSSTYLEV2EXPORTIMPORTDIALOG_H
