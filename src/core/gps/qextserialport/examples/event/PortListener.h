/**
 * @file PortListener.h
 * @brief Port Listener.
 * @see PortListener
 */

#ifndef PORTLISTENER_H_
#define PORTLISTENER_H_

#include <QObject>

class QextSerialPort;


/**
 * Port Listener.
 */
class PortListener : public QObject
{
	Q_OBJECT
	
	QextSerialPort * port;


	public:
		/**
		 * Constructor.
		 *  @param parent parent object.
		 */
		PortListener(QextSerialPort * port, QObject * parent = 0);
	
	public slots:
		/**
		 * Receive data from serial port.
		 */
		void receive();
		
		/**
		 * Report written bytes.
		 * 	@param bytes number of written bytes.
		 */
		void reportWritten(qint64 bytes);
		
		/**
		 * Report port closing.
		 */
		void reportClose();
		
		/**
		 * Report DSR line.
		 * 	@param status line status.
		 */
		void reportDsr(bool status);

};


#endif /*PORTLISTENER_H_*/
