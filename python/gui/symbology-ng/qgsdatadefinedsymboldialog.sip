/** \ingroup gui
 * \class QgsDataDefinedSymbolDialog
 * \deprecated no longer used and will be removed in QGIS 3.0
 */
class QgsDataDefinedSymbolDialog : QDialog
{
%TypeHeaderCode
#include <qgsdatadefinedsymboldialog.h>
%End
  public:
    class DataDefinedSymbolEntry
    {
    public:
      DataDefinedSymbolEntry( const QString& p, const QString& t, const QString& v, const QString& h );
    };

    QgsDataDefinedSymbolDialog( const QList< QgsDataDefinedSymbolDialog::DataDefinedSymbolEntry >& entries, const QgsVectorLayer* vl, QWidget * parent /TransferThis/ = 0, Qt::WindowFlags f = 0 ) /Deprecated/;
    ~QgsDataDefinedSymbolDialog();
    QMap< QString, QString > dataDefinedProperties() const /Deprecated/;

    //common help texts
    static QString doubleHelpText() /Deprecated/;
    static QString colorHelpText() /Deprecated/;
    static QString offsetHelpText() /Deprecated/;
    static QString fileNameHelpText() /Deprecated/;
    static QString horizontalAnchorHelpText() /Deprecated/;
    static QString verticalAnchorHelpText() /Deprecated/;
    static QString gradientTypeHelpText() /Deprecated/;
    static QString gradientCoordModeHelpText() /Deprecated/;
    static QString gradientSpreadHelpText() /Deprecated/;
    static QString boolHelpText() /Deprecated/;
    static QString lineStyleHelpText() /Deprecated/;
    static QString joinStyleHelpText() /Deprecated/;
    static QString capStyleHelpText() /Deprecated/;
    static QString fillStyleHelpText() /Deprecated/;
};
