class QgsSpinBox : QSpinBox
{
%TypeHeaderCode
#include <qgsspinbox.h>
%End

  public:
    enum ClearValueMode
    {
      MinimumValue,
      MaximumValue,
      CustomValue
    };

    explicit QgsSpinBox( QWidget *parent /TransferThis/ = 0 );

    //! determines if the widget will show a clear button
    //! @note the clear button will set the widget to its minimum value
    void setShowClearButton( const bool showClearButton );
    bool showClearButton() const;

    /**Sets if the widget will allow entry of simple expressions, which are
     * evaluated and then discarded.
     * @param enabled set to true to allow expression entry
     * @note added in QGIS 2.7
     */
    void setExpressionsEnabled( const bool enabled );
    /**Returns whether the widget will allow entry of simple expressions, which are
     * evaluated and then discarded.
     * @returns true if spin box allows expression entry
     * @note added in QGIS 2.7
     */
    bool expressionsEnabled() const;

    //! Set the current value to the value defined by the clear value.
    virtual void clear();

    /**
     * @brief setClearValue defines the clear value for the widget and will automatically set the clear value mode to CustomValue
     * @param customValue defines the numerical value used as the clear value
     * @param clearValueText is the text displayed when the spin box is at the clear value. If not specified, no special value text is used.
     */
    void setClearValue( int customValue, QString clearValueText = QString() );
    /**
     * @brief setClearValueMode defines if the clear value should be the minimum or maximum values of the widget or a custom value
     * @param mode mode to user for clear value
     * @param clearValueText is the text displayed when the spin box is at the clear value. If not specified, no special value text is used.
     */
    void setClearValueMode( ClearValueMode mode, QString clearValueText = QString() );

    //! returns the value used when clear() is called.
    int clearValue() const;

    virtual int valueFromText( const QString & text ) const;
    virtual QValidator::State validate( QString & input, int & pos ) const;

  protected:
    virtual void resizeEvent( QResizeEvent* event );
    virtual void changeEvent( QEvent* event );
};
