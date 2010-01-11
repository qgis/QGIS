/**
 * @file main.cpp
 * @brief Main file.
 * @author Micha³ Policht
 */

#include <QApplication>
#include <qextserialport.h>

#include "defs.h"
#include "MainWindow.h"
#include "MessageWindow.h"


int main(int argc, char *argv[])
{
	int exec;

	QApplication app(argc, argv);
    //redirect debug messages to the MessageWindow dialog
	qInstallMsgHandler(MessageWindow::AppendMsgWrapper);

	MainWindow mainWindow(APP_TITLE);
	mainWindow.show();
	exec = app.exec();
	return exec;
}


