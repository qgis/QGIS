#ifndef QGSRELIEFDIALOG_H
#define QGSRELIEFDIALOG_H

#include "ui_qgsreliefdialogbase.h"
#include "qgsrelief.h"

class QgsReliefDialog: public QDialog, private Ui::QgsReliefDialogBase
{
    Q_OBJECT
  public:

    enum DisplayMode
    {
      NoParameter,
      HillshadeInput,
      ReliefInput
    };

    QgsReliefDialog( DisplayMode mode = NoParameter, QWidget * parent = 0, Qt::WindowFlags f = 0 );
    ~QgsReliefDialog();

    QList< QgsRelief::ReliefColor > reliefColors() const;
    QString inputFile() const;
    QString outputFile() const;
    QString outputFormat() const;

    bool addResultToProject() const;
    double zFactor() const;
    double lightAzimuth() const;
    double lightAngle() const;

  private slots:
    void on_mOutputLayerLineEdit_textChanged( const QString& text );
    void on_mAutomaticColorButton_clicked();
    void on_mOutputLayerToolButton_clicked();
    void on_mRemoveClassButton_clicked();
    void on_mReliefClassTreeWidget_itemDoubleClicked( QTreeWidgetItem* item, int column );
    void on_mExportToCsvButton_clicked();

  private:
    /**Stores relation between driver name and extension*/
    QMap<QString, QString> mDriverExtensionMap;
};

#endif // QGSRELIEFDIALOG_H
