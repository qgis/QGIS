#ifndef QGSDATUMTRANSFORMTABLEMODEL_H
#define QGSDATUMTRANSFORMTABLEMODEL_H


#include <QAbstractTableModel>

#include "qgis_app.h"
#include "qgscoordinatetransformcontext.h"

class APP_EXPORT QgsDatumTransformTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:

    enum Headers {
        SourceCrsHeader = 0,
        SourceTransformHeader,
        DestinationCrsHeader,
        DestinationTransformHeader,
    };

    QgsDatumTransformTableModel( QObject* parent = nullptr);

    void setTransformContext( QgsCoordinateTransformContext &context );

public:
    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

private:
    QgsCoordinateTransformContext mTransformContext;
};

#endif // QGSDATUMTRANSFORMTABLEMODEL_H
