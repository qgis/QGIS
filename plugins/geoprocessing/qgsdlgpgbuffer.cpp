
#include <qlineedit.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include "qgsdlgpgbuffer.h"

QgsDlgPgBuffer::QgsDlgPgBuffer(QWidget *parent, const char *name)
  :QgsDlgPgBufferBase(parent,name){
}
QgsDlgPgBuffer::~QgsDlgPgBuffer(){
}
void QgsDlgPgBuffer::setBufferLabel(QString &lbl){
  lblBufferInfo->setText(lbl);
}
QString QgsDlgPgBuffer::bufferDistance(){
  return txtBufferDistance->text();
}
QString QgsDlgPgBuffer::bufferLayerName(){
  return txtBufferedLayerName->text();
}
bool QgsDlgPgBuffer::addLayerToMap(){
  return chkAddToMap->isChecked();
}
void QgsDlgPgBuffer::addFieldItem(QString field){
  cmbFields->insertItem(field);
}
