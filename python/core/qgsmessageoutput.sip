

/**
\brief Interface for showing messages from QGIS in GUI independent way

This class provides abstraction of a dialog for showing output to the user.
By default QgsMessageConsoleOutput will be used if not overridden
with other message output creator function.

QGIS application uses QgsMessageView class for displaying a dialog to the user.

Object deletes itself when it's not needed anymore. Children should use
signal destroyed() to notify the deletion
*/
class QgsMessageOutput
{
%TypeHeaderCode
#include <qgsmessageoutput.h>
%End

  public:
    
    //! message can be in plain text or in html format
    enum MessageType { MessageText, MessageHtml };
    
    //! virtual destructor
    virtual ~QgsMessageOutput();

    //! set message, it won't be displayed until 
    virtual void setMessage(const QString& message, MessageType msgType) = 0;
    
    //! message to be appended to the current text
    virtual void appendMessage(const QString& message) = 0;
    
    //! set title for the messages
    virtual void setTitle(const QString& title) = 0;
    
    //! display the message to the user
    virtual void showMessage(bool blocking = true) = 0;
    
    //! sets function that will be used to create message output
    // TODO: implementation where python class could be passed
    // static void setMessageOutputCreator(MESSAGE_OUTPUT_CREATOR f);
    
    //! function that returns new class derived from QgsMessageOutput
    //! (don't forget to delete it then)
    static QgsMessageOutput* createMessageOutput();

};


/**
\brief Default implementation of message output interface
           
This class outputs messages to the standard output. Therefore it might
be the right choice for apps without GUI.
*/
class QgsMessageOutputConsole : QObject, QgsMessageOutput
{
%TypeHeaderCode
#include <qgsmessageoutput.h>
%End

  public:
    
    QgsMessageOutputConsole();
    
    virtual void setMessage(const QString& message, MessageType msgType);
    
    virtual void appendMessage(const QString& message);
    
    virtual void setTitle(const QString& title);
    
    //! sends the message to the standard output
    virtual void showMessage(bool blocking = true);
    
  signals:
    
    //! signals that object will be destroyed and shouldn't be used anymore
    void destroyed();

};

