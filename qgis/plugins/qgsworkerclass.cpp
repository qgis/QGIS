// $FILENAME
#include "qgsworkerclass.h"
#include <qmessagebox.h>

QgsWorkerClass::QgsWorkerClass()

{
}


QgsWorkerClass::~QgsWorkerClass()
{
}
void QgsWorkerClass::open(){
	QMessageBox::information(this, "Worker class", "Message from open function in worker class");
}
void QgsWorkerClass::newThing(){
}