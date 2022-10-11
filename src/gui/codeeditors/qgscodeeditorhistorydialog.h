/***************************************************************************
    qgscodeeditorhistorydialog.h
    ----------------------
    begin                : October 2022
    copyright            : (C) 2022 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCODEEDITORHISTORYDIALOG_H
#define QGSCODEEDITORHISTORYDIALOG_H

#include "ui_qgscodeditorhistorydialogbase.h"
#include <QDialog>
#include <QPointer>
#include <QStringListModel>
#include "qgis_gui.h"
#include "qgis_sip.h"

#define SIP_NO_FILE

class QgsCodeEditor;


///@cond PRIVATE

class CodeHistoryModel : public QStringListModel
{
    Q_OBJECT

  public:
    CodeHistoryModel( QObject *parent );
    QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const override;

  private:

    QFont mFont;
};

///@endcond


/**
 * \ingroup gui
 * \class QgsCodeEditorHistoryDialog
 * \brief A dialog for displaying and managing command history for a QgsCodeEditor widget.
 * \note Not available in Python bindings
 * \since QGIS 3.30
 */
class GUI_EXPORT QgsCodeEditorHistoryDialog : public QDialog, private Ui::QgsCodeEditorHistoryDialogBase
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsCodeEditorHistoryDialog.
     * \param editor associated code editor widget
     * \param parent parent widget
     */
    QgsCodeEditorHistoryDialog( QgsCodeEditor *editor, QWidget *parent SIP_TRANSFERTHIS = nullptr );

  private slots:

    void executeSelectedHistory();
    void runCommand( const QModelIndex &index );
    void saveHistory();
    void reloadHistory();
    void deleteItem();

  private:

    QPointer< QgsCodeEditor > mEditor;
    CodeHistoryModel *mModel = nullptr;

};

#endif // QGSCODEEDITORHISTORYDIALOG_H
