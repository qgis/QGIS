/**
 * @file MessageWindow.h
 * @brief Message Window.
 * @see MessageWindow
 * @author Micha³ Policht
 */

#ifndef MESSAGEWINDOW_H_
#define MESSAGEWINDOW_H_


#include <QDockWidget>
#include <QTextEdit>
#include <QEvent>

/**
 * Message Window. Handling errors and other messages.
 */
class MessageWindow: public QDockWidget
{
	Q_OBJECT

	QTextEdit msgTextEdit;				///< Main widget.
	static MessageWindow* MsgHandler;	///< Set in constructor.
	static const char* WINDOW_TITLE; 	///< Window title.

	private:
		static QString QtMsgToQString(QtMsgType type, const char *msg);
	
	protected:
		/**
		 * Handle custom events. MessageWindow hadles custom events listed in
		 * EventType enum.
		 */
		virtual void customEvent(QEvent* event);
		
	public:
		enum EventType {MessageEvent = QEvent::User};	///< Custom event types.
		
		/**
		 * Default constructor.
		 * 	@param parent parent widget.
		 * 	@param flags widget flags.
		 */
		MessageWindow(QWidget* parent = 0, Qt::WFlags flags = 0);

		/**
		 * Append message wrapper. Since ISO forbids casting member functions
		 * to C functions, wrapper is needed to use this class as QtMsgHandler.
		 * This method is thread-safe but not reentrant.
		 * 	@param type message type.
		 * 	@param msg message string.
		 */
		static void AppendMsgWrapper(QtMsgType type, const char *msg);
		
		/**
		 * Post message event to the main event loop. This function encapsulates
		 * message into MessageEvent object and passes it to the main event loop.
		 * 	@param type message type.
		 * 	@param msg message string.
		 */
		void postMsgEvent(QtMsgType type, const char *msg);
	
};


/**
 * Message Event. Custom event used by @ref MessageWindow to provide multi-threaded 
 * access. Encapsulates message inside @a msg variable.
 */
class MessageEvent: public QEvent
{
	public:
		QString msg;	///< Message string.
		
		/**
		 * Contructor.
		 * 	@param msg message to post.
		 */
		MessageEvent(QString & msg);
};

#endif /*MESSAGEWINDOW_H_*/
