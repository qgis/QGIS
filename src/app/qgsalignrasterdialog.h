#ifndef QGSALIGNRASTERDIALOG_H
#define QGSALIGNRASTERDIALOG_H

#include <QDialog>

#include "ui_qgsalignrasterdialog.h"

class QgsAlignRaster;

/** Dialog providing user interface for QgsAlignRaster */
class QgsAlignRasterDialog : public QDialog, private Ui::QgsAlignRasterDialog
{
    Q_OBJECT
  public:
    explicit QgsAlignRasterDialog( QWidget *parent = 0 );
    ~QgsAlignRasterDialog();

  signals:

  protected slots:
    void addLayer();
    void removeLayer();
    void editLayer();

    void updateConfigFromReferenceLayer();

    void runAlign();

    void destinationCrsChanged();

    void clipExtentChanged();

  protected:
    void populateLayersView();
    void updateAlignedRasterInfo();

  protected:
    QgsAlignRaster* mAlign;
};


class QgsMapLayerComboBox;
class QCheckBox;

/** Simple dialog to display details of one layer's configuration */
class QgsAlignRasterLayerConfigDialog : public QDialog
{
    Q_OBJECT
  public:
    QgsAlignRasterLayerConfigDialog();

    QString inputFilename() const;
    QString outputFilename() const;
    int resampleMethod() const;
    bool rescaleValues() const;

    void setItem( const QString& inputFilename, const QString& outputFilename, int resampleMethod, bool rescaleValues );

  protected slots:
    void browseOutputFilename();

  protected:
    QgsMapLayerComboBox* cboLayers;
    QLineEdit* editOutput;
    QPushButton* btnBrowse;
    QComboBox* cboResample;
    QCheckBox* chkRescale;
    QDialogButtonBox* btnBox;
};


#endif // QGSALIGNRASTERDIALOG_H
