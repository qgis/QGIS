#ifndef QGSRASTERTERRAINANALYSISDIALOG_H
#define QGSRASTERTERRAINANALYSISDIALOG_H

#include "ui_qgsrasterterrainanalysisdialogbase.h"
#include "qgsrelief.h"

class QgsRasterTerrainAnalysisDialog: public QDialog, private Ui::QgsRasterTerrainAnalysisDialogBase
{
    Q_OBJECT
  public:

    enum DisplayMode
    {
      NoParameter,
      HillshadeInput,
      ReliefInput
    };

    QgsRasterTerrainAnalysisDialog( DisplayMode mode = NoParameter, QWidget * parent = 0, Qt::WindowFlags f = 0 );
    ~QgsRasterTerrainAnalysisDialog();

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
    void on_mAddClassButton_clicked();
    void on_mRemoveClassButton_clicked();
    void on_mUpPushButton_clicked();
    void on_mDownPushButton_clicked();
    void on_mReliefClassTreeWidget_itemDoubleClicked( QTreeWidgetItem* item, int column );
    void on_mExportToCsvButton_clicked();
    void on_mExportColorsButton_clicked();
    void on_mImportColorsButton_clicked();
    void on_mButtonBox_accepted();

  private:
    /**Stores relation between driver name and extension*/
    QMap<QString, QString> mDriverExtensionMap;
};

#endif //QGSRASTERTERRAINANALYSISDIALOG_H
