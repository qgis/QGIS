/** \class QgsCheckBoxDelegate
 * \ingroup gui
 * QComboBox subclass which allows selecting multiple items.
 * @note added in QGIS 3.0
 **/
class QgsCheckableComboBox : QComboBox
{
%TypeHeaderCode
#include <qgscheckablecombobox.h>
%End

  public:

    /** Constructor for QgsCheckableComboBox.
     * @param parent parent object
     */
    QgsCheckableComboBox( QWidget *parent = nullptr );

    /** Returns separator used to separate items in the display text.
     * @see setSeparator()
     */
    QString separator() const;

    /** Set separator used to separate items in the display text.
     * @param separator separator to use
     * @see separator()
     */
    void setSeparator( const QString &separator );

    /** Returns default text which will be displayed in the widget
     * when no items selected.
     * @see setDefaultText()
     */
    QString defaultText() const;

    /** Set default text which will be displayed in the widget when
     * no items selected.
     * @param text default text
     * @see defaultText()
     */
    void setDefaultText( const QString &text );

    /** Returns currently checked items.
     * @see setCheckedItems()
     */
    QStringList checkedItems() const;

    /** Returns the checked state of the item identified by index
     * @param index item index
     * @see setItemCheckState()
     * @see toggleItemCheckState()
     */
    Qt::CheckState itemCheckState( int index ) const;

    /** Sets the item check state to state
     * @param index item index
     * @param state check state
     * @see itemCheckState()
     * @see toggleItemCheckState()
     */
    void setItemCheckState( int index, Qt::CheckState state );

    /** Toggles the item check state
     * @param index item index
     * @see itemCheckState()
     * @see setItemCheckState()
     */
    void toggleItemCheckState( int index );

    /** Hides the list of items in the combobox if it is currently
     * visible and resets the internal state.
     */
    virtual void hidePopup();

  signals:

    /** This signal is emitted whenever the checked items list changed.
     */
    void checkedItemsChanged( const QStringList &items );

  public slots:

    /** Set items which should be checked/selected.
     * @param items items to select
     * @see checkedItems()
     */
    void setCheckedItems( const QStringList &items );

  protected:
    virtual void resizeEvent( QResizeEvent *event );
};
