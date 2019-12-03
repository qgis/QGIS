CONFIG      += qt warn_on thread
CONFIG      += release

contains (QT_VERSION, ^5.*){
	QT += widgets
}

# Comment the lines bellow if you want to build libqemf statically
#CONFIG      += QEmfDll

	    
	    

