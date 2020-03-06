/***************************************************************************
                             qgsmodeldesignerdialog.h
                             ------------------------
    Date                 : March 2020
    Copyright            : (C) 2020 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMODELDESIGNERDIALOG_H
#define QGSMODELDESIGNERDIALOG_H

#include "qgis.h"
#include "qgis_gui.h"
#include "ui_qgsmodeldesignerdialogbase.h"

#include "qgsprocessingtoolboxmodel.h"

class QgsMessageBar;
class QgsProcessingModelAlgorithm;

///@cond NOT_STABLE

#ifndef SIP_RUN

class GUI_EXPORT QgsModelerToolboxModel : public QgsProcessingToolboxProxyModel
{
  public:
    explicit QgsModelerToolboxModel( QObject *parent = nullptr );
    Qt::ItemFlags flags( const QModelIndex &index ) const override;
    Qt::DropActions supportedDragActions() const override;

};

#endif

/**
 * \ingroup gui
 * \brief Model designer dialog base class
 * \warning Not stable API
 * \since QGIS 3.14
 */
class GUI_EXPORT QgsModelDesignerDialog : public QMainWindow, public Ui::QgsModelDesignerDialogBase
{
  public:

    QgsModelDesignerDialog( QWidget *parent SIP_TRANSFERTHIS = nullptr, Qt::WindowFlags flags = nullptr );

  protected:

    virtual void repaintModel( bool showControls = true ) = 0;
    virtual QgsProcessingModelAlgorithm *model() = 0;
    virtual void addAlgorithm( const QString &algorithmId, const QPointF &pos ) = 0;
    virtual void addInput( const QString &inputId, const QPointF &pos ) = 0;

    QToolBar *toolbar() { return mToolbar; }
    QAction *actionOpen() { return mActionOpen; }
    QAction *actionSave() { return mActionSave; }
    QAction *actionSaveAs() { return mActionSaveAs; }
    QAction *actionSaveInProject() { return mActionSaveInProject; }
    QAction *actionEditHelp() { return mActionEditHelp; }
    QAction *actionRun() { return mActionRun; }
    QAction *actionExportImage() { return mActionExportImage; }
    QLineEdit *textName() { return mNameEdit; }
    QLineEdit *textGroup() { return mGroupEdit; }
    QTreeWidget *inputsTree() { return mInputsTreeWidget; }

    QgsMessageBar *messageBar() { return mMessageBar; }
    QGraphicsView *view() { return mView; }

    void updateVariablesGui();

  private slots:

    void zoomIn();
    void zoomOut();
    void zoomActual();
    void zoomFull();
    void exportToImage();
    void exportToPdf();
    void exportToSvg();
    void exportAsPython();

  private:

    QgsMessageBar *mMessageBar = nullptr;
    QgsModelerToolboxModel *mAlgorithmsModel = nullptr;

};

///@endcond

#endif // QGSMODELDESIGNERDIALOG_H
