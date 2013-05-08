#ifndef QGSDATADEFINEDSYMBOLLAYERDIALOG_H
#define QGSDATADEFINEDSYMBOLLAYERDIALOG_H

#include "ui_qgsdatadefinedsymboldialogbase.h"
#include <QDialog>

class QgsVectorLayer;
class QComboBox;



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

    QgsDataDefinedSymbolDialog( const QList< DataDefinedSymbolEntry >& entries, const QgsVectorLayer* vl, QWidget * parent = 0, Qt::WindowFlags f = 0 );
    ~QgsDataDefinedSymbolDialog();
    QMap< QString, QString > dataDefinedProperties() const;

    //common help texts
    static QString doubleHelpText();
    static QString colorHelpText();
    static QString offsetHelpText();
    static QString fileNameHelpText();

  private slots:
    void expressionButtonClicked();

  private:
    const QgsVectorLayer* mVectorLayer;

    /**Tries to fiend a combo box field for an expression string (considering whitespaces, brackets around attribute names)
        @return index or -1 in case not found*/
    int comboIndexForExpressionString( const QString& expr, const QComboBox* cb );
};

#endif // QGSDATADEFINEDSYMBOLLAYERDIALOG_H
