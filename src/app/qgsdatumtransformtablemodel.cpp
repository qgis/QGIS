#include "qgsdatumtransformtablemodel.h"

QgsDatumTransformTableModel::QgsDatumTransformTableModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

void QgsDatumTransformTableModel::setTransformContext(QgsCoordinateTransformContext &context)
{
  mTransformContext = context;
  reset();
}

int QgsDatumTransformTableModel::rowCount(const QModelIndex &parent) const
{
    return mTransformContext.sourceDestinationDatumTransforms().count()
            + mTransformContext.sourceDatumTransforms().count()
            + mTransformContext.destinationDatumTransforms().count();
}

int QgsDatumTransformTableModel::columnCount(const QModelIndex &parent) const
{
    return 4;
}

QVariant QgsDatumTransformTableModel::data(const QModelIndex &index, int role) const
{


    return QVariant();
}

QVariant QgsDatumTransformTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Vertical)
        return QVariant();

    switch (role) {
    case Qt::DisplayRole:
        switch (section) {
        case SourceCrsHeader:
            return tr("Source CRS");
        case SourceTransformHeader:
            return tr("Source datum transform");
        case DestinationCrsHeader:
            return tr("Destination CRS");
        case DestinationTransformHeader:
            return tr("Destination datum transform");
        default:
            break;
        }
        break;
    default:
        break;
    }

    return QVariant();
}
