/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/attributetable/qgsdualview.h                                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsDualView : QStackedWidget
{
%Docstring
This widget is used to show the attributes of a set of features of a QgsVectorLayer.
The attributes can be edited.
It supports two different layouts: the table layout, in which the attributes for the features
are shown in a table and the editor layout, where the features are shown as a selectable list
and the attributes for the currently selected feature are shown in a form.
%End

%TypeHeaderCode
#include "qgsdualview.h"
%End
  public:

    enum ViewMode
    {

      AttributeTable,

      AttributeEditor
    };


    explicit QgsDualView( QWidget *parent /TransferThis/ = 0 );
%Docstring
 Constructor

:param parent:  The parent widget
%End

    void init( QgsVectorLayer *layer, QgsMapCanvas *mapCanvas, const QgsFeatureRequest &request = QgsFeatureRequest(), const QgsAttributeEditorContext &context = QgsAttributeEditorContext(),
               bool loadFeatures = true );
%Docstring
Has to be called to initialize the dual view.

:param layer:      The layer which should be used to fetch features
:param mapCanvas:  The mapCanvas (used for the FilterMode
QgsAttributeTableFilterModel.ShowVisible)
:param request:    Use a modified request to limit the shown features
:param context:    The context in which this view is shown
:param loadFeatures: whether to initially load all features into the view. If set to
false, limited features can later be loaded using setFilterMode()
%End

    void setView( ViewMode view );
%Docstring
Change the current view mode.

:param view: The view mode to set

.. seealso:: :py:func:`view`
%End

    ViewMode view() const;
%Docstring
Returns the current view mode.

.. seealso:: :py:func:`setView`

.. versionadded:: 2.16
%End

    void setFilterMode( QgsAttributeTableFilterModel::FilterMode filterMode );
%Docstring
Set the filter mode

:param filterMode:
%End

    QgsAttributeTableFilterModel::FilterMode filterMode();
%Docstring
Get the filter mode

:return: the filter mode
%End

    void setSelectedOnTop( bool selectedOnTop );
%Docstring
Toggle the selectedOnTop flag. If enabled, selected features will be moved to top.

:param selectedOnTop: True: Show selected features on top.
False: Use defined sorting column.
%End

    int featureCount();
%Docstring
Returns the number of features on the layer.

:return: Number of features
%End

    int filteredFeatureCount();
%Docstring
Returns the number of features which are currently visible, according to the
filter restrictions

:return: Number of features
%End

    void setFilteredFeatures( const QgsFeatureIds &filteredFeatures );
%Docstring
Set a list of currently visible features

:param filteredFeatures:  A list of feature ids
%End

    QgsFeatureIds filteredFeatures();
%Docstring
Get a list of currently visible feature ids.
%End

    QgsAttributeTableModel *masterModel() const;
%Docstring
Returns the model which has the information about all features (not only filtered)

:return: The master model
%End

    void setRequest( const QgsFeatureRequest &request );
%Docstring
Set the request

:param request: The request
%End

    void setFeatureSelectionManager( QgsIFeatureSelectionManager *featureSelectionManager );
%Docstring
Set the feature selection model

:param featureSelectionManager: the feature selection model
%End

    QgsAttributeTableView *tableView();
%Docstring
Returns the table view

:return: The table view
%End

    void setAttributeTableConfig( const QgsAttributeTableConfig &config );
%Docstring
Set the attribute table config which should be used to control
the appearance of the attribute table.
%End

    void setSortExpression( const QString &sortExpression, Qt::SortOrder sortOrder = Qt::AscendingOrder );
%Docstring
Set the expression used for sorting the table and feature list.
%End

    QString sortExpression() const;
%Docstring
Get the expression used for sorting the table and feature list.
%End

  public slots:

    void setCurrentEditSelection( const QgsFeatureIds &fids );
%Docstring
 Set the current edit selection in the AttributeEditor mode.

:param fids:   A list of edited features (Currently only one at a time is supported)
%End

    bool saveEditChanges();
%Docstring
 saveEditChanges

:return: true if the saving was OK. false is possible due to connected
validation logic.
%End

    void openConditionalStyles();

    void setMultiEditEnabled( bool enabled );
%Docstring
Sets whether multi edit mode is enabled.

.. versionadded:: 2.16
%End

    void toggleSearchMode( bool enabled );
%Docstring
Toggles whether search mode should be enabled in the form.

:param enabled: set to true to switch on search mode

.. versionadded:: 2.16
%End

    void copyCellContent() const;
%Docstring
Copy the content of the selected cell in the clipboard.

.. versionadded:: 1.16
%End

    void cancelProgress( );
%Docstring
Cancel the progress dialog (if any)

.. versionadded:: 3.0
%End

  signals:

    void displayExpressionChanged( const QString &expression );
%Docstring
Is emitted, whenever the display expression is successfully changed

:param expression: The expression that was applied
%End

    void filterChanged();
%Docstring
Is emitted, whenever the filter changes
%End

    void filterExpressionSet( const QString &expression, QgsAttributeForm::FilterType type );
%Docstring
Is emitted when a filter expression is set using the view.

:param expression: filter expression
:param type: filter type

.. versionadded:: 2.16
%End

    void formModeChanged( QgsAttributeForm::Mode mode );
%Docstring
Emitted when the form changes mode.

:param mode: new mode
%End

    void showContextMenuExternally( QgsActionMenu *menu, const QgsFeatureId fid );
%Docstring
Emitted when selecting context menu on the feature list to create the context menu individually

:param menu: context menu
:param fid: feature id of the selected feature
%End

  protected:
    virtual void hideEvent( QHideEvent *event );


};

class QgsAttributeTableAction : QAction
{

%TypeHeaderCode
#include "qgsdualview.h"
%End
  public:

    QgsAttributeTableAction( const QString &name, QgsDualView *dualView, const QUuid &action, const QModelIndex &fieldIdx );
%Docstring
Create a new attribute table action.

.. versionadded:: 3.0
%End

  public slots:
    void execute();
    void featureForm();

};

class QgsAttributeTableMapLayerAction : QAction
{

%TypeHeaderCode
#include "qgsdualview.h"
%End
  public:
    QgsAttributeTableMapLayerAction( const QString &name, QgsDualView *dualView, QgsMapLayerAction *action, const QModelIndex &fieldIdx );

  public slots:
    void execute();

};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/attributetable/qgsdualview.h                                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
