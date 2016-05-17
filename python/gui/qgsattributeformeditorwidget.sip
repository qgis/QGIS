/** \ingroup gui
 * \class QgsAttributeFormEditorWidget
 * A widget consisting of both an editor widget and additional widgets for controlling the behaviour
 * of the editor widget depending on a number of possible modes. For instance, if the parent attribute
 * form is in the multi edit mode, this widget will show both the editor widget and a tool button for
 * controlling the multi edit results.
 * \note Added in version 2.16
 */
class QgsAttributeFormEditorWidget : QWidget
{
%TypeHeaderCode
#include <qgsattributeformeditorwidget.h>
%End

  public:

    //! Widget modes
    enum Mode
    {
      DefaultMode, /*!< Default mode, only the editor widget is shown */
      MultiEditMode, /*!< Multi edit mode, both the editor widget and a QgsMultiEditToolButton is shown */
      // TODO: SearchMode, /*!< Layer search/filter mode */
    };

    /** Constructor for QgsAttributeFormEditorWidget.
     * @param editorWidget associated editor widget wrapper
     * @param form parent attribute form
     */
    explicit QgsAttributeFormEditorWidget( QgsEditorWidgetWrapper* editorWidget, QgsAttributeForm* form /TransferThis/ );

    ~QgsAttributeFormEditorWidget();

    /** Sets the current mode for the widget. The widget will adapt its state and visible widgets to
     * reflect the updated mode. Eg, showing multi edit tool buttons if the mode is set to MultiEditMode.
     * @param mode widget mode
     * @see mode()
     */
    void setMode( Mode mode );

    /** Returns the current mode for the widget.
     * @see setMode()
     */
    Mode mode() const;

    /** Resets the widget to an initial value.
     * @param initialValue initial value to show in widget
     * @param mixedValues set to true to initially show the mixed values state
     */
    void initialize( const QVariant& initialValue, bool mixedValues = false );

    /** Returns true if the widget's value has been changed since it was initialized.
     * @see initialize()
     */
    bool hasChanged() const;

    /** Returns the current value of the attached editor widget.
     */
    QVariant currentValue() const;

  public slots:

    /** Sets whether the widget should be displayed in a "mixed values" mode.
     * @param mixed set to true to show in a mixed values state
     */
    void setIsMixed( bool mixed );

    /** Called when field values have been committed;
     */
    void changesCommitted();

  signals:

    //! Emitted when the widget's value changes
    //! @param value new widget value
    void valueChanged( const QVariant& value );


};
