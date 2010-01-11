/* qesptest.h
**************************************/
#ifndef _QESPTEST_H_
#define _QESPTEST_H_

#include <QWidget>

class QLineEdit;
class QTextEdit;
class QextSerialPort;
class QSpinBox;

class QespTest :  public QWidget
{
  Q_OBJECT
public:
  QespTest(QWidget *parent=0);

  virtual ~QespTest();

private:
  QLineEdit *message;
  QSpinBox* delaySpinBox;
  QTextEdit *received_msg;
  QextSerialPort *port;

private slots:
  void transmitMsg();
  void receiveMsg();
  void appendCR();
  void appendLF();
  void closePort();
  void openPort();
};

#endif
