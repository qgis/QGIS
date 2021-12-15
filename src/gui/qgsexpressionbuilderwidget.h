/***************************************************************************
    qgisexpressionbuilderwidget.h - A generic expression string builder widget.
     --------------------------------------
    Date                 :  29-May-2011
    Copyright            : (C) 2011 by Nathan Woodrow
    Email                : woodrow.nathan at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSEXPRESSIONBUILDER_H
#define QGSEXPRESSIONBUILDER_H

#include <QWidget>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>

#include "ui_qgsexpressionbuilder.h"

#include "qgis_sip.h"
#include "qgis_gui.h"
#include "qgsexpressioncontext.h"
#include "qgsexpression.h"
#include "qgsexpressiontreeview.h"


class QgsFields;
class QgsExpressionHighlighter;
class QgsRelation;


/**
 * \ingroup gui
 * \brief A reusable widget that can be used to build a expression string.
  * See QgsExpressionBuilderDialog for example of usage.
  */
class GUI_EXPORT QgsExpressionBuilderWidget : public QWidget, private Ui::QgsExpressionBuilderWidgetBase
{
    Q_OBJECT
  public:

    /**
     * Flag to determine what should be loaded
     * \since QGIS 3.14
     */
    enum Flag
    {
      LoadNothing = 0, //!< Do not load anything
      LoadRecent = 1 << 1, //!< Load recent expressions given the collection key
      LoadUserExpressions = 1 << 2, //!< Load user expressions
      LoadAll = LoadRecent | LoadUserExpressions, //!< Load everything
    };
    Q_DECLARE_FLAGS( Flags, Flag )
    Q_FLAG( Flag )


    /**
     * Create a new expression builder widget with an optional parent.
     */
    QgsExpressionBuilderWidget( QWidget *parent SIP_TRANSFERTHIS = nullptr );
    ~QgsExpressionBuilderWidget() override;

    /**
     * Initialize without any layer
     * \since QGIS 3.14
     */
    void init( const QgsExpressionContext &context = QgsExpressionContext(), const QString &recentCollection = QStringLiteral( "generic" ), QgsExpressionBuilderWidget::Flags flags = LoadAll );

    /**
     * Initialize with a layer
     * \since QGIS 3.14
     */
    void initWithLayer( QgsVectorLayer *layer, const QgsExpressionContext &context = QgsExpressionContext(), const QString &recentCollection = QStringLiteral( "generic" ), QgsExpressionBuilderWidget::Flags flags = LoadAll );

    /**
     * Initialize with given fields without any layer
     * \since QGIS 3.14
     */
    void initWithFields( const QgsFields &fields, const QgsExpressionContext &context = QgsExpressionContext(), const QString &recentCollection = QStringLiteral( "generic" ), QgsExpressionBuilderWidget::Flags flags = LoadAll );

    /**
     * Sets layer in order to get the fields and values
     * \note this needs to be called before calling loadFieldNames().
     */
    void setLayer( QgsVectorLayer *layer );

    /**
     * Returns the current layer or a nullptr.
     */
    QgsVectorLayer *layer() const;

    //! \deprecated since QGIS 3.14 this is now done automatically
    Q_DECL_DEPRECATED void loadFieldNames() {} SIP_DEPRECATED

    //! \deprecated since QGIS 3.14 use expressionTree()->loadFieldNames() instead
    Q_DECL_DEPRECATED void loadFieldNames( const QgsFields &fields ) {mExpressionTreeView->loadFieldNames( fields );} SIP_DEPRECATED

    /**
     * Loads field names and values from the specified map.
     *  \since QGIS 2.12
     * \deprecated since QGIS 3.14 this will not do anything, use setLayer() instead
     */
    Q_DECL_DEPRECATED void loadFieldsAndValues( const QMap<QString, QStringList> &fieldValues ) SIP_DEPRECATED;

    //! Sets geometry calculator used in distance/area calculations.
    void setGeomCalculator( const QgsDistanceArea &da );

    /**
     * Gets the expression string that has been set in the expression area.
     * \returns The expression as a string.
     */
    QString expressionText();

    //! Sets the expression string for the widget
    void setExpressionText( const QString &expression );

    /**
     * The set expected format string. This is pure text format and no expression validation
     * is done against it.
     * \returns The expected value format.
     */
    QString expectedOutputFormat();

    /**
     * The set expected format string. This is pure text format and no expression validation
     * is done against it.
     * \param expected The expected value format for the expression.
     * \note Only a UI hint and not used for expression validation.
     */
    void setExpectedOutputFormat( const QString &expected );

    /**
     * Returns the expression context for the widget. The context is used for the expression
     * preview result and for populating the list of available functions and variables.
     * \see setExpressionContext
     * \since QGIS 2.12
     */
    QgsExpressionContext expressionContext() const { return mExpressionContext; }

