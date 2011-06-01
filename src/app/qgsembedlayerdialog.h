#ifndef QGSEMBEDLAYERSDIALOG_H
#define QGSEMBEDLAYERSDIALOG_H

#include "QDialog"
#include "ui_qgsembedlayerdialogbase.h"

class QDomElement;

class QgsEmbedLayerDialog: public QDialog, private Ui::QgsEmbedLayerDialogBase
{
  Q_OBJECT
  public:
    QgsEmbedLayerDialog( QWidget * parent = 0, Qt::WindowFlags f = 0 );
    ~QgsEmbedLayerDialog();

    /**Returns name / projectfiles of groups to embed*/
    QList< QPair < QString, QString > > embeddedGroups() const;
    /**Returns layer id / projectfiles of single layers to embed*/
    QList< QPair < QString, QString > > embeddedLayers() const;

  private slots:
    void on_mBrowseFileToolButton_clicked();
    void on_mProjectFileLineEdit_editingFinished();
    void on_mTreeWidget_itemSelectionChanged();

  private:
    void changeProjectFile();
    void addLegendGroupToTreeWidget( const QDomElement& groupElem, QTreeWidgetItem* parent = 0 );
    void addLegendLayerToTreeWidget( const QDomElement& layerElem, QTreeWidgetItem* parent = 0 );
    void unselectChildren( QTreeWidgetItem* item );
};

#endif // QGSEMBEDLAYERSDIALOG_H
