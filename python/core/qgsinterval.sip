/** \ingroup core
 * \class QgsInterval
 * \brief A representation of the interval between two datetime values.
 * \note Added in version 2.16
 */

class QgsInterval
{
%TypeHeaderCode
#include <qgsinterval.h>
%End

  public:

    // YEAR const value taken from postgres query
    // SELECT EXTRACT(EPOCH FROM interval '1 year')
    //! Seconds per year (average)
    static const int YEARS;
    //! Seconds per month, based on 30 day month
    static const int MONTHS;
    //! Seconds per week
    static const int WEEKS;
    //! Seconds per day
    static const int DAY;
    //! Seconds per hour
    static const int HOUR;
    //! Seconds per minute
    static const int MINUTE;

    /** Default constructor for QgsInterval. Creates an invalid interval.
     */
    QgsInterval();

    /** Constructor for QgsInterval.
     * @param seconds duration of interval in seconds
     */
    QgsInterval( double seconds );

    /** Returns the interval duration in years (based on an average year length)
     * @see setYears()
     */
    double years() const;

    /** Sets the interval duration in years.
     * @param years duration in years (based on average year length)
     * @see years()
     */
    void setYears( double years );

    /** Returns the interval duration in months (based on a 30 day month).
     * @see setMonths()
     */
    double months() const;

    /** Sets the interval duration in months.
     * @param months duration in months (based on a 30 day month)
     * @see months()
     */
    void setMonths( double months );

    /** Returns the interval duration in weeks.
     * @see setWeeks()
     */
    double weeks() const;

    /** Sets the interval duration in weeks.
     * @param weeks duration in weeks
     * @see weeks()
     */
    void setWeeks( double weeks );

    /** Returns the interval duration in days.
     * @see setDays()
     */
    double days() const;

    /** Sets the interval duration in days.
     * @param days duration in days
     * @see days()
     */
    void setDays( double days );

    /** Returns the interval duration in hours.
     * @see setHours()
     */
    double hours() const;

    /** Sets the interval duration in hours.
     * @param hours duration in hours
     * @see hours()
     */
    void setHours( double hours );

    /** Returns the interval duration in minutes.
     * @see setMinutes()
     */
    double minutes() const;

    /** Sets the interval duration in minutes.
     * @param minutes duration in minutes
     * @see minutes()
     */
    void setMinutes( double minutes );

    /** Returns the interval duration in seconds.
     * @see setSeconds()
     */
    double seconds() const;

    /** Sets the interval duration in seconds.
     * @param seconds duration in seconds
     * @see seconds()
     */
    void setSeconds( double seconds );

    /** Returns true if the interval is valid.
     * @see setValid()
     */
    bool isValid() const;

    /** Sets whether the interval is valid.
     * @param valid set to true to set the interval as valid.
     * @see isValid()
     */
    void setValid( bool valid );

    bool operator==( const QgsInterval& other ) const;

    /** Converts a string to an interval
     * @param string string to parse
     * @returns interval, or invalid interval if string could not be parsed
     */
    static QgsInterval fromString( const QString& string );

    //! Allows direct construction of QVariants from intervals.
    operator QVariant() const;

};

