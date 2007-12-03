/***************************************************************************
     qgsgrassshell.cpp
     --------------------------------------
    Date                 : Sun Sep 16 12:06:10 AKDT 2007
    Copyright            : (C) 2007 by Gary E. Sherman
    Email                : sherman at mrcc dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <iostream>
#include <vector>

#include <qstring.h>
#include <qapplication.h>
#include <qpushbutton.h>
#include <qwidget.h>
#include <q3textedit.h>
#include <q3process.h>
#include <qmessagebox.h>
#include <q3cstring.h>
#include <qfile.h>
#include <qdatastream.h>
#include <qstringlist.h>
#include <qsocketnotifier.h>
#include <q3socket.h>
#include <q3socketdevice.h>
#include <qevent.h>
#include <q3textbrowser.h>
#include <qregexp.h>
#include <qcursor.h>
#include <qlayout.h>
#include <qclipboard.h>
#include <qfontmetrics.h>
#include <q3progressbar.h>

#include "qgsapplication.h"

#include "qgsgrassshell.h"
//Added by qt3to4:
#include <QGridLayout>
#include <QKeyEvent>
#include <QResizeEvent>
#include <QMouseEvent>

extern "C" {
#include <stdio.h>
#include <stdlib.h>
#ifndef _MSC_VER
#include <unistd.h>
#else
#include <io.h>
#endif

#ifndef WIN32
#ifdef Q_OS_MACX
#include <util.h>
#else
#ifdef __NetBSD__
#include <util.h>
#else
#ifdef __FreeBSD__
#include <termios.h>
#include <libutil.h>
#else
#include <pty.h>
#endif
#endif
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>
#endif //!WIN32
}

QgsGrassShell::QgsGrassShell ( QgsGrassTools *tools, 
    QTabWidget * parent, const char * name  ):
QDialog(parent), QgsGrassShellBase(), mTools(tools)
{
  mValid = false;
  mSkipLines = 2;
  mTabWidget = parent;

#ifdef WIN32
  QMessageBox::warning( 0, "Warning", 
      "GRASS Shell is not supported on Windows." );
  return;
#else 

  setupUi(this);

  QGridLayout *layout = new QGridLayout( mTextFrame, 1, 1 );
  mText = new QgsGrassShellText( this, mTextFrame);
  layout->addWidget ( mText, 0 , 0 );
  mText->show();
  
  connect(mCloseButton, SIGNAL(clicked()), this, SLOT(closeShell()));

  mFont = QFont ( "Courier", 10 );

  mAppDir = mTools->appDir();

#ifndef Q_WS_MAC
  // Qt4.3.2/Mac Q3TextEdit readOnly property causes keys to be processed as keyboard actions
  mText->setReadOnly(TRUE);
#endif
  //mText->setFocusPolicy ( QWidget::NoFocus ); // To get key press directly

#ifndef HAVE_OPENPTY
  mText->append ( "GRASS shell is not supported" ); 
  return;
#endif

  // TODO set cursor IbeamCursor
  // This does not work - the cursor is used for scrollbars -> disabled
  //mText->setCursor ( QCursor(Qt::IbeamCursor) );
  mParagraph = -1; // first will be 0
  mIndex = -1;

  mNewLine = true;

  for ( int i = 0; i < ModeCount; i++ )
  {
    resetMode(i);
  }

  int uid;
  seteuid( uid=getuid() );	/* Run unprivileged */

  // Get and open pseudo terminal 
  // Note: 0 (stdin), 1 (stdout) or 2 (stderr)
  int fdSlave; // slave file descriptor
  seteuid(0);
  int ret =  openpty ( &mFdMaster, &fdSlave, NULL, NULL, NULL );
  if ( ret != 0 )
  {
    QMessageBox::warning( 0, "Warning", "Cannot open pseudo terminal" );
    return;
  }
  fchown( fdSlave, uid, (gid_t)-1);
  fchmod( fdSlave, S_IRUSR | S_IWUSR);
  seteuid(uid);

