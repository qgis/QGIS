#!/usr/bin/perl
#***************************************************************************
#    test_builder.pl
#    --------------------------------------
#   Date                 : Sun Sep 16 12:22:00 AKDT 2007
#   Copyright            : (C) 2007 by Gary E. Sherman
#   Email                : sherman at mrcc dot com
#***************************************************************************
#*                                                                         *
#*   This program is free software; you can redistribute it and/or modify  *
#*   it under the terms of the GNU General Public License as published by  *
#*   the Free Software Foundation; either version 2 of the License, or     *
#*   (at your option) any later version.                                   *
#*                                                                         *
#***************************************************************************/

use Cwd;

#####################################################
# A script to automate creation of a new unit test
# Authors TSutton
# April 08, 2006
#####################################################
# $Id: plugin_builder.pl 5212 2006-04-07 23:21:38Z timlinux $ #

#make sure we are in a the tests/src/core dir 
$myDir = fastgetcwd;
$sourceDir = "../../../src/core";
print "\n\nChecking that we are in the <qgis dir>/test/src/core/ directory....";
if ($myDir =~ m/tests\/src\/core$/) 
{
  print "yes\n";
}
else 
{
  print "no\n";
  print $myDir;
  print "\nPlease relocate to the /test/src/core/ directory before attempting to run this script.\n";
  exit;
}
# get the needed information from the user
$testClass="";
$argCount = $#ARGV+1;
if ($argCount > 0)
{
  $testClass=@ARGV[ 0 ];
}
else
{
  print "\n\nEnter the name of the class for which the test will be created.\n";
  print "Used mixed case notation.\n";
  print "e.g. QgsSymbol\n";
  $testClass =<STDIN>;
  chop $testClass;
}

$testClassLowerCaseName = lc($testClass); #todo convert to lower case 

#
# Check source file is ok
#

if ($testClass eq "") 
{
  print "ClassName not supplied ...exiting...";
  exit;
}
print "Checking if source class exists in filesystem ...";
if (-e "${sourceDir}/${testClassLowerCaseName}.cpp" ) 
{
  print "yes\n";
}
else
{
  print "no, exiting\n";
  print "${sourceDir}/${testClassLowerCaseName}.cpp does not exist!\n";
  exit;
}


print "Stubs will be created for the following methods:\n";
open CPPFILE, "<$sourceDir/$testClassLowerCaseName.cpp"|| die 'Unable to open header file $testClassLowerCaseName.cpp';
$stubString="";
$lastLine="";
while(<CPPFILE>)
{
  if(m/${testClass}::[A-Za-z0-9]*\(/)
  {
    #get the matched part of the line
    $line = $&;
    #strip off the ::
    $line =~ s/:://g;
    #strip off the (
    $line =~ s/\(//g;
    if ($lastLine eq $line)
    {
      #duplicate entry
    }
    else
    {
      #add it to our stub code
      $stubString = $stubString . "    void $line()\n\{\n\n\};\n";
      #show the user the list
      print $line;
      print "\n";
    }
    $lastLine=$line;
  }
}
$createIt="n";
if ($argCount eq 0)
{
  print "-----------------------------\n";  
  print "Create the unit test? [y/n]: ";
  $createIt = <STDIN>;
  chop $createIt;
}
else
{
  $createIt="y";
}

if(($createIt eq 'y') || ($createIt eq 'Y'))
{
  #
  # its a go -- create the unit test and modify the build files
  #
  system("cp test_template.cpp test$testClassLowerCaseName.cpp");

  # Substitute the class name in the file
  system("perl -pi -e 's/\\\[testClassLowerCaseName\\\]/$testClassLowerCaseName/g' test$testClassLowerCaseName.cpp");
  system("perl -pi -e 's/\\\[testClassCamelCaseName\\\]/$testClass/g' test$testClassLowerCaseName.cpp");
  system("perl -pi -e 's/\\\[TestMethods\\\]/$stubString/g' test$testClassLowerCaseName.cpp");
  # Add an entry to Makefile.am
  open MAKEFILE, "<./Makefile.am" || die 'Unable to open Makefile.am';
  open MAKEFILEMOD, ">./Makefile.am.mod" || die 'Unable to create Makefile.am.mod';
  # read through Makefile.am and write each line to Makefile.am.mod
  while(<MAKEFILE>)
  {
    if(/^\s*bin_PROGRAMS =*/)
    {
      # add our application binary name to the next line
      print MAKEFILEMOD;
      print MAKEFILEMOD "\t\ttest$testClassLowerCaseName \\\n";
    }
    elsif(/^\s*BUILT_SOURCES =*/)
    {
      # add our application binary name to the next line
      print MAKEFILEMOD;
      print MAKEFILEMOD "\t\t\$(test${testClassLowerCaseName}_MOC) \\\n";
    }
    else
    {
      print MAKEFILEMOD;
    }
  }
  #before closing the file add the lines for our new test class
  print MAKEFILEMOD "\n";
  print MAKEFILEMOD "test${testClassLowerCaseName}_MOC = test${testClassLowerCaseName}.moc.cpp\n";
  print MAKEFILEMOD "test${testClassLowerCaseName}_SOURCES = test${testClassLowerCaseName}.cpp\n";
  print MAKEFILEMOD "test${testClassLowerCaseName}_LDADD = \$(GLOBALLDADD)\n";
  print MAKEFILEMOD "test${testClassLowerCaseName}_CXXFLAGS =  \$(GLOBALCXXFLAGS)\n";

  # close the Makefile file handles
  close MAKEFILEMOD;
  close MAKEFILE;

  # save Makefile.am in case we die before done moving things around
  system("mv Makefile.am Makefile.am.save");
  # move the new Makefile.am to where it belongs
  system("mv Makefile.am.mod Makefile.am");
  # delete the original Makefile.am
  unlink("Makefile.am.save");


  print << "EOP";

Your test unit has been created now as ${testClassLowerCaseName}.cpp.

EOP

}
else
{
  # user cancelled
  print "Test unit not created\n";
}

