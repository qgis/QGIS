/**
 * @file MainWindow.h
 * @brief Application's Main Window.
 * @see MainWindow
 * @author Micha³ Policht
 */


#ifndef MAINWINDOW_H_
#define MAINWINDOW_H_


#include <QMainWindow>

class QMenu;
class QAction;


/**
 * Application's Main Window.
 */
class MainWindow : public QMainWindow
{
	Q_OBJECT
	
	/** 
	 * @name Menu Bar 
	 */
//@{
	QMenu *fileMenu;
		QAction *exitAct;
	QMenu *helpMenu;
		QAction *aboutAct;
//@}
	
	
	private:
		/**
		 * Create menus.
		 */
		void createMenus();
		
		/**
		 * Create actions.
		 */
		void createActions();
		
	private slots:
		/**
		 * About application popup.
		 */
		void about();
		
	public:
		/**
		 * MainWindow default constructor.
		 * 	@param title window title.
		 */
		MainWindow(const QString &title);
		
};

#endif /*MAINWINDOW_H_*/

