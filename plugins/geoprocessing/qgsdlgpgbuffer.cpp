
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
QString QgsDlgPgBuffer::geometryColumn(){
  return txtGeometryColumn->text();
}
QString QgsDlgPgBuffer::srid(){
  return txtSrid->text();
}
QString QgsDlgPgBuffer::objectIdColumn(){
  return cmbFields->currentText();
}
QString QgsDlgPgBuffer::schema(){
  return cmbSchema->currentText();
}
void QgsDlgPgBuffer::addFieldItem(QString field){
  cmbFields->insertItem(field);
}
void QgsDlgPgBuffer::addSchema(QString schema){
  cmbSchema->insertItem(schema);
}
void QgsDlgPgBuffer::setSrid(QString srid){
  txtSrid->setText(srid);
}
void QgsDlgPgBuffer::setBufferLayerName(QString name){
  txtBufferedLayerName->setText(name);
}
void QgsDlgPgBuffer::setGeometryColumn(QString name){
  txtGeometryColumn->setText(name);
}


