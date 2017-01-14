import os
import glob
import sys

cpp = open(sys.argv[1], "w")
cpp.write(
    "#include \"qgscontexthelp.h\"\n"
    "#include <QCoreApplication>\n\n"
    "QHash<QString, QString> QgsContextHelp::sContextHelpTexts;\n"
    "\n"
    "void QgsContextHelp::init()\n"
    "{\n"
    "  if( !sContextHelpTexts.isEmpty() )\n"
    "    return;\n"
)

for f in sorted(glob.glob('resources/context_help/*')):
    n = os.path.basename(f)
    if os.path.isdir(f):
        # Protect from IOError: [Errno 21] Is a directory
        continue
    with open(f) as content:
        cpp.write("\n  sContextHelpTexts.insert( \"{0}\", QCoreApplication::translate( \"context_help\", \"{1}\") );".format(
            n, content.read().replace("\\", "&#92;").replace('\\', '\\\\').replace('"', '\\"').replace('\n', '\\n"\n\"')))

cpp.write("\n}\n")
cpp.close()
