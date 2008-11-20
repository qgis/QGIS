
class QgsMessageViewer: QDialog, QgsMessageOutput //, Ui::QgsMessageViewer
{
%TypeHeaderCode
#include <qgsmessageviewer.h>
%End

  public:
    QgsMessageViewer(QWidget *parent = 0, Qt::WFlags fl = QgisGui::ModalDialogFlags);
    ~QgsMessageViewer();
    
    virtual void setMessage(const QString& message, MessageType msgType);
    
    virtual void appendMessage(const QString& message);
    
    virtual void showMessage(bool blocking = true);

    virtual void setTitle(const QString& title);

    // Call one of the setMessage...() functions first.
    // Subsequent calls to appendMessage use the format as determined
    // by the call to setMessage...()

    // Treats the given text as html.
    void setMessageAsHtml(const QString& msg);
    // Treats the given text as plain text
    void setMessageAsPlainText(const QString& msg);
    // A checkbox that can be used for something like 
    // "don't show this message again"
    void setCheckBoxText(const QString& text);
    // Make the check box visible/invisible
    void setCheckBoxVisible(bool visible);
    // Sets the check state
    void setCheckBoxState(Qt::CheckState state);
    // The state of the checkbox
    Qt::CheckState checkBoxState();
    // Specifies a QSettings tag to store/retrieve the checkbox
    // state to/from. Use an empty QString to disable this feature.
    void setCheckBoxQSettingsLabel( QString label );
};

