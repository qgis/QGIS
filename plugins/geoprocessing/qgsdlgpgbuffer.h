#ifndef QGSDLGPGBUFFER_H
#define QGSDLGPGBUFFER_H
#include "qgsdlgpgbufferbase.h"
class QgsDlgPgBuffer : public QgsDlgPgBufferBase
{
  Q_OBJECT
public:
  QgsDlgPgBuffer(QWidget *parent=0, const char *name=0);
  QgsDlgPgBuffer::~QgsDlgPgBuffer();
  void setBufferLabel(QString &lbl);
  QString bufferDistance();
  QString bufferLayerName();
  QString objectIdColumn();
  QString geometryColumn();
  QString srid();
  QString schema();
  bool addLayerToMap();
  void addFieldItem(QString field);
  void addSchema(QString schema);
  void setSrid(QString srid);
  void setBufferLayerName(QString name);
  void setGeometryColumn(QString name);
 };
#endif // QGSDLGPGBUFFER_H
