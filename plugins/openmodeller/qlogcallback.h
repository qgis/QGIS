#ifndef QLOGCALLBACK_H
#define QLOGCALLBACK_H
#include <openmodeller/Log.hh>
#include <string>
#include <qtextstream.h>
#include <qtextbrowser.h>
/** Although the call back needs a text stream, all we want is to get the msg.c_str() 
 * from the logger, so having the textstream is just so that we fit the 
 * g_log callback model, but its not actually used anywhere
 */
class QLogCallback : public Log::LogCallback {

    public:
        QLogCallback( QTextStream& qtos ) 
        {}

        void operator()(Log::Level level, const std::string& msg ) 
        {
          textBrowser->append(msg.c_str());
          textBrowser->scrollToBottom ();
          //std::cout << msg.c_str();
        }

        void setTextBrowser(QTextBrowser *theTextBrowser)
        {
          textBrowser = theTextBrowser;

        }
    private:
        QTextBrowser * textBrowser;

};
#endif //LOGCALLBACK_H
