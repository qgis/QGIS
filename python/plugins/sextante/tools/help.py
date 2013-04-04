from sextante.core.Sextante import Sextante
import os
from sextante.core.SextanteUtils import mkdir
from sextante.parameters.ParameterSelection import ParameterSelection

def createBaseHelpFile(alg, folder):
    folder = os.path.join(folder, alg.provider.getName().lower())
    mkdir(folder)
    cmdLineName = alg.commandLineName()[alg.commandLineName().find(":") + 1:].lower()
    validChars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"
    safeFilename = ''.join(c for c in cmdLineName if c in validChars)
    filepath = os.path.join(folder, safeFilename + ".rst")
    file = open(filepath, "w")
    file.write(alg.name.upper())
    file.write("\n")
    file.write("=" * len(alg.name))
    file.write("\n\n")
    file.write("Description\n")
    file.write("-----------\n\n")
    file.write("Parameters\n")
    file.write("----------\n\n")
    for param in alg.parameters:
        file.write("- ``" + param.description + "[" + param.parameterName()[9:] + "]``:\n")
    file.write("\nOutputs\n")
    file.write("-------\n\n")
    for out in alg.outputs:
        file.write("- ``" + out.description + "[" + out.outputTypeName()[6:] + "]``:\n")
    file.write("\nSee also\n")
    file.write("---------\n\n")
    file.write("\nConsole usage\n")
    file.write("-------------\n\n")
    file.write("\n::\n\n")
    s = "\tsextante.runalg('" + alg.commandLineName() + "', "
    for param in alg.parameters:
        s+= str(param.name.lower().strip()) + ", "
    for out in alg.outputs:
        if not out.hidden:
            s+=str(out.name.lower().strip()) + ", "
    s = s[:-2] +")\n"
    file.write(s)

    s =""
    hasSelection = False
    for param in alg.parameters:
        if isinstance(param, ParameterSelection):
            hasSelection = True
            s+="\n\t" + param.name.lower() + "(" + param.description + ")\n"
            i=0
            for option in param.options:
                s+= "\t\t" + str(i) + " - " + str(option) + "\n"
                i+=1
    if hasSelection:
        file.write("\n\tAvailable options for selection parameters:\n")
        file.write(s)
    file.close()


def createBaseHelpFiles(folder):
    for provider in Sextante.providers:
        for alg in provider.algs:
            createBaseHelpFile(alg, folder)
