#!/usr/bin/perl

#
# WARNING: PROOF OF CONCEPT ONLY.  DO NOT RUN UNLESS YOU KNOW WHAT YOU ARE DOING.
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

  my($cmd) = "/usr/local/Trolltech/Qt-4.0.0/bin/qt3to4 -alwaysOverwrite $directory$direntry";
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

    $line =~ s/\.ascii\(\)/\.toAscii\(\)\.data\(\)/g;

    # 1a. Use of QStrings in std::ostream operator<< context
    #
    # Qt3:
    #   QString foo;  foo.local8Bit();
    #
    # Qt4:
    #   QString foo;  foo.toLocal8Bit().data();

    $line =~ s/\.local8Bit\(\)/\.toLocal8Bit\(\)\.data\(\)/g;

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
