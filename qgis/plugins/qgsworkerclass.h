
#ifndef QGSWORKERCLASS_H
#define QGSWORKERCLASS_H

#include <qobject.h>

/**
 * 
 * Gary Sherman
 **/
class QgsWorkerClass : public QObject
{
Q_OBJECT
public:
  QgsWorkerClass();
  ~QgsWorkerClass();
  public slots:
  	void open();
	void newThing();
};

#endif
