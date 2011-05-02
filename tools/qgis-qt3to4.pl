#!/usr/bin/perl

#
# WARNING: PROOF OF CONCEPT ONLY.  DO NOT RUN UNLESS YOU KNOW WHAT YOU ARE DOING.
# REQUIRES QTDIR to be set!! [gsherman]
#

#
# This program will convert the given directory from Qt3 code
# to Qt4-with-Qt3Support code.
#
# Run this on a backup of your CVS working directory repository
# first, and don't commit any of the converted files!
# At least not until the QGIS developer community are ready to
# sever ties with the Qt3 libraries.
#
# Synopsis:
# This perl script will run the qt3to4 program included in the
# Qt4 distribution.  Then it will do several more involved fixups
# that qt3to4 missed.
#
# It assumes that all .cpp and .h files in the directory are to
# be converted (sorry, Makefiles and *.pro files are ignored).
#
# $Id$
#

sub RunQt3to4
{
  my($directory, $direntry) = @_;

  if (-f ($directory.$direntry.".portinglog.txt"))
  {
    print "$directory$direntry was already processed by qt3to4.\n";
    return;
  }

  # Copy old version for backup purposes
  my($cmd) = "cp $directory$direntry $directory$direntry.qt3.old";
  print "About to run '$cmd'\n";
  `$cmd`;

  my($cmd) = "$ENV{'QTDIR'}/bin/qt3to4 -alwaysOverwrite $directory$direntry";
  print "About to run '$cmd'\n";
  `$cmd`;

  $cmd = "mv portinglog.txt ".$directory.$direntry.".portinglog.txt";
  print "About to run '$cmd'\n";
  `$cmd`;


}