#ifdef QGISDEBUG
  std::cerr << "mFdMaster = " << mFdMaster << std::endl;
  std::cerr << "fdSlave = " << fdSlave << std::endl;
#endif

  fcntl( mFdMaster, F_SETFL, O_NDELAY); 
  //fcntl( fdSlave, F_SETFL, O_NDELAY); // enable? 

  QString slaveName = ttyname(fdSlave);
#ifdef QGISDEBUG
  std::cerr << "master ttyname = " << ttyname(mFdMaster)  << std::endl;
  std::cerr << "slave ttyname = " << ttyname(fdSlave)  << std::endl;
#endif

  //close ( fdSlave ); // -> crash  

  // Fork slave and open shell 
  int pid = fork();
#ifdef QGISDEBUG
  std::cerr << "pid = " << pid << std::endl;
#endif
  if ( pid == -1 )
  {
    QMessageBox::warning( 0, "Warning", "Cannot fork shell" );
    return;
  }

  // Child - slave
  if ( pid == 0 ) 
  {
#ifdef QGISDEBUG
    std::cerr << "child ->" << std::endl;
#endif

    // TODO close all opened file descriptors - close(0)???
#ifndef Q_OS_DARWIN
    // Makes child process unusable on Mac
    close ( mFdMaster );
#endif

    //close ( fdSlave ); // -> freeze  

    setsid();
    seteuid(0);

    int fd = open ( (char*) slaveName.ascii(), O_RDWR);
    if ( fd < 0 ) 
    {
      QMessageBox::warning( 0, "Warning", "Cannot open slave file "
          "in child process" );
      return;
    }

    fchown(fd, uid, (gid_t)-1);
    fchmod(fd, S_IRUSR | S_IWUSR);
    setuid(uid);

    dup2 (fd, 0);  /* stdin */
    dup2 (fd, 1);  /* stdout */
    dup2 (fd, 2);  /* stderr */

    // TODO: test if shell is available
    QString shell = ( getenv("SHELL") );
    if ( shell.isEmpty() )
    {
      shell = "/bin/bash";
    }

    char *norc = "";
    QFileInfo si(shell);
    if ( si.fileName() ==  "bash" || si.fileName() ==  "sh" )
    { 
      norc = "--norc";
    }
    else if ( si.fileName() ==  "tcsh" || si.fileName() ==  "csh" )
    { 
      norc = "-f";
    }

    // Warning: execle + --norc will not inherit not given variables
    // -> overwrite here
    char *env = "GRASS_MESSAGE_FORMAT=gui";
    char *envstr = new char[strlen(env)+1];
    strcpy ( envstr, env );
    putenv( envstr );

    putenv ( "GISRC_MODE_MEMORY" );  // unset

    env = "PS1=GRASS > ";
    envstr = new char[strlen(env)+1];
    strcpy ( envstr, env );
    putenv( envstr );

    env = "TERM=vt100";
    envstr = new char[strlen(env)+1];
    strcpy ( envstr, env );
    putenv( envstr );

    //char *envar[] = { "PS1=GRASS > ", "TERM=vt100", "GISRC_MODE_MEMORY=", 
    //                  "GRASS_MESSAGE_FORMAT=gui", (char *)0 };

    //execle ( (char*)shell.ascii(), (char *)si.fileName().ascii(), 
    //         norc, (char *) 0, envar);
    execl ( (char*)shell.ascii(), (char *)si.fileName().ascii(), 
        norc, (char *) 0);

    // Failed (QMessageBox here does not work)
    fprintf ( stderr, "GRASS_INFO_ERROR(1,1): Cannot start shell %s\n", 
        (char*)shell.ascii() );
    exit(1);
  }

  mPid = pid;

  // Create socket notifier
  mOutNotifier = new QSocketNotifier ( mFdMaster, QSocketNotifier::Read, this);

  QObject::connect ( mOutNotifier, SIGNAL(activated(int)),
      this, SLOT(readStdout(int)));

  // Set tab stops ???
  mTabStop.resize(200);
  for ( int i = 0 ; i * 8 < (int)mTabStop.size(); i++ )
  {
    mTabStop[i*8] = true;
  }

  // Set trap to write history on SIGUSR1
  //QString trap = "trap 'history -w' SIGUSR1\015\012";
  QString trap = "trap 'history -w' SIGUSR1\015";
  write( mFdMaster, trap.ascii(), trap.length());
  mText->clear();

  resizeTerminal();
  mValid = true;
