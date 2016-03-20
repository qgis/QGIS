
/** A combobox which lets the user select map scale from predefined list
 * and highlights nearest to current scale value
 */
class QgsScaleComboBox : QComboBox
{
%TypeHeaderCode
#include <qgsscalecombobox.h>
%End

  public:
    QgsScaleComboBox( QWidget* parent /TransferThis/ = 0 );
    ~QgsScaleComboBox();

    //! Function to read the selected scale as text
    QString scaleString();
    //! Function to set the selected scale from text
    bool setScaleString( const QString& scaleTxt );
    //! Function to read the selected scale as double
    double scale() const;
    //! Function to set the selected scale from double
    void setScale( double scale );
    //! Function to read the min scale
    double minScale() const;

    //! Helper function to convert a double to scale string
    // Performs rounding, so an exact representation is not to
    // be expected.
    static QString toString( double scale );
    //! Helper function to convert a scale string to double
    static double toDouble( const QString& scaleString, bool *ok = 0 );

  signals:
    //! Signal is emitted when *user* has finished editing/selecting a new scale.
    void scaleChanged( double scale );

  public slots:
    void updateScales( const QStringList &scales = QStringList() );
    //! Function to set the min scale
    void setMinScale( double scale );

  protected:
    void showPopup();
};

