/** \class QgsSubstitutionListWidget
 * \ingroup gui
 * A widget which allows users to specify a list of substitutions to apply to a string, with
 * options for exporting and importing substitution lists.
 * \note added in QGIS 3.0
 * \see QgsSubstitutionListDialog
 */

class QgsSubstitutionListWidget : public QgsPanelWidget
{
%TypeHeaderCode
  #include <qgssubstitutionlistwidget.h>
%End

  public:

     /** Constructor for QgsSubstitutionListWidget.
     * @param parent parent widget
     */
    QgsSubstitutionListWidget( QWidget* parent /TransferThis/ = nullptr );

    /** Sets the list of substitutions to show in the widget.
     * @param substitutions substitution list
     * @see substitutions()
     */
    void setSubstitutions( const QgsStringReplacementCollection& substitutions );

    /** Returns the list of substitutions currently defined by the widget.
     * @see setSubstitutions()
     */
    QgsStringReplacementCollection substitutions() const;

  signals:

    //! Emitted when the substitution definitions change.
    void substitutionsChanged( const QgsStringReplacementCollection& substitutions );

};



/** \class QgsSubstitutionListDialog
 * \ingroup gui
 * A dialog which allows users to specify a list of substitutions to apply to a string, with
 * options for exporting and importing substitution lists.
 * \see QgsSubstitutionListWidget
*/

class QgsSubstitutionListDialog : public QDialog
{
%TypeHeaderCode
  #include <qgssubstitutionlistwidget.h>
%End

  public:

    /** Constructor for QgsSubstitutionListDialog.
     * @param parent parent widget
     */
    QgsSubstitutionListDialog( QWidget* parent /TransferThis/ = nullptr );

    /** Sets the list of substitutions to show in the dialog.
     * @param substitutions substitution list
     * @see substitutions()
     */
    void setSubstitutions( const QgsStringReplacementCollection& substitutions );

    /** Returns the list of substitutions currently defined by the dialog.
     * @see setSubstitutions()
     */
    QgsStringReplacementCollection substitutions() const;

};
