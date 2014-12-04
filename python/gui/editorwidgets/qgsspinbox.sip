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

    //! Set the current value to the value defined by the clear value.
    virtual void clear();

    /**
     * @brief setClearValue defines the clear value as a custom value and will automatically set the clear value mode to CustomValue
     * @param defines the numerical value used as the clear value
     * @param clearValueText is the text displayed when the spin box is at the clear value. If not specified, no special value text is used.
     */
    void setClearValue( int customValue, QString clearValueText = QString() );
    /**
     * @brief setClearValueMode defines if the clear value should be the minimum or maximum values of the widget or a custom value
     * @param clearValueText is the text displayed when the spin box is at the clear value. If not specified, no special value text is used.
     */
    void setClearValueMode( ClearValueMode mode, QString clearValueText = QString() );

    //! returns the value used when clear() is called.
    int clearValue();

  protected:
    virtual void resizeEvent( QResizeEvent* event );
    virtual void changeEvent( QEvent* event );
};
