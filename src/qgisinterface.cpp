#include <iostream>
#include "qgisinterface.h"
#include "qgisapp.h"

QgisInterface::QgisInterface(QgisApp * _qgis, const char *name):QWidget(_qgis, name)
{


}

QgisInterface::~QgisInterface()
{
}

void QgisInterface::zoomFull()
{
}
void QgisInterface::zoomPrevious()
{
}
void QgisInterface::zoomActiveLayer()
{
}
int QgisInterface::getInt()
{
}