sub Qt3to4File
{
  my($filename) = @_;

  print "Doing custom conversions to '$filename' ...\n";

  open(CPP, $filename) || die "Can't open file $filename: $!";
  my($cppqt4) = "";

  while (<CPP>)
  {
    my($line) = $_;

    # Start applying coding hacks:

    # String substitutions:

    # 1. Use of QStrings in std::ostream operator<< context
    #
    # Qt3:
    #   QString foo;  foo.ascii();
    #
    # Qt4:
    #   QString foo;  foo.toAscii().data();

    # Restricted to filenames as we now use "local8Bit" in ostreams and it also breaks QKeyEvent's "ascii()"
    if ($filename =~ /qgsrasterlayer\.cpp$/)
    {
      $line =~ s/ascii\(\)/toAscii\(\)\.data\(\)/g;
    }

    # 1a. Use of QStrings in std::ostream operator<< context
    #
    # Qt3:
    #   QString foo;  foo.local8Bit();
    #
    # Qt4:
    #   QString foo;  foo.toLocal8Bit().data();

    $line =~ s/local8Bit\(\)/toLocal8Bit\(\)\.data\(\)/g;

    # 2. Fix qt3to4's use of QImageIO to QPictureIO
    #
    # Qt3:
    #   #include <QImageIO>
    #   QImageIO::outputFormats().count();
    #
    # Qt4:
    #   #include <QPictureIO>
    #   QPictureIO::outputFormats().count();

    $line =~ s/\#include \<QImageIO\>/\#include \<QPictureIO\>/;

    $line =~ s/QImageIO\:\:/QPictureIO\:\:/g;

    # 2.a. QImage scale
    #
    # Qt3:
    #   QImage foo;
    #   foo.scale(10,10);
    #
    # Qt4:
    #   QImage foo;
    #   foo.scaled(10,10);

    # Commented out in favour of using Qt version ifdef
    # since this broke QMatrix.scale()
    #$line =~ s/\.scale\(/\.scaled\(/g;

    # 3. Fix qt3to4's oversight of QCanvas to Q3Canvas
    #
    # Qt3:
    #   #include <qcanvas.h>
    #   class QgsComposerView: public QCanvasView
    #
    # Qt4:
    #   #include <Q3CanvasView>
    #   class QgsComposerView: public Q3CanvasView

    $line =~ s/\#include \<qcanvas.h\>/\#include \<Q3CanvasView\>/;

    $line =~ s/public QCanvasView/public Q3CanvasView/g;


    # Menu substitutions:

    # 4. Qt3 QPopupMenu vs. Qt4 QMenu
    #
    # Qt3:
    #   QPopupMenu *foo;
    #   foo->indexOf(a);
    #
    # Qt4:
    #   QMenu *foo;
    #   foo->actions().indexOf(a);

    # for this one, use specific cases, not a general "indexOf" grep, as it may be too wide a net
    # Commented out in favour of using Qt version ifdef
    #$line =~ s/popupMenuFile\-\>indexOf/popupMenuFile\-\>actions\(\)\.indexOf/g;


    # FileInfo substitutions:

    # 5. Base Names
    #
    # Qt3:
    #   QFileInfo foo;
    #   foo.baseName(TRUE);
    #
    # Qt4:
    #   QFileInfo foo;
    #   foo.completeBaseName();

    $line =~ s/baseName\(TRUE\)/completeBaseName\(\)/g;


    # QIODevice substitutions:

    # 5.a.a. end-of-line terminators
    #
    # Qt3:
    #   QFile foo;
    #   foo.open(IO_Translate);
    #
    # after qt3to4:
    #   QFile foo;
    #   foo.open(QIODevice::Translate);
    #
    # Qt4:
    #   QFile foo;
    #   foo.open(QIODevice::Text);

    $line =~ s/QIODevice\:\:Translate/QIODevice\:\:Text/g;


    # Widget substitutions:

    # 5.a. StrongFocus
    #
    # Qt3:
    #   QWidget foo;
    #   foo.setFocusPolicy(QWidget::StrongFocus);
    #
    # Qt4:
    #   QWidget foo;
    #   foo.setFocusPolicy(Qt::StrongFocus);

    $line =~ s/QWidget\:\:StrongFocus/Qt\:\:StrongFocus/g;


    # QProgressDialog substitutions:

    # 5.b. wasCancelled - now American spelling
    #
    # Qt3:
    #   QProgressDialog foo;
    #   if (foo.wasCancelled()) { ... ]
    #
    # Qt4:
    #   Q3ProgressDialog foo;
    #   if (foo.wasCanceled()) { ... ]

    $line =~ s/\.wasCancelled\(\)/\.wasCanceled\(\)/g;


    # Additional items for specific files

    # 6. Add <QDesktopWidget> include to qgisapp.cpp AND splashscreen.cpp

    if (
        ($filename =~ /qgisapp\.cpp$/) or
        ($filename =~ /splashscreen\.cpp$/)
       )
    {
      if ($line =~ /Added by qt3to4\:/)  # Good as place as any to add it
      {
        $line .= "#include <QDesktopWidget>\n";
      }
    }

    # 6a. Add <QTextOStream> include to qgscoordinatetransform.cpp

    if ($filename =~ /qgscoordinatetransform\.cpp$/)
    {
      if ($line =~ /Qt4-only includes to go here/)  # Good as place as any to add it
      {
        $line .= "#include <QTextOStream>\n";
      }
    }

    # 6b. Add <QStringList> include to qgsprojectproperty.cpp

    if ($filename =~ /qgsprojectproperty\.cpp$/)
    {
      if ($line =~ /Qt4-only includes to go here/)  # Good as place as any to add it
      {
        $line .= "#include <QStringList>\n";
      }
    }

    # 7. Mop up overzealous color conversions by qt3to4 to qgscontinuouscolrenderer.cpp
    # Qt::red -> red,
    # Qt::green -> green,
    # Qt::blue -> blue.

    if ($filename =~ /qgscontinuouscolrenderer\.cpp$/)
    {
      $line =~ s/Qt\:\:red/red/g;
      $line =~ s/Qt\:\:green/green/g;
      $line =~ s/Qt\:\:blue/blue/g;
    }

    # End of substitutions
    $cppqt4 .= $line;

  }
  close(CPP);

  open(CPPQT4, ">$filename") || die "Can't create file $filename: $!";
  print CPPQT4 $cppqt4;
  close(CPPQT4);
}

