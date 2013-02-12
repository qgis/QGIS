
/** A combobox which lets the user select map scale from predefined list
 * and highlights nearest to current scale value
 */
class QgsScaleComboBox : QComboBox
{
%TypeHeaderCode
#include <qgsscalecombobox.h>
%End

  public:
    QgsScaleComboBox( QWidget* parent = 0 );
    ~QgsScaleComboBox();

    //! Function to read the selected scale as text
    // @note added in 2.0
    QString scaleString();
    //! Function to set the selected scale from text
    // @note added in 2.0
    bool setScaleString( QString scaleTxt );
    //! Function to read the selected scale as double
    // @note added in 2.0
    double scale();
    //! Function to set the selected scale from double
    // @note added in 2.0
    void setScale( double scale );

    //! Helper function to convert a double to scale string
    // Performs rounding, so an exact representation is not to
    // be expected.
    // @note added in 2.0
    static QString toString( double scale );
    //! Helper function to convert a scale string to double
    // @note added in 2.0
    static double toDouble( QString scaleString, bool *ok = NULL );

  signals:
    //! Signal is emitted when *user* has finished editing/selecting a new scale.
    // @note added in 2.0
    void scaleChanged();

  public slots:
    void updateScales( const QStringList &scales = QStringList() );

  protected:
    void showPopup();
};