#endif // !WIN32
}

QgsGrassShell::~QgsGrassShell()
{
#ifdef QGISDEBUG
  std::cerr << "QgsGrassShell::~QgsGrassShell()" << std::endl;
#endif

#ifndef WIN32
  // This was old trick to write history
  /*
     write( mFdMaster, "exit\015\012", 6);
     while ( 1 ) 
     {
     readStdout(0);

     int status;
     if ( waitpid ( mPid, &status, WNOHANG ) > 0 ) break;

     struct timespec t, r;
     t.tv_sec = 0;
     t.tv_nsec = 10000000; // 0.01 s
     nanosleep ( &t, &r );
     }
     */

  // Write history
  if ( kill(mPid,SIGUSR1) == -1 )
  {
    std::cerr << "cannot write history (signal SIGUSR1 to pid = " << mPid << ")" << std::endl;
  }

  std::cerr << "kill shell pid = " << mPid << std::endl;
  if ( kill(mPid,SIGTERM ) == -1 )
  {
    std::cerr << "cannot kill shell pid = " << mPid << std::endl;
  }
#endif
}

void QgsGrassShell::keyPressEvent( QKeyEvent * e  )
{
#ifdef QGISDEBUG
  std::cerr << "QgsGrassShell::keyPressEvent()" << std::endl;
#endif

  char s[10];
  int length = 0;
  int ret = 0;

  if ( !mValid ) return;

  mProgressBar->setProgress ( 0, 100 );

  char c = (char) e->ascii();
#ifdef QGISDEBUG
  std::cerr << "c = " << (int)c << " key = " << e->key() 
    << " text = " << e->text().local8Bit().data() << std::endl;
#endif
  s[0] = c;
  length = 1;

  // Set key down 
  if ( e->key() == Qt::Key_Control ) mKeyDown[DownControl] = true;
  else if ( e->key() == Qt::Key_Shift ) mKeyDown[DownShift] = true;
  else if ( e->key() == Qt::Key_Alt ) mKeyDown[DownAlt] = true;
  else if ( e->key() == Qt::Key_Meta ) mKeyDown[DownMeta] = true;

  if ( c == 0 )
  {
    switch ( e->key() )
    {
      case Qt::Key_Up :
        strcpy ( s, "\033[A" );
        length = 3;
        break;

      case Qt::Key_Down :
        strcpy ( s, "\033[B" );
        length = 3;
        break;

      case Qt::Key_Right :
        strcpy ( s, "\033[C" );
        length = 3;
        break;

      case Qt::Key_Left :
        strcpy ( s, "\033[D" );
        length = 3;
        break;
    }
  }

  ret = write( mFdMaster, s, length);
#ifdef QGISDEBUG
  std::cerr << "write ret = " << ret << std::endl;
#endif
}

void QgsGrassShell::keyReleaseEvent( QKeyEvent * e  )
{
#ifdef QGISDEBUG
  //    std::cerr << "QgsGrassShell::keyReleaseEvent()" << std::endl;
#endif

  // Reset key down 
  if ( e->key() == Qt::Key_Control ) mKeyDown[DownControl] = false;
  else if ( e->key() == Qt::Key_Shift ) mKeyDown[DownShift] = false;
  else if ( e->key() == Qt::Key_Alt ) mKeyDown[DownAlt] = false;
  else if ( e->key() == Qt::Key_Meta ) mKeyDown[DownMeta] = false;
}

