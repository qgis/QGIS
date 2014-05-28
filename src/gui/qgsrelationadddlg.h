#ifndef QGSRELATIONADDDLG_H
#define QGSRELATIONADDDLG_H

#include <QDialog>
#include "ui_qgsrelationadddlgbase.h"

class QgsVectorLayer;

class GUI_EXPORT QgsRelationAddDlg : public QDialog, private Ui::QgsRelationAddDlgBase
{
    Q_OBJECT

  public:
    explicit QgsRelationAddDlg( QWidget *parent = 0 );

    void addLayers( QList<QgsVectorLayer* > layers );

    QString referencingLayerId();
    QString referencedLayerId();
    QList< QPair< QString, QString > > references();
    QString relationId();
    QString relationName();


  private slots:
    void on_mCbxReferencingLayer_currentIndexChanged( int index );
    void on_mCbxReferencedLayer_currentIndexChanged( int index );

  private:
    void loadLayerAttributes( QComboBox* cbx, QgsVectorLayer* layer );

    QMap< QString, QgsVectorLayer* > mLayers;

};

#endif // QGSRELATIONADDDLG_H
