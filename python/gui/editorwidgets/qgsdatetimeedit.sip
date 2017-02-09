
class QgsDateTimeEdit : QDateTimeEdit
{

%TypeHeaderCode
#include "qgsdatetimeedit.h"
%End


  public:
    explicit QgsDateTimeEdit( QWidget *parent /TransferThis/ = 0 );

    //! determines if the widget allows setting null date/time.
    void setAllowNull( bool allowNull );
    bool allowNull() const;

    /**
     * @brief setDateTime set the date time in the widget and handles null date times.
     * @note since QDateTimeEdit::setDateTime() is not virtual, setDateTime must be called for QgsDateTimeEdit.
     */
    void setDateTime( const QDateTime &dateTime );

    /**
     * @brief dateTime returns the date time which can eventually be a null date/time
     * @note since QDateTimeEdit::dateTime() is not virtual, dateTime must be called for QgsDateTimeEdit.
     */
    QDateTime dateTime() const;

    //! Set the current date as NULL
    //! @note if the widget is not configured to accept NULL dates, this will have no effect
    virtual void clear();

    /** Resets the widget to show no value (ie, an "unknown" state).
     * @note added in QGIS 2.16
     */
    void setEmpty();

  protected:
    virtual void resizeEvent( QResizeEvent* event );

    void mousePressEvent( QMouseEvent*event );
};
