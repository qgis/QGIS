#ifndef QGSDATADEFINEDSYMBOLLAYERDIALOG_H
#define QGSDATADEFINEDSYMBOLLAYERDIALOG_H

#include "ui_qgsdatadefinedsymboldialogbase.h"
#include <QDialog>

class QgsVectorLayer;
class QComboBox;


/** \ingroup gui
 * \class QgsDataDefinedSymbolDialog
 * \deprecated no longer used and will be removed in QGIS 3.0
 */
class GUI_EXPORT QgsDataDefinedSymbolDialog: public QDialog, private Ui::QgsDataDefinedSymbolDialog
{
    Q_OBJECT
  public:

    struct DataDefinedSymbolEntry
    {
      DataDefinedSymbolEntry( const QString& p, const QString& t, const QString& v, const QString& h ):
          property( p ), title( t ), initialValue( v ), helpText( h ) {}
      QString property;
      QString title;
      QString initialValue;
      QString helpText;
    };

    Q_DECL_DEPRECATED QgsDataDefinedSymbolDialog( const QList< DataDefinedSymbolEntry >& entries, const QgsVectorLayer* vl, QWidget * parent = 0, Qt::WindowFlags f = 0 );
    ~QgsDataDefinedSymbolDialog();
    Q_DECL_DEPRECATED QMap< QString, QString > dataDefinedProperties() const;

    //common help texts
    Q_DECL_DEPRECATED static QString doubleHelpText();
    Q_DECL_DEPRECATED static QString colorHelpText();
    Q_DECL_DEPRECATED static QString offsetHelpText();
    Q_DECL_DEPRECATED static QString fileNameHelpText();
    Q_DECL_DEPRECATED static QString horizontalAnchorHelpText();
    Q_DECL_DEPRECATED static QString verticalAnchorHelpText();
    Q_DECL_DEPRECATED static QString gradientTypeHelpText();
    Q_DECL_DEPRECATED static QString gradientCoordModeHelpText();
    Q_DECL_DEPRECATED static QString gradientSpreadHelpText();
    Q_DECL_DEPRECATED static QString boolHelpText();
    Q_DECL_DEPRECATED static QString lineStyleHelpText();
    Q_DECL_DEPRECATED static QString joinStyleHelpText();
    Q_DECL_DEPRECATED static QString capStyleHelpText();
    Q_DECL_DEPRECATED static QString fillStyleHelpText();

  private:
    const QgsVectorLayer* mVectorLayer;
};

#endif // QGSDATADEFINEDSYMBOLLAYERDIALOG_H
