#ifndef QGSDB2TABLEMODEL_H
#define QGSDB2TABLEMODEL_H

#include <QStandardItemModel>

#include "qgis.h"

/** Layer Property structure */
struct QgsDb2LayerProperty
{
  QString     type;
  QString     schemaName;
  QString     tableName;
  QString     geometryColName;
  QStringList pkCols;
  QString     srid;
  QString     sql;
};


class QIcon;

/** A model that holds the tables of a database in a hierarchy where the
schemas are the root elements that contain the individual tables as children.
The tables have the following columns: Type, Schema, Tablename, Geometry Column, Sql*/
class QgsDb2TableModel : public QStandardItemModel
{
    Q_OBJECT
  public:
    QgsDb2TableModel();
    ~QgsDb2TableModel();

    /** Adds entry for one database table to the model*/
    void addTableEntry( const QgsDb2LayerProperty &property );

    /** Sets an sql statement that belongs to a cell specified by a model index*/
    void setSql( const QModelIndex& index, const QString& sql );

    /** Sets one or more geometry types to a row. In case of several types, additional rows are inserted.
       This is for tables where the type is dectected later by thread*/
    void setGeometryTypesForTable( QgsDb2LayerProperty layerProperty );

    /** Returns the number of tables in the model*/
    int tableCount() const { return mTableCount; }

    enum columns
    {
      dbtmSchema = 0,
      dbtmTable,
      dbtmType,
      dbtmGeomCol,
      dbtmSrid,
      dbtmPkCol,
      dbtmSelectAtId,
      dbtmSql,
      dbtmColumns
    };

    bool setData( const QModelIndex &index, const QVariant &value, int role = Qt::EditRole ) override;

    QString layerURI( const QModelIndex &index, const QString &connInfo, bool useEstimatedMetadata );

    static QIcon iconForWkbType( QGis::WkbType type );

    static QGis::WkbType wkbTypeFromDb2( QString dbType );

    static QString displayStringForWkbType( QGis::WkbType type );

  private:
    /** Number of tables in the model*/
    int mTableCount;
};
#endif
