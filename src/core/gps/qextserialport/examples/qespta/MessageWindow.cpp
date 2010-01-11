/**
 * @file MessageWindow.cpp
 * @brief MessageWindow Implementation.
 * @see MessageWindow.h
 * @author Micha³ Policht
 */


/*
==============
<INIT>
==============
*/

#include "MessageWindow.h"
#include <QMessageBox>
#include <QCoreApplication>
#include <QMutexLocker>

const char* MessageWindow::WINDOW_TITLE = "Message Window";
MessageWindow* MessageWindow::MsgHandler = NULL;


/*
==============
<CONSTRUCTORS>
==============
*/

MessageWindow::MessageWindow(QWidget* parent, Qt::WFlags flags) 
	: QDockWidget(parent, flags),
		msgTextEdit(this)
{
	setWindowTitle(tr(WINDOW_TITLE));
	msgTextEdit.setReadOnly(true);
	setWidget(&msgTextEdit);

	MessageWindow::MsgHandler = this;
}


/*
==============
<DESTRUCTOR>
==============
*/


/*
==============
<SLOTS>
==============
*/


/*
==============
<METHODS>
==============
*/

//static
QString MessageWindow::QtMsgToQString(QtMsgType type, const char *msg)
{
	switch (type) {
		case QtDebugMsg:
			return QString("Debug: ")+QString(msg);
		case QtWarningMsg:
			return QString("Warning: ")+QString(msg);
		case QtCriticalMsg:
			return QString("Critical: ")+QString(msg);
		case QtFatalMsg:
			return QString("Fatal: ")+QString(msg);
		default:
			return QString("Unrecognized message type: ")+QString(msg);
	}
}

//static
void MessageWindow::AppendMsgWrapper(QtMsgType type, const char* msg)
{
	static QMutex mutex;
	QMutexLocker locker(&mutex);
	
	if (MessageWindow::MsgHandler != NULL)
		return MessageWindow::MsgHandler->postMsgEvent(type, msg);
	else
		fprintf(stderr, MessageWindow::QtMsgToQString(type, msg).toAscii());
}

void MessageWindow::customEvent(QEvent* event)
{
	if (static_cast<MessageWindow::EventType>(event->type()) == MessageWindow::MessageEvent)
		msgTextEdit.append(dynamic_cast<MessageEvent::MessageEvent* >(event)->msg);
}

void MessageWindow::postMsgEvent(QtMsgType type, const char* msg)
{
	QString qmsg = MessageWindow::QtMsgToQString(type, msg);
	switch (type) {
		case QtDebugMsg:
			break;
		case QtWarningMsg:
			qmsg.prepend("<FONT color=\"#FF0000\">");
			qmsg.append("</FONT>");
			break;
		case QtCriticalMsg:
			if (QMessageBox::critical(this, "Critical Error", qmsg, 
					QMessageBox::Ignore,
					QMessageBox::Abort,
					QMessageBox::NoButton) == QMessageBox::Abort)
				abort(); // core dump
			qmsg.prepend("<B><FONT color=\"#FF0000\">");
			qmsg.append("</FONT></B>");
			break;
		case QtFatalMsg:
			QMessageBox::critical(this, "Fatal Error", qmsg, QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
			abort(); // deliberately core dump
	}
	//it's impossible to change GUI directly from thread other than the main thread 
	//so post message encapsulated by MessageEvent to the main thread's event queue
	QCoreApplication::postEvent(this, new MessageEvent::MessageEvent(qmsg));
}


/*
==============
<HELPERS>
==============
*/

MessageEvent::MessageEvent(QString & msg):
	QEvent(static_cast<QEvent::Type>(MessageWindow::MessageEvent))
{
	this->msg = msg;
}
