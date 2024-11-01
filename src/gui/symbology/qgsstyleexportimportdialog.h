/***************************************************************************
    qgsstyleexportimportdialog.h
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

#include "ui_qgsstyleexportimportdialogbase.h"
#include "qgis_gui.h"
#include "qgis_sip.h"
#include "qgshelp.h"

#include <memory>

class QgsStyle;
class QgsStyleGroupSelectionDialog;
class QgsTemporaryCursorOverride;
class QgsStyleProxyModel;
class QTemporaryFile;

/**
 * \ingroup gui
 * \class QgsStyleExportImportDialog
 */
class GUI_EXPORT QgsStyleExportImportDialog : public QDialog, private Ui::QgsStyleExportImportDialogBase
{
    Q_OBJECT

  public:
    //! Dialog modes
    enum Mode
    {
      Export, //!< Export existing symbols mode
      Import, //!< Import xml file mode
    };

    /**
     * Constructor for QgsStyleExportImportDialog, with the specified \a parent widget.
     *
     * Creates a dialog for importing symbols into the given \a style, or exporting symbols from the \a style.
     * The \a mode argument dictates whether the dialog is to be used for exporting or importing symbols.
     */
    QgsStyleExportImportDialog( QgsStyle *style, QWidget *parent SIP_TRANSFERTHIS = nullptr, Mode mode = Export );
    ~QgsStyleExportImportDialog() override;

    /**
     * Sets the initial \a path to use for importing files, when the dialog is in a Import mode.
     *
     * \since QGIS 3.6
     */
    void setImportFilePath( const QString &path );

    /**
     * \brief selectSymbols select symbols by name
     * \param symbolNames list of symbol names
     */
    void selectSymbols( const QStringList &symbolNames );

    /**
     * \brief deselectSymbols deselect symbols by name
     * \param symbolNames list of symbol names
     */
    void deselectSymbols( const QStringList &symbolNames );

  public slots:
    void doExportImport();

    /**
     * \brief selectByGroup open select by group dialog
     */
    void selectByGroup();

    /**
     * \brief selectAll selects all symbols
     */
    void selectAll();

    /**
     * \brief clearSelection deselects all symbols
     */
    void clearSelection();

    /**
     * Selects favorite symbols
     * \since QGIS 3.14
     */
    void selectFavorites();

    /**
     * Deselects favorite symbols
     * \since QGIS 3.14
     */
    void deselectFavorites();

    /**
     * Select the symbols belonging to the given tag
     * \param tagName the name of the group to be selected
     */
    void selectTag( const QString &tagName );

    /**
     * Deselect the symbols belonging to the given tag
     * \param tagName the name of the group to be deselected
     */
    void deselectTag( const QString &tagName );

    /**
     * \brief selectSmartgroup selects all symbols from a smart group
     * \param groupName
     */
    void selectSmartgroup( const QString &groupName );

    /**
     * \brief deselectSmartgroup deselects all symbols from a smart group
     * \param groupName
     */
    void deselectSmartgroup( const QString &groupName );

    void importTypeChanged( int );

  private slots:
    void selectionChanged( const QItemSelection &selected, const QItemSelection &deselected );
    void showHelp();

    void fetch();
    void importFileChanged( const QString &path );

  private:
    enum ImportSource
    {
      File,
      //Official,
      Url,
    };

    void downloadStyleXml( const QUrl &url );
    bool populateStyles();
    void moveStyles( QModelIndexList *selection, QgsStyle *src, QgsStyle *dst );

    QgsStyleGroupSelectionDialog *mGroupSelectionDlg = nullptr;
    QTemporaryFile *mTempFile = nullptr;

    QString mFileName;
    Mode mDialogMode;

    QgsStyleProxyModel *mModel = nullptr;

    QgsStyle *mStyle = nullptr;
    std::unique_ptr<QgsStyle> mTempStyle;
    std::unique_ptr<QgsTemporaryCursorOverride> mCursorOverride;
};

#endif // QGSSTYLEV2EXPORTIMPORTDIALOG_H
