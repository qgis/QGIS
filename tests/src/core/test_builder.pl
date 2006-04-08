#!/usr/bin/perl
use Cwd;

#####################################################
# A script to automate creation of a new unit test
# Authors TSutton
# April 08, 2006
#####################################################
# $Id: plugin_builder.pl 5212 2006-04-07 23:21:38Z timlinux $ #

#make sure we are in a the tests/src/core dir 
$myDir = fastgetcwd;
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
print "\n\nEnter the name of the class for which the test will be created.\n";
print "Used mixed case notation.\n";
print "e.g. QgsSymbol\n";
$testClass =<STDIN>;
chop $testClass;
$testClassLowerCaseName = lc($testClass); #todo convert to lower case 
print "Create the unit test? [y/n]: ";
$createIt = <STDIN>;
chop $createIt;

if(($createIt eq 'y') || ($createIt eq 'Y'))
{
  #
  # its a go -- create the unit test and modify the build files
  #
  system("cp test_template.cpp test$testClassLowerCaseName.cpp");

  # Substitute the class name in the file
  system("perl -pi -e 's/\\\[testClassLowerCaseName\\\]/$testClassLowerCaseName/g' test$testClassLowerCaseName.cpp");
  system("perl -pi -e 's/\\\[testClassCamelCaseName\\\]/$testClass/g' test$testClassLowerCaseName.cpp");
  #
  # TODO: write a parser to pull out method signatures from class bing tested and create a stub
  # for each method in the generated test class to replace teh [TestMethods] placeholder
  #

  # Add an entry to Makefile.am
  open MAKEFILE, "<./Makefile.am" || die 'Unable to open Makefile.am';
  open MAKEFILEMOD, ">./Makefile.am.mod" || die 'Unable to create Makefile.am.mod';
  # read through Makefile.am and write each line to Makefile.am.mod
  while(<MAKEFILE>)
  {
    if(/^\s*bin_PROGRAMS =*/)
    {
      # add our application binary name to the next line
      print MAKEFILEMOD "\\"; 
      print MAKEFILEMOD "\t\t$testClassLowerCaseName \n";
    }
    else
    {
      print MAKEFILEMOD;
    }
    if(/^\s*BUILT_SOURCES =*/)
    {
      # add our application binary name to the next line
      print MAKEFILEMOD "\\"; 
      print MAKEFILEMOD "\t\t${testClassLowerCaseName}_MOC \n";
    }
    else
    {
      print MAKEFILEMOD;
    }
  }
  #before closing the file add the lines for our new test class
  print MAKEFILEMOD "test${testClassLowerCaseName}_MOC = test${testClassLowerCaseName}.moc.cpp";
  print MAKEFILEMOD "test${testClassLowerCaseName}_SOURCES = ${testClassLowerCaseName}.cpp";
  print MAKEFILEMOD "test${testClassLowerCaseName}_LDADD = $(GLOBALLDADD)";
  print MAKEFILEMOD "test${testClassLowerCaseName}_CXXFLAGS =  $(GLOBALCXXFLAGS)";

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

