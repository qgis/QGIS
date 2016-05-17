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

    //! @deprecated will be removed in QGIS 3.0
    QgsDataDefinedSymbolDialog( const QList< QgsDataDefinedSymbolDialog::DataDefinedSymbolEntry >& entries, const QgsVectorLayer* vl, QWidget * parent /TransferThis/ = 0, const Qt::WindowFlags& f = 0 ) /Deprecated/;
    ~QgsDataDefinedSymbolDialog();

    //! @deprecated will be removed in QGIS 3.0
    QMap< QString, QString > dataDefinedProperties() const /Deprecated/;

    //common help texts
    //! @deprecated will be removed in QGIS 3.0
    static QString doubleHelpText() /Deprecated/;
    //! @deprecated will be removed in QGIS 3.0
    static QString colorHelpText() /Deprecated/;
    //! @deprecated will be removed in QGIS 3.0
    static QString offsetHelpText() /Deprecated/;
    //! @deprecated will be removed in QGIS 3.0
    static QString fileNameHelpText() /Deprecated/;
    //! @deprecated will be removed in QGIS 3.0
    static QString horizontalAnchorHelpText() /Deprecated/;
    //! @deprecated will be removed in QGIS 3.0
    static QString verticalAnchorHelpText() /Deprecated/;
    //! @deprecated will be removed in QGIS 3.0
    static QString gradientTypeHelpText() /Deprecated/;
    //! @deprecated will be removed in QGIS 3.0
    static QString gradientCoordModeHelpText() /Deprecated/;
    //! @deprecated will be removed in QGIS 3.0
    static QString gradientSpreadHelpText() /Deprecated/;
    //! @deprecated will be removed in QGIS 3.0
    static QString boolHelpText() /Deprecated/;
    //! @deprecated will be removed in QGIS 3.0
    static QString lineStyleHelpText() /Deprecated/;
    //! @deprecated will be removed in QGIS 3.0
    static QString joinStyleHelpText() /Deprecated/;
    //! @deprecated will be removed in QGIS 3.0
    static QString capStyleHelpText() /Deprecated/;
    //! @deprecated will be removed in QGIS 3.0
    static QString fillStyleHelpText() /Deprecated/;
};
