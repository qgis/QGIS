/** \ingroup gui
 * \class QgsMultiEditToolButton
 * A tool button widget which is displayed next to editor widgets in attribute forms, and
 * allows for controlling how the widget behaves and interacts with the form while in multi
 * edit mode.
 * \note Added in version 2.16
 */
class QgsMultiEditToolButton : QToolButton
{
%TypeHeaderCode
#include <qgsmultiedittoolbutton.h>
%End
  public:

    //! Button states
    enum State
    {
      Default, /*!< Default state, all features have same value for widget */
      MixedValues, /*!< Mixed state, some features have different values for the widget */
      Changed, /*!< Value for widget has changed but changes have not yet been committed */
    };

    /** Constructor for QgsMultiEditToolButton.
     * @param parent parent object
     */
    explicit QgsMultiEditToolButton( QWidget *parent /TransferThis/ = nullptr );

    /** Returns the current displayed state of the button.
     */
    State state() const;

    /** Sets the field associated with this button. This is used to customise the widget menu
     * and tooltips to match the field properties.
     * @param field associated field
     */
    void setField( const QgsField& field );

  public slots:

    /** Sets whether the associated field contains mixed values.
     * @param mixed whether field values are mixed
     * @see isMixed()
     * @see setIsChanged()
     * @see resetChanges()
     */
    void setIsMixed( bool mixed );

    /** Sets whether the associated field has changed.
     * @param changed whether field has changed
     * @see isChanged()
     * @see setIsMixed()
     * @see resetChanges()
     */
    void setIsChanged( bool changed );

    /** Resets the changed state for the field.
     * @see setIsMixed()
     * @see setIsChanged()
     * @see changesCommitted()
     */
    void resetChanges();

    /** Called when field values have been changed and field now contains all the same values.
     * @see resetChanges()
     */
    void changesCommitted();

  signals:

    //! Emitted when the "set field value for all features" option is selected.
    void setFieldValueTriggered();

    //! Emitted when the "reset to original values" option is selected.
    void resetFieldValueTriggered();
};