    /**
     * Sets the expression context for the widget. The context is used for the expression
     * preview result and to populate the list of available functions and variables.
     * \param context expression context
     * \see expressionContext
     * \since QGIS 2.12
     */
    void setExpressionContext( const QgsExpressionContext &context );

    //! Returns if the expression is valid
    bool isExpressionValid();

    /**
     * Adds the current expression to the given \a collection.
     * By default it is saved to the collection "generic".
     * \deprecated since QGIS 3.14 use expressionTree()->saveRecent() instead
     */
    Q_DECL_DEPRECATED void saveToRecent( const QString &collection = "generic" ) SIP_DEPRECATED;

    /**
     * Loads the recent expressions from the given \a collection.
     * By default it is loaded from the collection "generic".
     * \deprecated since QGIS 3.14 use expressionTree()->loadRecent() instead
     */
    Q_DECL_DEPRECATED void loadRecent( const QString &collection = QStringLiteral( "generic" ) )SIP_DEPRECATED ;

    /**
     * Returns the expression tree
     * \since QGIS 3.14
     */
    QgsExpressionTreeView *expressionTree() const;

    /**
     * Loads the user expressions.
     * \deprecated since QGIS 3.14 use expressionTree()->loadUserExpressions() instead
     * \since QGIS 3.12
     */
    Q_DECL_DEPRECATED void loadUserExpressions() SIP_DEPRECATED;

    /**
     * Stores the user \a expression with given \a label and \a helpText.
     * \deprecated since QGIS 3.14 use expressionTree()->saveToUserExpressions() instead
     * \since QGIS 3.12
     */
    Q_DECL_DEPRECATED void saveToUserExpressions( const QString &label, const QString &expression, const QString &helpText ) SIP_DEPRECATED;

    /**
     * Removes the expression \a label from the user stored expressions.
     * \deprecated since QGIS 3.14 use expressionTree()->removeFromUserExpressions() instead
     * \since QGIS 3.12
     */
    Q_DECL_DEPRECATED void removeFromUserExpressions( const QString &label ) SIP_DEPRECATED;

    /**
     * Creates a new file in the function editor
     */
    void newFunctionFile( const QString &fileName = "scratch" );

    /**
     * Saves the current function editor text to the given file.
     */
    void saveFunctionFile( QString fileName );

    /**
     * Loads code from the given file into the function editor
     */
    void loadCodeFromFile( QString path );

    /**
     * Loads code into the function editor
     */
    void loadFunctionCode( const QString &code );

    /**
     * Updates the list of function files found at the given path
     */
    void updateFunctionFileList( const QString &path );

    /**
     * Returns a pointer to the dialog's function item model.
     * This method is exposed for testing purposes only - it should not be used to modify the model.
     * \since QGIS 3.0
     * \deprecated since QGIS 3.14
     */
    Q_DECL_DEPRECATED QStandardItemModel *model() SIP_DEPRECATED;

    /**
     * Returns the project currently associated with the widget.
     * \see setProject()
     * \since QGIS 3.0
     */
    QgsProject *project();

    /**
     * Sets the \a project currently associated with the widget. This
     * controls which layers and relations and other project-specific items are shown in the widget.
     * \see project()
     * \since QGIS 3.0
     */
    void setProject( QgsProject *project );

    /**
     * Will be set to TRUE if the current expression text reported an eval error
     * with the context.
     *
     * \since QGIS 3.0
     */
    bool evalError() const;

    /**
     * Will be set to TRUE if the current expression text reports a parser error
     * with the context.
     *
     * \since QGIS 3.0
     */
    bool parserError() const;

    /**
     * Sets whether the expression preview is visible.
     *
     * \since QGIS 3.22
     */
    void setExpressionPreviewVisible( bool isVisible );

  public slots:

    /**
     * Load sample values into the sample value area.
     * Including available values, in case the formatter can
     * provide them (eg. RelationReference).
     */
    void loadSampleValues();

    /**
     * Load all unique values from the set layer into the sample area.
     * Including all available values, in case the formatter can
     * provide them (eg. RelationReference).
     */
    void loadAllValues();

    /**
     * Load used sample values into the sample value area.
     * Only the used ones. Without available values, even if the
     * formatter can provide them (eg. RelationReference).
     *
     * \since QGIS 3.12
     */
    void loadSampleUsedValues();

    /**
     * Load all unique values from the set layer into the sample area.
     * Only the used ones. Without available values, even if the
     * formatter can provide them (eg. RelationReference).
     *
     * \since QGIS 3.12
     */
    void loadAllUsedValues();

    /**
     * Auto save the current Python function code.
     */
    void autosave();