void QgsGrassShell::readStdout( int socket )
{
#ifdef QGISDEBUG
  //std::cerr << "QgsGrassShell::readStdout()" << std::endl;
#endif

  char buf[4097];
  int len; 
  while ( (len = read( mFdMaster, buf, 4096)) > 0 )
  {	
    // Terminate string
    buf[len] = '\0';

    mStdoutBuffer.append ( buf );
  }

  printStdout();
}

void QgsGrassShell::printStdout()
{
  // Debug
#ifdef QGISDEBUG
  std::cerr << "****** buffer ******" << std::endl;
  std::cerr << "-->";
  for ( int i = 0; i < (int)mStdoutBuffer.length(); i++ )
  {
    int c = mStdoutBuffer[i];
    QString s = "";
    if ( c > '\037' && c != '\177' ) // control characters
    {
      s = (char) c;
      std::cerr << s.local8Bit().data();
    }
    else
    {
      std::cerr << "(c=" << QString::number(c,8).local8Bit().data() << ")";
    }
  }
  std::cerr << "<--" << std::endl;
#endif

  eraseCursor();
  // To make it faster we want to print maximum lenght blocks from buffer
  while ( mStdoutBuffer.length() > 0 ) 
  {
#ifdef QGISDEBUG
    std::cerr << "------ cycle ------" << std::endl;
#endif

    // Search control character
    int control = -1; 
    for ( int i = 0; i < (int)mStdoutBuffer.length(); i++ )
    {
      int c = mStdoutBuffer[i];
      if ( c < '\037' || c == '\177' )
      {
        control = i;
        break;
      }
    }
#ifdef QGISDEBUG
    std::cerr << "control = " << control << std::endl;
#endif

    // Process control character if found at index 0
    if ( control == 0 ) 
    {
      int c = mStdoutBuffer[0];
#ifdef QGISDEBUG
      std::cerr << "c = " << QString::number(c,8).local8Bit().data() << std::endl;
#endif

      // control sequence
      if ( c == '\033' )
      {
        //std::cerr << "control sequence" << std::endl;

        bool found = false;

        // It is sequence, so it should be at least one more character
        // wait for more data 
        if ( mStdoutBuffer.length() < 2 ) break;
        if ( mStdoutBuffer[1] == ']' && mStdoutBuffer.length() < 3 ) break;

        // ESC ] Ps ; Pt BEL    (xterm title hack)
        QRegExp rx ( "\\](\\d+);([^\\a]+)\\a" ); 
        if ( rx.search ( mStdoutBuffer, 1 ) == 1 ) 
        {
          int mlen = rx.matchedLength();
#ifdef QGISDEBUG
          std::cerr << "ESC(set title): " << rx.cap(2).local8Bit().data() << std::endl;
#endif
          mStdoutBuffer.remove ( 0, mlen+1 );
          found = true;
        }

        if ( !found ) 
        {
          //    ESC [ Pn ; Pn FINAL
          // or ESC [ = Pn ; Pn FINAL
          // or ESC [ = Pn ; Pn FINAL
          // TODO: QRegExp captures only last of repeated patterns 
          //       ( ; separated nums - (;\\d+)* )
          rx.setPattern ( "\\[([?=])*(\\d+)*(;\\d+)*([A-z])" ); 
          if ( rx.search ( mStdoutBuffer, 1 ) == 1 ) 
          {
            int mlen = rx.matchedLength();
            char final = rx.cap(4).at(0).latin1();

            std::cerr << "final = " << final << std::endl;
            //std::cerr << "ESC: " << rx.cap(0) << std::endl;

            switch ( final )
            {
              case 'l' : // RM - Reset Mode
              case 'h' : // SM - Set Mode
                {
                  int mode = -1;
                  switch ( rx.cap(2).toInt() )
                  {
                    case 4 :
                      mode = Insert;
                      break;

                    default:
                      std::cerr << "ESC ignored: " << rx.cap(0).local8Bit().data() << std::endl;
                      break;
                  }
                  if ( mode >= 0 )
                  {
                    if ( final == 'l' )
                      resetMode ( mode );
                    else
                      setMode ( mode );
                  }
                  break;
                }

              case 'm' : // SGR - Select Graphic Rendition
                if ( rx.cap(2).isEmpty() || rx.cap(2).toInt() == 0 )
                {
                  for ( int i = 0; i < RendetionCount; i++ )
                  {
                    mRendetion[i] = false;
                  }
                }
                else
                {
                  std::cerr << "ESC SGR ignored: " << rx.cap(0).local8Bit().data() << std::endl;
                }
                break;

              case 'P' : // DCH - Delete Character
                {
                  int n = rx.cap(2).toInt();
                  mText->setSelection ( mParagraph, mIndex, mParagraph, mIndex+n, 0 );
                  mText->removeSelectedText ( 0 );
                  break;
                }

              case 'K' : // EL - Erase In Line
                if ( rx.cap(2).isEmpty() || rx.cap(2).toInt() == 0 )
                {
                  //mText->setSelectionAttributes ( 1, QColor(255,255,255), true );  
                  mText->setSelection ( mParagraph, mIndex, mParagraph, 
                      mText->paragraphLength(mParagraph), 0 );
                  mText->removeSelectedText ( 0 );
                }
                break;

                // TODO: multiple tab stops
              case 'H' : // Horizontal Tabulation Set (HTS)
                mTabStop[mIndex] = true;
                std::cerr << "TAB set on " << mIndex << std::endl;
                break;

              case 'g' : // Tabulation Clear (TBC)
                // ESC [ g 	Clears tab stop at the cursor
                // ESC [ 2 g 	Clears all tab stops in the line
                // ESC [ 3 g 	Clears all tab stops in the Page
                std::cerr << "TAB reset" << std::endl;
                if ( rx.cap(2).isEmpty() || rx.cap(2).toInt() == 0 )
                {
                  mTabStop[mIndex] = false;
                } 
                else
                {
                  for (int i = 0; i < (int)mTabStop.size(); i++ ) 
                    mTabStop[mIndex] = false;
                }
                break;

              default:
                std::cerr << "ESC ignored: " << rx.cap(0).local8Bit().data() << std::endl;
                break;
            }

            mStdoutBuffer.remove ( 0, mlen+1 );
            found = true;
          }
        }

        if ( !found ) 
        {
          // ESC # DIGIT
          rx.setPattern ( "#(\\d)" ); 
          if ( rx.search ( mStdoutBuffer, 1 ) == 1 )
          {
            std::cerr << "ESC ignored: " << rx.cap(0).local8Bit().data() << std::endl;
            mStdoutBuffer.remove ( 0, 3 );
            found = true;
          }
        }

        if ( !found ) 
        {
          // ESC CHARACTER
          rx.setPattern ( "[A-z<>=]" ); 
          if ( rx.search ( mStdoutBuffer, 1 ) == 1 )
          {
            std::cerr << "ESC ignored: " << rx.cap(0).local8Bit().data() << std::endl;
            mStdoutBuffer.remove ( 0, 2 );
            found = true;
          }
        }

        // TODO: it can happen that the sequence is not complete ->
        //       no match -> how to distinguish unknown sequence from
        //       missing characters
        if ( !found ) 
        {
          // For now move forward
          std::cerr << "UNKNOWN ESC ignored: " << mStdoutBuffer.mid(1,5).data() << std::endl;
          mStdoutBuffer.remove ( 0, 1 );
        }
      }
      else
      {
        // control character
        switch ( c ) 
        {
          case '\015' : // CR
            //std::cerr << "CR" << std::endl;
            mStdoutBuffer.remove ( 0, 1 );
            // TODO : back tab stops?
            mIndex = 0;
            break;

          case '\012' : // NL
            //std::cerr << "NL" << std::endl;
            newLine();
            mStdoutBuffer.remove ( 0, 1 );
            break;

          case '\010' : // BS 
            //std::cerr << "BS" << std::endl;
            mIndex--;
            mStdoutBuffer.remove ( 0, 1 );
            break;

          case '\011' : // HT (tabulator)
            {
              //std::cerr << "HT" << std::endl;
              QString space;
              for ( int i = mIndex; i < (int)mTabStop.size(); i++ )
              {
                space.append ( " " );
                if ( mTabStop[i] ) break;
              }
              insert (space);
              mStdoutBuffer.remove ( 0, 1 );
              break;
            }

          case '>' : // Keypad Numeric Mode 
            std::cerr << "Keypad Numeric Mode ignored: " 
              << QString::number(c,8).local8Bit().data() << std::endl;
            mStdoutBuffer.remove ( 0, 2 );
            break;

          default : // unknown control, do nothing
            std::cerr << "UNKNOWN control char ignored: " << QString::number(c,8).local8Bit().data() << std::endl;
            mStdoutBuffer.remove ( 0, 1 );
            break;
        }
      }
      continue;
    }

    // GRASS messages. GRASS messages start with GRASS_INFO_
    // and stop with \015\012 (\n)

    // first info
    QRegExp rxinfo ( "GRASS_INFO_" );
    int message = rxinfo.search(mStdoutBuffer);

    if ( message == 0 ) // Info found at index 0
    { 
      // First try percent
      QRegExp rxpercent ( "GRASS_INFO_PERCENT: (\\d+)\\015\\012" );
      if ( rxpercent.search(mStdoutBuffer) == 0 ) {
        int mlen = rxpercent.matchedLength();
        int progress = rxpercent.cap(1).toInt();
        mProgressBar->setProgress ( progress, 100 );
        mStdoutBuffer.remove ( 0, mlen );
        continue;
      }

      QRegExp rxwarning ( "GRASS_INFO_WARNING\\(\\d+,\\d+\\): ([^\\015]*)\\015\\012" );
      QRegExp rxerror ( "GRASS_INFO_ERROR\\(\\d+,\\d+\\): ([^\\015]*)\\015\\012" );
      QRegExp rxend ( "GRASS_INFO_END\\(\\d+,\\d+\\)\\015\\012" );

      int mlen = 0; 
      QString msg;
      QString img;
      if ( rxwarning.search(mStdoutBuffer) == 0 ) 
      {
        mlen = rxwarning.matchedLength();
        msg = rxwarning.cap(1);
        img = QgsApplication::pkgDataPath() + "/themes/default/grass/grass_module_warning.png";
      }
      else if ( rxerror.search(mStdoutBuffer) == 0 ) 
      {
        mlen = rxerror.matchedLength();
        msg = rxerror.cap(1);
        img = QgsApplication::pkgDataPath() + "/themes/default/grass/grass_module_error.png";
      }

      if ( mlen > 0 ) // found error or warning
      {
#ifdef QGISDEBUG
        std::cerr << "MSG: " << msg.local8Bit().data() << std::endl;
#endif

        // Delete all previous empty paragraphs.
        // Messages starts with \n (\015\012) which is previously interpreted 
        // as new line, so it is OK delete it, but it is not quite correct
        // to delete more  because it can be regular module output
        // but it does not look nice to have empty rows before 
        removeEmptyParagraphs();

        msg.replace ( "&", "&amp;" );
        msg.replace ( "<", "&lt;" );
        msg.replace ( ">", "&gt;" );
        msg.replace ( " ", "&nbsp;" );

        mText->setTextFormat(Qt::RichText);
        mText->append ( "<img src=\"" + img + "\">" + QString::fromLocal8Bit( msg ) );
        mParagraph++;
        mNewLine = true;
        mStdoutBuffer.remove ( 0, mlen );
        continue;
      } 

      if ( rxend.search(mStdoutBuffer) == 0 ) 
      {
        mlen = rxend.matchedLength();
        mStdoutBuffer.remove ( 0, mlen );
        continue;
      }

      // No complete message found => wait for input 
      // TODO: 1) Sleep for a moment because GRASS writes 
      //          1 character in loop
      //       2) Fix GRASS to write longer strings
      break;
    }

    // Print plain text
    int length = mStdoutBuffer.length();
    if ( control >= 0 ) length = control;
    if ( message >= 0 && (control == -1 || control > message ) )
    {
      length = message;
    }

    if ( length > 0 ) 
    {
      QString out = mStdoutBuffer.left ( length ) ;
#ifdef QGISDEBUG
      std::cerr << "TXT: '" << out.local8Bit().data()<< "'" << std::endl;
#endif

      insert ( QString::fromLocal8Bit( out ) );

      mStdoutBuffer.remove ( 0, length );
    }
  }

  showCursor();
  mText->ensureCursorVisible();
}

