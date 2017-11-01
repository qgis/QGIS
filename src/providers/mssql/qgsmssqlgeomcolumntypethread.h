#ifndef QGSMSSQLGEOMCOLUMNTYPETHREAD_H
#define QGSMSSQLGEOMCOLUMNTYPETHREAD_H

#include "qgsmssqltablemodel.h"

#include <QThread>

// A class that determines the geometry type of a given database
// schema.table.column, with the option of doing so in a separate
// thread.

class QgsMssqlGeomColumnTypeThread : public QThread
{
    Q_OBJECT
  public:
    QgsMssqlGeomColumnTypeThread( const QString &connectionName, bool useEstimatedMetadata );

    // These functions get the layer types and pass that information out
    // by emitting the setLayerType() signal.
    virtual void run() override;

  signals:
    void setLayerType( const QgsMssqlLayerProperty &layerProperty );

  public slots:
    void addGeometryColumn( const QgsMssqlLayerProperty &layerProperty );
    void stop();

  private:
    QgsMssqlGeomColumnTypeThread() = delete;

    QString mConnectionName;
    bool mUseEstimatedMetadata;
    bool mStopped;
    QList<QgsMssqlLayerProperty> layerProperties;
};

#endif // QGSMSSQLGEOMCOLUMNTYPETHREAD_H
