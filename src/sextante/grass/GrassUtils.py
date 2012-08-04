import os
from sextante.core.SextanteUtils import SextanteUtils, mkdir
import subprocess
from sextante.core.SextanteConfig import SextanteConfig
from sextante.core.SextanteLog import SextanteLog
import stat
import shutil
import plugin_installer

class GrassUtils:

    GRASS_LATLON = "GRASS_LATLON"
    #GRASS_AUTO_REGION = "GRASS_AUTO_REGION"
    GRASS_REGION_XMIN = "GRASS_REGION_XMIN"
    GRASS_REGION_YMIN = "GRASS_REGION_YMIN"
    GRASS_REGION_XMAX = "GRASS_REGION_XMAX"
    GRASS_REGION_YMAX = "GRASS_REGION_YMAX"
    GRASS_REGION_CELLSIZE = "GRASS_REGION_CELLSIZE"
    GRASS_FOLDER = "GRASS_FOLDER"
    GRASS_HELP_FOLDER = "GRASS_HELP_FOLDER"
    GRASS_WIN_SHELL = "GRASS_WIN_SHELL"
    GRASS_LOG_COMMANDS = "GRASS_LOG_COMMANDS"
    GRASS_LOG_CONSOLE = "GRASS_LOG_CONSOLE"

    @staticmethod
    def grassBatchJobFilename():
        '''This is used in linux. This is the batch job that we assign to
        GRASS_BATCH_JOB and then call GRASS and let it do the work'''
        filename = "grass_batch_job.sh";
        batchfile = SextanteUtils.userFolder() + os.sep + filename
        return batchfile

    @staticmethod
    def grassScriptFilename():
        '''this is used in windows. We create a script that initializes
        GRASS and then uses grass commands'''
        filename = "grass_script.bat";
        filename = SextanteUtils.userFolder() + os.sep + filename
        return filename

    @staticmethod
    def grassPath():
        if not SextanteUtils.isWindows() and not SextanteUtils.isMac():
            return ""

        folder = SextanteConfig.getSetting(GrassUtils.GRASS_FOLDER)
        if folder == None:
            if SextanteUtils.isWindows():
                folder = plugin_installer.__file__
                idx = folder.find('qgis')
                folder = folder[:idx] + "grass"
                if not os.path.isdir(folder):
                    return ""
                for subfolder in os.listdir(folder):
                    if subfolder.startswith("grass"):
                        folder = folder + os.sep + subfolder
                        break
            else:
                return "/Applications/GRASS-6.4.app/Contents/MacOS"

        return folder

    @staticmethod
    def grassHelpPath():
        folder = SextanteConfig.getSetting(GrassUtils.GRASS_HELP_FOLDER)
        if folder == None or folder == "":
            if SextanteUtils.isWindows():
                testfolders = [os.path.join(GrassUtils.grassPath(), "docs", "html")]
            else:
                testfolders = ['/usr/share/doc/grass-doc/html']
            for f in testfolders:
                if os.path.exists(f):
                    folder = f
                    break

        return folder

    @staticmethod
    def grassWinShell():
        folder = SextanteConfig.getSetting(GrassUtils.GRASS_WIN_SHELL)
        if folder == None:
            folder = plugin_installer.__file__
            idx = folder.find('qgis')
            folder = folder[:idx] + "msys"

        return folder

    @staticmethod
    def grassDescriptionPath():
        return os.path.join(os.path.dirname(__file__), "description")

    @staticmethod
    def createGrassScript(commands):
        folder = GrassUtils.grassPath()
        shell = GrassUtils.grassWinShell()

        script = GrassUtils.grassScriptFilename()
        gisrc =  SextanteUtils.userFolder() + os.sep + "sextante.gisrc"

        #temporary gisrc file
        output = open(gisrc, "w")
        location = "temp_location"
        mapset = "user"
        gisdbase = os.path.join(os.path.expanduser("~"), "sextante", "tempdata", "grassdata")
        output.write("GISDBASE: " + gisdbase + "\n");
        output.write("LOCATION_NAME: " + location + "\n");
        output.write("MAPSET: " + mapset + "\n");
        output.write("GRASS_GUI: text\n");
        output.close();

        output=open(script, "w")
        output.write("set HOME=" + os.path.expanduser("~") + "\n");
        output.write("set GISRC=" + gisrc + "\n")
        output.write("set GRASS_SH=" + shell + "\\bin\\sh.exe\n")
        output.write("set PATH=" + shell + os.sep + "bin;" + shell + os.sep + "lib;" + "%PATH%\n")
        output.write("set WINGISBASE=" + folder + "\n")
        output.write("set GISBASE=" + folder + "\n");
        output.write("set GRASS_PROJSHARE=" + folder + os.sep + "share" + os.sep + "proj" + "\n")
        output.write("set GRASS_MESSAGE_FORMAT=gui\n")
        #Replacement code for etc/Init.bat
        output.write("if \"%GRASS_ADDON_PATH%\"==\"\" set PATH=%WINGISBASE%\\bin;%WINGISBASE%\\lib;%PATH%\n")
        output.write("if not \"%GRASS_ADDON_PATH%\"==\"\" set PATH=%WINGISBASE%\\bin;%WINGISBASE%\\lib;%GRASS_ADDON_PATH%;%PATH%\n")
        output.write("\n")
        output.write("set GRASS_VERSION=" + GrassUtils.getGrassVersion() + "\n");
        output.write("if not \"%LANG%\"==\"\" goto langset\n");
        output.write("FOR /F \"usebackq delims==\" %%i IN (`\"%WINGISBASE%\\etc\\winlocale\"`) DO @set LANG=%%i\n");
        output.write(":langset\n")
        output.write("\n")
        output.write("set PATHEXT=%PATHEXT%;.PY\n")
        output.write("set PYTHONPATH=%PYTHONPATH%;%WINGISBASE%\\etc\\python;%WINGISBASE%\\etc\\wxpython\\n");
        output.write("\n")
        output.write("g.gisenv.exe set=\"MAPSET=" + mapset + "\"\n")
        output.write("g.gisenv.exe set=\"LOCATION=" + location + "\"\n")
        output.write("g.gisenv.exe set=\"LOCATION_NAME=" + location + "\"\n")
        output.write("g.gisenv.exe set=\"GISDBASE=" + gisdbase + "\"\n")
        output.write("g.gisenv.exe set=\"GRASS_GUI=text\"\n")
        for command in commands:
            output.write(command + "\n")
        output.write("\n");
        output.write("exit\n");
        output.close();

    @staticmethod
    def createGrassBatchJobFileFromGrassCommands(commands):
        fout = open(GrassUtils.grassBatchJobFilename(), "w")
        for command in commands:
            fout.write(command + "\n")
        fout.write("exit")
        fout.close()

    @staticmethod
    def grassMapsetFolder():
        tempfolder = os.path.join(os.path.expanduser("~"), "sextante", "tempdata", "grassdata", "temp_location")
        mkdir(tempfolder)
        return tempfolder


    @staticmethod
    def createTempMapset():
        '''Creates a temporary location and mapset(s) for GRASS data processing. A minimal set of folders and files is created in the
         system's default temporary directory. The settings files are written with sane defaults, so GRASS can do its work. File
        structure and content will vary slightly depending on whether the user wants to process lat/lon or x/y data.'''

        latlon = SextanteConfig.getSetting(GrassUtils.GRASS_LATLON)
        folder = GrassUtils.grassMapsetFolder()
        mkdir(os.path.join(folder, "PERMANENT"))
        mkdir(os.path.join(folder, "user"))
        mkdir(os.path.join(folder, "PERMANENT", ".tmp"))
        GrassUtils.writeGrassWindow(os.path.join(folder, "PERMANENT", "DEFAULT_WIND"));
        outfile = open(os.path.join(folder, "PERMANENT", "MYNAME"), "w")
        if not latlon:
            outfile.write("SEXTANTE GRASS interface: temporary x/y data processing location.\n");
        else:
            outfile.write("SEXTANTE GRASS interface: temporary lat/lon data processing location.\n");
        outfile.close();
        if latlon:
            outfile = open(os.path.join(folder, "PERMANENT", "PROJ_INFO"), "w")
            outfile.write("name: Latitude-Longitude\n")
            outfile.write("proj: ll\n")
            outfile.write("ellps: wgs84\n")
            outfile.close()
            outfile = open(os.path.join(folder, "PERMANENT", "PROJ_UNITS"), "w")
            outfile.write("unit: degree\n");
            outfile.write("units: degrees\n");
            outfile.write("meters: 1.0\n");
            outfile.close();
        GrassUtils.writeGrassWindow(os.path.join(folder, "PERMANENT", "WIND"));
        mkdir(os.path.join(folder, "user", "dbf"))
        mkdir(os.path.join(folder, "user", ".tmp"))
        outfile = open(os.path.join(folder, "user", "VAR"), "w")
        outfile.write("DB_DRIVER: dbf\n");
        outfile.write("DB_DATABASE: $GISDBASE/$LOCATION_NAME/$MAPSET/dbf/\n");
        outfile.close()
        GrassUtils.writeGrassWindow(os.path.join(folder, "user", "WIND"));

    @staticmethod
    def writeGrassWindow(filename):
        out = open(filename, "w")
        latlon = SextanteConfig.getSetting(GrassUtils.GRASS_LATLON)
        if not latlon:
            out.write("proj:       0\n");
            out.write("zone:       0\n");
            out.write("north:      1\n");
            out.write("south:      0\n");
            out.write("east:       1\n");
            out.write("west:       0\n");
            out.write("cols:       1\n");
            out.write("rows:       1\n");
            out.write("e-w resol:  1\n");
            out.write("n-s resol:  1\n");
            out.write("top:        1\n");
            out.write("bottom:     0\n");
            out.write("cols3:      1\n");
            out.write("rows3:      1\n");
            out.write("depths:     1\n");
            out.write("e-w resol3: 1\n");
            out.write("n-s resol3: 1\n");
            out.write("t-b resol:  1\n");
        else:
            out.write("proj:       3\n");
            out.write("zone:       0\n");
            out.write("north:      1N\n");
            out.write("south:      0\n");
            out.write("east:       1E\n");
            out.write("west:       0\n");
            out.write("cols:       1\n");
            out.write("rows:       1\n");
            out.write("e-w resol:  1\n");
            out.write("n-s resol:  1\n");
            out.write("top:        1\n");
            out.write("bottom:     0\n");
            out.write("cols3:      1\n");
            out.write("rows3:      1\n");
            out.write("depths:     1\n");
            out.write("e-w resol3: 1\n");
            out.write("n-s resol3: 1\n");
            out.write("t-b resol:  1\n");
        out.close()


    @staticmethod
    def executeGrass(commands, progress):
        if SextanteUtils.isWindows():
            GrassUtils.createGrassScript(commands)
            command = ["cmd.exe", "/C ", GrassUtils.grassScriptFilename()]
        else:
            gisrc =  SextanteUtils.userFolder() + os.sep + "sextante.gisrc"
            os.putenv("GISRC", gisrc)
            os.putenv("GRASS_MESSAGE_FORMAT", "gui")
            os.putenv("GRASS_BATCH_JOB", GrassUtils.grassBatchJobFilename())
            GrassUtils.createGrassBatchJobFileFromGrassCommands(commands)
            os.chmod(GrassUtils.grassBatchJobFilename(), stat.S_IEXEC | stat.S_IREAD | stat.S_IWRITE)
            if SextanteUtils.isMac():
                command = GrassUtils.grassPath() + os.sep + "grass.sh " + GrassUtils.grassMapsetFolder() + "/user"
            else:
                command = "grass64 " + GrassUtils.grassMapsetFolder() + "/user"
        loglines = []
        loglines.append("GRASS execution console output")
        proc = subprocess.Popen(command, shell=True, stdout=subprocess.PIPE, stdin=subprocess.PIPE,stderr=subprocess.STDOUT, universal_newlines=True).stdout
        for line in iter(proc.readline, ""):
            if "GRASS_INFO_PERCENT" in line:
                try:
                    progress.setPercentage(int(line[len("GRASS_INFO_PERCENT")+ 2:]))
                except:
                    pass
            else:
                loglines.append(line)
        if SextanteConfig.getSetting(GrassUtils.GRASS_LOG_CONSOLE):
            SextanteLog.addToLog(SextanteLog.LOG_INFO, loglines)
        shutil.rmtree(GrassUtils.grassMapsetFolder(), True)

    @staticmethod
    def getGrassVersion():
        #I do not know if this should be removed or let the user enter it
        #or something like that... This is just a temporary thing
        return "6.4.0"







