/** \ingroup gui
 * \class QgsVariableEditorWidget
 * A tree based widget for editing expression context scope variables. The widget allows editing
 * variables from a QgsExpressionContextScope, and can optionally also show inherited
 * variables from a QgsExpressionContext.
 * \note added in QGIS 2.12
 */

class QgsVariableEditorWidget : QWidget
{
%TypeHeaderCode
#include <qgsvariableeditorwidget.h>
%End

  public:

    /** Constructor for QgsVariableEditorWidget.
     * @param parent parent widget
     */
    QgsVariableEditorWidget( QWidget *parent /TransferThis/ = 0 );

    ~QgsVariableEditorWidget();

    /** Overwrites the QgsExpressionContext for the widget. Setting a context
     * allows the widget to show all inherited variables for the context,
     * and highlight any overridden variables within scopes.
     * @param context expression context
     * @see context()
     */
    void setContext( QgsExpressionContext* context );

    /** Returns the current expression context for the widget. QgsVariableEditorWidget widgets
     * are created with an empty context by default.
     * @see setContext()
     */
    QgsExpressionContext* context() const;

    /** Reloads all scopes from the editor's current context. This method should be called
     * after adding or removing scopes from the attached context.
     * @see context()
     */
    void reloadContext();

    /** Sets the editable scope for the widget. Only variables from the editable scope can
     * be modified by users.
     * @param scopeIndex index of current editable scope. Set to -1 to disable
     * editing and make the widget read-only.
     * @see editableScope()
     */
    void setEditableScopeIndex( int scopeIndex );

    /** Returns the current editable scope for the widget.
     * @returns editable scope, or 0 if no editable scope is set
     * @see setEditableScopeIndex()
     */
    QgsExpressionContextScope* editableScope() const;

    /** Sets the setting group for the widget. QgsVariableEditorWidget widgets with
     * the same setting group will synchronise their settings, eg the size
     * of columns in the tree widget.
     * @param group setting group
     * @see settingGroup()
     */
    void setSettingGroup( const QString &group );

    /** Returns the setting group for the widget. QgsVariableEditorWidget widgets with
     * the same setting group will synchronise their settings, eg the size
     * of columns in the tree widget.
     * @returns setting group name
     * @see setSettingGroup()
     */
    QString settingGroup() const;

    /** Returns a map variables set within the editable scope. Read only variables are not
     * returned. This method can be used to retrieve the variables edited an added by
     * users via the widget.
     */
    QgsStringMap variablesInActiveScope() const;

  signals:

    /** Emitted when the user has modified a scope using the widget.
     */
    void scopeChanged();

  protected:

    void showEvent( QShowEvent *event );

};
