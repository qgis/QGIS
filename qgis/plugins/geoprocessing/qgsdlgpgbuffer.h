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
  bool addLayerToMap();
  void addFieldItem(QString field);
 };
#endif // QGSDLGPGBUFFER_H
