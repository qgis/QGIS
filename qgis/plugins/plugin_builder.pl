#!/usr/bin/perl

#####################################################
# A script to automate creation of a new QGIS plugin
# using the plugin_template
# Authors GSherman TSutton
# Feb 21, 2004
#####################################################
# $Id$ #

# get the needed information from the user
print "Directory for the new plugin: ";
$pluginDir =<STDIN>;
chop $pluginDir;

print "Plugin name: " ;
$pluginName = <STDIN>;
chop $pluginName;

print "Plugin description: " ;
$pluginDescription = <STDIN>;
chop $pluginDescription;

print "Menu name: ";
$menuName = <STDIN>;
chop $menuName;

print "Menu item name: ";
$menuItemName = <STDIN>;
chop $menuItemName;

# print a summary of what's about to happen
print << "EOF";

Summary of plugin parameters:
---------------------------------------------
Plugin directory      $pluginDir
Name of the plugin:   $pluginName
Menu name:            $menuName
Menu item name:       $menuItemName

Warning - Proceeding will make changes to Makefile.am in this directory,
as well as ../configure.in. Please use caution.
EOF
# ask if we should proceed
print "Create the plugin? [y/n]: ";
$createIt = <STDIN>;
chop $createIt;

if(($createIt eq 'y') || ($createIt eq 'Y')){
  #
  # its a go -- create the plugin and modify the build files
  #
  # create the new plugin directory
  system("cp -r plugin_template $pluginDir");
  # remove the CVS directory 
  system("rm -rf $pluginDir/CVS");
  # substitute the plugin specific vars in the various files
  # this is a brute force approach but its quick and dirty :)
  system("perl -pi -e 's/\\\[pluginname\\\]/$pluginName/g' $pluginDir/*.cpp");
  system("perl -pi -e 's/\\\[pluginname\\\]/$pluginName/g' $pluginDir/*.h");
  system("perl -pi -e 's/\\\[pluginname\\\]/$pluginName/g' $pluginDir/*.am");
 
  system("perl -pi -e 's/\\\[plugindescription\\\]/$pluginDescription/g' $pluginDir/*.cpp");
  system("perl -pi -e 's/\\\[plugindescription\\\]/$pluginDescription/g' $pluginDir/*.h");
  system("perl -pi -e 's/\\\[plugindescription\\\]/$pluginDescription/g' $pluginDir/*.am");
  
  system("perl -pi -e 's/\\\[menuname\\\]/$menuName/g' $pluginDir/*.cpp");
  system("perl -pi -e 's/\\\[menuname\\\]/$menuName/g' $pluginDir/*.h");
  system("perl -pi -e 's/\\\[menuname\\\]/$menuName/g' $pluginDir/*.am");
  
  system("perl -pi -e 's/\\\[menuitemname\\\]/$menuItemName/g' $pluginDir/*.cpp");
  system("perl -pi -e 's/\\\[menuitemname\\\]/$menuItemName/g' $pluginDir/*.h");
  system("perl -pi -e 's/\\\[menuitemname\\\]/$menuItemName/g' $pluginDir/*.am");
  
  # Add an entry to qgis/plugins/Makefile.am
  # We won't add it the EXTRA_DIST since we don't want to necesarily distribute
  # third party plugins
  open MAKEFILE, "<./Makefile.am" || die 'Unable to open Makefile.am';
  open MAKEFILEMOD, ">./Makefile.am.mod" || die 'Unable to create Makefile.am.mod';
  # read through Makefile.am and write each line to Makefile.am.mod
  while(<MAKEFILE>){
    if(/^\s*SUBDIRS =*/){
      # add our plugin dir to the SUBDIRS line
      chop;
      print MAKEFILEMOD;
      print MAKEFILEMOD " $pluginDir\n";
    }else{
      print MAKEFILEMOD;
    }
  }
  # close the Makefile file handles
  close MAKEFILEMOD;
  close MAKEFILE;
  
  # save Makefile.am in case we die before done moving things around
  system("mv Makefile.am Makefile.am.save");
  # move the new Makefile.am to where it belongs
  system("mv Makefile.am.mod Makefile.am");
  # delete the original Makefile.am
  unlink("Makefile.am.save");

  # Add an entry to qgis/configure.in
  # Do we really want to do this or add a message telling the user how to do
  # it?
  open CONFIGUREIN, "<../configure.in" || die 'Unable to open ../configure.in';
  open CONFIGUREINMOD, ">../configure.in.mod" || die 'Unable to create ../configure.in.mod';
  # read through configure.in until we find the AC_CONFIG_FILES section
   while(<CONFIGUREIN>){
    if(/^\s*AC_CONFIG_FILES*/){
      # set the flag so we can look for the closing ])
      $inConfigFile = 1;
      print CONFIGUREINMOD;
      
    }else{
      if($inConfigFile){
        if(/^\s*\]\)*/){
          # write our entry 
          print CONFIGUREINMOD "\tplugins/$pluginDir/Makefile\n";
          $inConfigFile = 0;
        }
      }
      print CONFIGUREINMOD;
    }
  }
  close CONFIGUREIN;
  close CONFIGUREINMOD;
  
   # save configure.in in case we die before done moving things around
  system("mv ../configure.in ../configure.in.save");
  # move the new configure.in to where it belongs
  system("mv ../configure.in.mod ../configure.in");
  # delete the original configure.in
  unlink("../configure.in.save");
  
# print out some end of processing info
print << "EOP";

Your plugin ($pluginName) has been created in $pluginDir.
Makefile.am and configure.in have been modified.
To build the plugin, you must change to the top level of the source tree and
run autoreconf, configure, then make.

EOP

}else{
  # user cancelled
  print "Plugin not created\n";
}

