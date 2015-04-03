import os
import subprocess

def convertUiFile(f):

    command = ["pyuic4.bat", f, "-o", "ui_" + os.path.splitext(f)[0] + ".py"]

    proc = subprocess.Popen(
        command,
        shell=True,
        stdout=subprocess.PIPE,
        stdin=open(os.devnull),
        stderr=subprocess.STDOUT,
        universal_newlines=True,
    ).stdout
    for line in iter(proc.readline, ''):
        pass


if __name__ == '__main__':
    folder = "."
    for descriptionFile in os.listdir(folder):
        if descriptionFile.endswith('ui'):
            convertUiFile(descriptionFile)
