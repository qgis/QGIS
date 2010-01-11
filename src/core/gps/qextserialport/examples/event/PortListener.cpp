/**
 * @file PortListener.cpp
 * @brief PortListener Implementation.
 * @see PortListener.h
 */


/*
==============
<INIT>
==============
*/

#include <QTextStream>
#include <qextserialport.h>
#include "PortListener.h"


/*
==============
<CONSTRUCTORS>
==============
*/

PortListener::PortListener(QextSerialPort * port, QObject * parent):
	QObject(parent)
{
	this->port = port;
}


/*
==============
<DESTRUCTOR>
==============
*/


/*
==============
<STATIC>
==============
*/


/*
==============
<SLOTS>
==============
*/

void PortListener::receive()
{
	char data[1024];
	QTextStream out(stdout);
	
	out << "data received: ";
	int bytesRead = port->read(data, 1024);
	data[bytesRead] = '\0';
	out << data << " (" << bytesRead << " bytes)" << endl;
}

void PortListener::reportWritten(qint64 bytes)
{
	QTextStream out(stdout);
	
	out << bytes << " bytes written" << endl;
}

void PortListener::reportClose()
{
	QTextStream out(stdout);
	
	out << "closing port" << endl;	
}

void PortListener::reportDsr(bool status)
{
	QTextStream out(stdout);
	
	if (status)
		out << "device was turned on" << endl;
	else
		out << "device was turned off" << endl;
}

/*
==============
<VIRTUAL>
==============
*/


/*
==============
<NON-VIRTUAL>
==============
*/