sub Qt3to4UicFile
{
  my($filename) = @_;

  print "Doing custom UIC conversions to '$filename' ... ";

  open(UIC, $filename) || die "Can't open file $filename: $!";
  my($uicqt4) = "";

  while (<UIC>)
  {
    my($line) = $_;

    # Start applying coding hacks:

    # String substitutions:

    # 1. Remove "menubar->setFrameShape(QMenuBar::MenuBarPanel)"
    next if ( $line =~ /menubar\-\>setFrameShape\(QMenuBar\:\:MenuBarPanel\)/ );

    # 2. Remove "menubar->setFrameShadow(QMenuBar::Raised)"
    next if ( $line =~ /menubar\-\>setFrameShadow\(QMenuBar\:\:Raised\)/ );

    $uicqt4 .= $line;
  }
  close(UIC);

  # Rename old version for backup purposes
  my($cmd) = "mv $filename $filename.qt3.old";
  print "About to run '$cmd'\n";
  `$cmd`;

  open(UICQT4, ">$filename") || die "Can't overwrite file $filename: $!";
  print UICQT4 $uicqt4;
  close(UICQT4);

  print "Done.\n";
}

sub ProcessQt3to4
{
  my($directory, $direntry) = @_;

  &RunQt3to4($directory, $direntry);
  &Qt3to4File($directory.$direntry);
}

sub ProcessQt3to4Uic
{
  my($filename) = @_;

  &Qt3to4UicFile($filename);
}

sub ParseDirectory
{
  my($directory) = @_;

  if (!$directory)
  {
    $directory = "./";
  }

  if ($directory !~ /\/$/)     # no trailing slash
  {
    $directory .= "/";         # add slash
  }

  print "Opening $directory\n";

  opendir(DIR, $directory) || die "Can't open directory $directory: $!";

  my(@direntry) = ();
  my($direntry);

  while ($direntry = readdir(DIR))
  {
    push(@direntry, $direntry);
  }

  closedir(DIR);

  foreach $direntry (@direntry)
  {

    print "Checking directory entry '$directory$direntry' ...\n";

    if (-d $directory.$direntry)
    {
      # Is a directory, recurse if not ./ or ../
      if (
          ($direntry ne ".") and
          ($direntry ne "..")
         )
      {
        &ParseDirectory("$directory$direntry/");
      }
    }

    if (-f $directory.$direntry)
    {
      if ($direntry !~ /\.(moc|uic)\./)    # not moc or uic compiled files
      {
        if ($direntry =~ /.*\.cpp$/)    # ends with ".cpp"
        {
          print "  Found a .cpp file.\n";
          &ProcessQt3to4($directory, $direntry);
        }

        if ($direntry =~ /.*\.h$/)    # ends with ".h"
        {
          print "  Found an .h file.\n";
          &ProcessQt3to4($directory, $direntry);
        }

      }
    }
  }

}

# Check for QTDIR and exit if not set
if(length($ENV{'QTDIR'}) == 0){
  print<<EOF
  QTDIR must be set to the Qt 4 directory:
  export QTDIR=/my/path/to/qt
  You should also set the PATH to use the Qt 4 binaries:
  PATH=\$QTDIR/bin:\$PATH

  QTDIR not set -- exiting.
EOF
  ;

exit;
}
print "Starting $0...\n";

if ($ARGV[0] eq "-uic")
{
  # .uic.h one-shot conversion mode
  &Qt3to4UicFile($ARGV[1]);
}
else
{
  # .cpp and .h bulk-conversion mode
  &ParseDirectory($ARGV[0]);
}

print "$0 complete.\n";

#
# ENDS
#