void QgsGrassShell::removeEmptyParagraphs()
{
  while ( mParagraph >= 0 
      && mText->text(mParagraph).stripWhiteSpace().length() <= 0 )
  {
    mText->removeParagraph ( mParagraph );
    mParagraph--;
  }
  mIndex = mText->paragraphLength(mParagraph);
}

void QgsGrassShell::insert ( QString s )
{
#ifdef QGISDEBUG
  std::cerr << "insert()" << std::endl;
#endif

  if ( s.isEmpty() ) return; 

  // In theory mParagraph == mText->paragrephs()-1
  // but if something goes wrong (more paragraphs) we want to write 
  // at the end
  if ( mParagraph > -1 && mParagraph != mText->paragraphs()-1 )
  {
    std::cerr << "WRONG mParagraph!" << std::endl;
    mNewLine = true;
  }

  // Bug?: QTextEdit::setOverwriteMode does not work, always 'insert'
  //       -> if Insert mode is not set, delete first the string 
  //          to the right
  // mText->setOverwriteMode ( !mMode[Insert] ); // does not work
  if ( !mMode[Insert] && !mNewLine && mParagraph >= 0 &&
      mText->paragraphLength(mParagraph) > mIndex ) 
  {
#ifdef QGISDEBUG
    std::cerr << "erase old " << mIndex+s.length() << " chars "  << std::endl;
#endif
    mText->setSelection ( mParagraph, mIndex, mParagraph, mIndex+s.length(), 0 );
    mText->removeSelectedText ( 0 );
  }

  if ( mNewLine )
  {
    // Start new paragraph
    mText->setTextFormat(Qt::PlainText);
    mText->setCurrentFont ( mFont ); 
    mText->append ( s );
    mIndex = s.length();
    //mParagraph++;
    mParagraph = mText->paragraphs()-1;
    mNewLine = false;
  }
  else
  {
    // Append to existing paragraph
    mText->setCursorPosition ( mParagraph, mIndex );
    mText->setTextFormat(Qt::PlainText);
    mText->setCurrentFont ( mFont ); 
    mText->insert ( s );

    mIndex += s.length(); 
  }
}