    /**
     * Enabled or disable auto saving. When enabled Python scripts will be auto saved
     * when text changes.
     * \param enabled TRUE to enable auto saving.
     */
    void setAutoSave( bool enabled ) { mAutoSave = enabled; }

    /**
     * Adds the current expressions to the stored user expressions.
     * \since QGIS 3.12
     */
    void storeCurrentUserExpression( );

    /**
     * Removes the selected expression from the stored user expressions,
     * the selected expression must be a user stored expression.
     * \since QGIS 3.12
     */
    void removeSelectedUserExpression( );

    /**
     * Edits the selected expression from the stored user expressions,
     * the selected expression must be a user stored expression.
     * \since QGIS 3.14
     */
    void editSelectedUserExpression();

    /**
     * Returns the list of expression items matching a \a label.
     * \since QGIS 3.12
     * \deprecated since QGIS 3.14 use expressionTree()->findExpressions instead
     */
    const QList<QgsExpressionItem *> findExpressions( const QString &label );


  private slots:
    void indicatorClicked( int line, int index, Qt::KeyboardModifiers state );
    void onExpressionParsed( bool state );
    void expressionTreeItemChanged( QgsExpressionItem *item );
    void operatorButtonClicked();
    void btnRun_pressed();
    void btnNewFile_pressed();
    void btnRemoveFile_pressed();

    /**
     * Display a file dialog to choose where to store the exported expressions JSON file
     * and saves them to the selected destination.
     * \since QGIS 3.14
     */
    void exportUserExpressions_pressed();

    /**
     * Display a file dialog to choose where to load the expression JSON file from
     * and adds them to user expressions group.
     * \since QGIS 3.14
     */
    void importUserExpressions_pressed();
    void cmbFileNames_currentItemChanged( QListWidgetItem *item, QListWidgetItem *lastitem );
    void insertExpressionText( const QString &text );
    void txtExpressionString_textChanged();
    void txtSearchEditValues_textChanged();
    void mValuesListView_doubleClicked( const QModelIndex &index );
    void txtPython_textChanged();

  signals:

    /**
     * Emitted when the user changes the expression in the widget.
     * Users of this widget should connect to this signal to decide if to let the user
     * continue.
     * \param isValid Is TRUE if the expression the user has typed is valid.
     */
    void expressionParsed( bool isValid );

    /**
     * Will be set to TRUE if the current expression text reported an eval error
     * with the context.
     *
     * \since QGIS 3.0
     */
    void evalErrorChanged();

    /**
     * Will be set to TRUE if the current expression text reported a parser error
     * with the context.
     *
     * \since QGIS 3.0
     */
    void parserErrorChanged();

  protected:
    void showEvent( QShowEvent *e ) override;

  private:
    class ExpressionTreeMenuProvider : public QgsExpressionTreeView::MenuProvider
    {
      public:
        ExpressionTreeMenuProvider( QgsExpressionBuilderWidget *expressionBuilderWidget )
          : QgsExpressionTreeView::MenuProvider()
          , mExpressionBuilderWidget( expressionBuilderWidget ) {}

        QMenu *createContextMenu( QgsExpressionItem *item ) override;

      private:
        QgsExpressionBuilderWidget *mExpressionBuilderWidget;
    };

    int FUNCTION_MARKER_ID = 25;

    void createErrorMarkers( const QList<QgsExpression::ParserError> &errors );
    void createMarkers( const QgsExpressionNode *node );
    void clearFunctionMarkers();
    void clearErrors();
    void runPythonCode( const QString &code );
    QgsVectorLayer *contextLayer( const QgsExpressionItem *item ) const;
    void fillFieldValues( const QString &fieldName, QgsVectorLayer *layer, int countLimit, bool forceUsedValues = false );
    QString getFunctionHelp( QgsExpressionFunction *function );
    QString loadFunctionHelp( QgsExpressionItem *functionName );
    QString helpStylesheet() const;

    // To be called whenever expression context has been updated
    void expressionContextUpdated();

    // Will hold items with
    // * a display string that matches the represented field values
    // * custom data in Qt::UserRole + 1 that contains a ready to use expression literal ('quoted string' or NULL or a plain number )
    std::unique_ptr<QStandardItemModel> mValuesModel;
    std::unique_ptr<QSortFilterProxyModel> mProxyValues;

    ExpressionTreeMenuProvider *mExpressionTreeMenuProvider = nullptr;

    bool mAutoSave = true;
    QString mFunctionsPath;
    QgsVectorLayer *mLayer = nullptr;
    QgsExpressionHighlighter *highlighter = nullptr;
    bool mExpressionValid = false;
    QgsExpressionContext mExpressionContext;
    QPointer< QgsProject > mProject;

    // Translated name of the user expressions group
    QString mUserExpressionsGroupName;
};

// clazy:excludeall=qstring-allocations

#endif // QGSEXPRESSIONBUILDER_H
