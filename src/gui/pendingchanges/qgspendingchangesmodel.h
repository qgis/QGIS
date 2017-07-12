#ifndef QGSPENDINGCHANGESMODEL_H
#define QGSPENDINGCHANGESMODEL_H

#include <QStandardItemModel>
#include <QStandardItem>

#include "qgis_gui.h"
#include "qgsfeature.h"

class QgsMapLayer;
class QgsProject;
class QgsVectorLayerEditBuffer;

class GUI_EXPORT QgsPendingChangesModel : public QStandardItemModel
{
    Q_OBJECT

  public:
    QgsPendingChangesModel( QgsProject *project, QObject *parent = nullptr );

  private slots:

    void connectAddedLayers( const QList<QgsMapLayer *> layers );

    void featureAdded( QgsFeatureId id, QgsVectorLayer *layer );

    void featureDeleted( QgsFeatureId id, QgsVectorLayer *layer );

    void featureModified( QgsFeatureId id, QgsVectorLayer *layer );

  private:
    bool nodeContainsFeature(QStandardItem* root, QgsFeatureId id, QgsVectorLayer *layer);
    QList<QStandardItem *> makeFeatureItem( QgsFeatureId id, QgsVectorLayer *layer ) const;
    QStandardItem *mAddedFeaturesRoot = nullptr;
    QStandardItem *mDeletedFeatuersRoot = nullptr;
    QStandardItem *mChagnedFeaturesRoot = nullptr;

};

#endif // QGSPENDINGCHANGESMODEL_H
