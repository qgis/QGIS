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
#include "qgsprocessingmodelchilddependency.h"

class QgsMessageBar;
class QgsProcessingModelAlgorithm;
class QgsModelUndoCommand;
class QUndoView;
class QgsModelViewToolPan;
class QgsModelViewToolSelect;

///@cond NOT_STABLE

#ifndef SIP_RUN

class GUI_EXPORT QgsModelerToolboxModel : public QgsProcessingToolboxProxyModel
{
    Q_OBJECT
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
    Q_OBJECT
  public:

    QgsModelDesignerDialog( QWidget *parent SIP_TRANSFERTHIS = nullptr, Qt::WindowFlags flags = Qt::WindowFlags() );
    ~QgsModelDesignerDialog() override;

    void closeEvent( QCloseEvent *event ) override;

    /**
     * Starts an undo command. This should be called before any changes are made to the model.
     */
    void beginUndoCommand( const QString &text, int id = 0 );

    /**
     * Ends the current undo command. This should be called after changes are made to the model.
     */
    void endUndoCommand();

    /**
     * Returns the model shown in the dialog.
     */
    QgsProcessingModelAlgorithm *model();

    /**
     * Sets the \a model shown in the dialog.
     *
     * Ownership of \a model is transferred to the dialog.
     */
    void setModel( QgsProcessingModelAlgorithm *model SIP_TRANSFER );

    /**
     * Loads a model into the designer from the specified file \a path.
     */
    void loadModel( const QString &path );

    /**
     * Sets the related \a scene.
     */
    void setModelScene( QgsModelGraphicsScene *scene SIP_TRANSFER );

    /**
     * Save action.
     *
     * \since QGIS 3.24
     */
    enum class SaveAction
    {
      SaveAsFile, //!< Save model as a file
      SaveInProject, //!< Save model into project
    };

  public slots:

    /**
     * Raise, unminimize and activate this window.
     *
     * \since QGIS 3.24
     */
    void activate();

  protected:

    // cppcheck-suppress pureVirtualCall
    virtual void repaintModel( bool showControls = true ) = 0;
    virtual void addAlgorithm( const QString &algorithmId, const QPointF &pos ) = 0;
    virtual void addInput( const QString &inputId, const QPointF &pos ) = 0;
    virtual void exportAsScriptAlgorithm() = 0;
    // cppcheck-suppress pureVirtualCall
    virtual bool saveModel( bool saveAs = false ) = 0;

    QToolBar *toolbar() { return mToolbar; }
    QAction *actionOpen() { return mActionOpen; }
    QAction *actionSaveInProject() { return mActionSaveInProject; }
    QAction *actionRun() { return mActionRun; }
    QgsMessageBar *messageBar() { return mMessageBar; }
    QGraphicsView *view() { return mView; }

    void setDirty( bool dirty );

    /**
     * Checks if the model can current be saved, and returns TRUE if it can.
     */
    bool validateSave( SaveAction action );

    /**
     * Checks if there are unsaved changes in the model, and if so, prompts the user to save them.
     *
     * Returns FALSE if the cancel option was selected
     */
    bool checkForUnsavedChanges();

    /**
     * Sets the results of child algorithms for the last run of the model through the designer window.
     */
    void setLastRunChildAlgorithmResults( const QVariantMap &results );

    /**
     * Sets the inputs for child algorithms for the last run of the model through the designer window.
     */
    void setLastRunChildAlgorithmInputs( const QVariantMap &inputs );

    /**
     * Sets the model \a name.
     *
     * Updates both the name text edit and the model name itself.
     *
     * \since QGIS 3.24
     */
    void setModelName( const QString &name );

  private slots:
    void zoomIn();
    void zoomOut();
    void zoomActual();
    void zoomFull();
    void newModel();
    void exportToImage();
    void exportToPdf();
    void exportToSvg();
    void exportAsPython();
    void toggleComments( bool show );
    void updateWindowTitle();
    void deleteSelected();
    void populateZoomToMenu();
    void validate();
    void reorderInputs();
    void setPanelVisibility( bool hidden );
    void editHelp();

  private:

    enum UndoCommand
    {
      NameChanged = 1,
      GroupChanged
    };

    std::unique_ptr< QgsProcessingModelAlgorithm > mModel;

    QgsMessageBar *mMessageBar = nullptr;
    QgsModelerToolboxModel *mAlgorithmsModel = nullptr;

    QActionGroup *mToolsActionGroup = nullptr;

    QgsModelViewToolPan *mPanTool = nullptr;
    QgsModelViewToolSelect *mSelectTool = nullptr;
    QgsModelGraphicsScene *mScene = nullptr;

    bool mHasChanged = false;
    QUndoStack *mUndoStack = nullptr;
    std::unique_ptr< QgsModelUndoCommand > mActiveCommand;

    QAction *mUndoAction = nullptr;
    QAction *mRedoAction = nullptr;
    QUndoView *mUndoView = nullptr;
    QgsDockWidget *mUndoDock = nullptr;

    QMenu *mGroupMenu = nullptr;

    QAction *mActionCut = nullptr;
    QAction *mActionCopy = nullptr;
    QAction *mActionPaste = nullptr;
    int mBlockUndoCommands = 0;
    int mIgnoreUndoStackChanges = 0;

    QString mTitle;

    int mBlockRepaints = 0;

    QVariantMap mChildResults;
    QVariantMap mChildInputs;

    bool isDirty() const;

    void fillInputsTree();
    void updateVariablesGui();

    struct PanelStatus
    {
      PanelStatus( bool visible = true, bool active = false )
        : isVisible( visible )
        , isActive( active )
      {}
      bool isVisible;
      bool isActive;
    };
    QMap< QString, PanelStatus > mPanelStatus;
};



class GUI_EXPORT QgsModelChildDependenciesWidget : public QWidget
{
    Q_OBJECT

  public:

    QgsModelChildDependenciesWidget( QWidget *parent, QgsProcessingModelAlgorithm *model, const QString &childId );
    QList< QgsProcessingModelChildDependency > value() const { return mValue; }
    void setValue( const QList< QgsProcessingModelChildDependency >  &value );
  private slots:

    void showDialog();

  private:

    void updateSummaryText();

    QLineEdit *mLineEdit = nullptr;
    QToolButton *mToolButton = nullptr;

    QgsProcessingModelAlgorithm *mModel = nullptr;
    QString mChildId;

    QList< QgsProcessingModelChildDependency >  mValue;

    friend class TestProcessingGui;
};
///@endcond

#endif // QGSMODELDESIGNERDIALOG_H