void QgsGrassShell::newLine()
{
  if ( mSkipLines > 0 )
  {
    mText->clear();
    mSkipLines--;
  }
  if ( mNewLine ) 
  {
    mText->setTextFormat(Qt::PlainText);
    mText->setCurrentFont ( mFont ); 
    mText->append ( " " );
    //mParagraph++; 
    // To be sure that we are at the end
    mParagraph = mText->paragraphs()-1;
    mIndex = 0;
  }
  mNewLine = true;
}

void QgsGrassShell::eraseCursor()
{
  // Remove space representing cursor from the end of current paragraph
  if ( !mNewLine && mCursorSpace && mParagraph >= 0 ) 
  {
    mText->setSelection ( mParagraph, mIndex, mParagraph, mIndex+1, 0 );
    mText->removeSelectedText ( 0 );
  }
  mCursorSpace = false;
}

void QgsGrassShell::showCursor()
{
  // Do not highlite cursor if last printed paragraph was GRASS message
  if ( mNewLine ) return;

  // If cursor is at the end of paragraph add space  
  if ( mParagraph >= 0 && mIndex > mText->paragraphLength(mParagraph)-1 )
  {
    mText->setCursorPosition(mParagraph,mIndex);
    mText->setCursorPosition ( mParagraph, mIndex );
    mText->insert ( " " );
    // Warning: do not increase mIndex, 
    // the space if after current position
    mCursorSpace = true;
  }

  // Selection 1 is used as cursor highlite
  mText->setSelection ( mParagraph, mIndex, mParagraph, mIndex+1, 1 );
  mText->setSelectionAttributes ( 1, QColor(0,0,0), true );  
}

