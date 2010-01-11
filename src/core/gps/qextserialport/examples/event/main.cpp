/**
 * @file main.cpp
 * @brief Main file.
 * @author Michal Policht
 */

#include <QCoreApplication>
#include <QString>
#include <QTextStream>
#include <QThread>
#include <qextserialport.h>
#include "PortListener.h"

class SerialThread: public QThread
{
	QextSerialPort * port;
	
	protected:
		virtual void run()
		{
			QString inStr;
			QTextStream in(stdin);
			QTextStream out(stdout);
			
			out << "Special commands: open close bye" << endl << endl;
			while(inStr.compare("bye") != 0) {
				in >> inStr;
				if (inStr.compare("close") == 0) {
					out << "requesting close... " << endl; 
					port->close();
				}
				else if (inStr.compare("open") == 0) {
					out << "opening port... ";
					out << port->open(QIODevice::ReadWrite);
					out << endl;
				} else if (inStr.compare("bye") != 0)
					port->write(inStr.toAscii().constData(), inStr.length());
			}
			port->close();
		}
	
	public:
		SerialThread(QextSerialPort * port, QObject * parent = 0):
			QThread(parent)
		{
			this->port = port;
		}
}; 

int main(int argc, char *argv[])
{
	QTextStream out(stdout);
	QCoreApplication app(argc, argv);

	QextSerialPort * port = new QextSerialPort("COM1", QextSerialPort::EventDriven);
	port->setBaudRate(BAUD56000);
	port->setFlowControl(FLOW_OFF);
	port->setParity(PAR_NONE);
	port->setDataBits(DATA_8);
	port->setStopBits(STOP_2);
	port->open(QIODevice::ReadWrite);
	if (!(port->lineStatus() & LS_DSR)) {
		out << "warning: device is not turned on" << endl;
	}

	PortListener * listener = new PortListener(port);
	//Because port and listener are working in different threads 
	//Qt will choose queued connection. This may produce clusters of 
	//signals waiting in the queue.
	listener->connect(port, SIGNAL(readyRead()), listener, SLOT(receive()));
	listener->connect(port, SIGNAL(bytesWritten(qint64)), listener, SLOT(reportWritten(qint64)));
	listener->connect(port, SIGNAL(aboutToClose()), listener, SLOT(reportClose()));
	listener->connect(port, SIGNAL(dsrChanged(bool)), listener, SLOT(reportDsr(bool)));
	
	SerialThread * thread = new SerialThread(port);
	app.connect(thread, SIGNAL(finished()), & app, SLOT(quit()));
	thread->start();
	
	//event loop is required to utilize signals and slots mechanism 
	return app.exec();
}