void QgsGrassShell::mousePressEvent(QMouseEvent* e)
{
#ifdef QGISDEBUG
  std::cerr << "mousePressEvent()" << std::endl;
#endif

  if ( !mValid ) return;

  // paste clipboard
  if ( e->button() == Qt::MidButton )
  {
    QClipboard *cb = QApplication::clipboard();
    QString text = cb->text(QClipboard::Selection);
    write( mFdMaster, (char*) text.ascii(), text.length() );
  }
}

void QgsGrassShell::resizeTerminal()
{
#ifndef WIN32
  int width = mText->visibleWidth(); 
  int height = mText->visibleHeight(); 

  QFontMetrics fm ( mFont );
  int col = (int) (width / fm.width("x")); 
  int row = (int) (height / fm.height()); 

  struct winsize winSize;
  memset(&winSize, 0, sizeof(winSize));
  winSize.ws_row = row;
  winSize.ws_col = col;

  ioctl( mFdMaster, TIOCSWINSZ, (char *)&winSize );
#endif
}

void QgsGrassShell::readStderr()
{
}

void QgsGrassShell::closeShell()
{
#ifdef QGISDEBUG
  std::cerr << "QgsGrassShell::closeShell()" << std::endl;
#endif
  
  mTabWidget->removePage (this );
  delete this;
}


QgsGrassShellText::QgsGrassShellText ( QgsGrassShell *gs, 
    QWidget * parent, const char *name )
: Q3TextEdit (parent,name),
mShell(gs)
{
}

QgsGrassShellText::~QgsGrassShellText() {} 

void QgsGrassShellText::contentsMousePressEvent(QMouseEvent* e)
{
#ifdef QGISDEBUG
  std::cerr << "contentsMousePressEvent()" << std::endl;
#endif
  mShell->mousePressEvent(e);
  Q3TextEdit::contentsMousePressEvent(e);
}

void QgsGrassShellText::keyPressEvent ( QKeyEvent * e )
{
#ifdef QGISDEBUG
  std::cerr << "QgsGrassShellText::keyPressEvent()" << std::endl;
#endif
  mShell->keyPressEvent(e);
}

void QgsGrassShellText::keyReleaseEvent ( QKeyEvent * e )
{
#ifdef QGISDEBUG
  std::cerr << "QgsGrassShellText::keyReleaseEvent()" << std::endl;
#endif
  mShell->keyReleaseEvent(e);
}

void QgsGrassShellText::resizeEvent(QResizeEvent *e)
{
#ifdef QGISDEBUG
  std::cerr << "resizeEvent()" << std::endl;
#endif
  mShell->resizeTerminal();
  Q3TextEdit::resizeEvent(e);
}
