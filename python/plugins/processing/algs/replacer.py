import glob

import re

#regex = re.compile(r"( *)def defineCharacteristics(self):\n(\s*)self.name, self.i18n_name = self.trAlgorithm\('(.*)'\)\n(\s*)self.group, self.i18n_group = self.trAlgorithm\('(.*)'\)", re.MULTILINE)

regex = re.compile(r"^(.*)(\s*def name)", re.M | re.DOTALL)
regex2 = re.compile(r"", re.M | re.DOTALL)
regex = re.compile(r"^(.*?)(\s*def name.*)def defineCharacteristics\(self\):\s*\n(.*?)(\s*def .*)$", re.M | re.DOTALL)
#regex = re.compile(r"(\s*)def defineCharacteristics(self):", re.MULTILINE)

for filename in glob.iglob('./**/*.py', recursive=True):
    out = None
    with open(filename, 'r') as myfile:
        data = myfile.read()

        r = regex.search(data)
        if r:

            out = r.groups()[0]
            out += '\n\n    def __init__(self):\n        super().__init__()\n'
            out += r.groups()[2]
            out += r.groups()[1]
            out += r.groups()[3]
            print(out)

        #r2=regex2.search(data)
        #if r2:
    #		print(r2.groups()[1])
    #		print('-0-----')
    #		print(r2.groups()[2])

        #d=regex.sub(r"\1def name(self):\n\2return '\3'\n\n\1def group(self):\n\2return '\5'\n\n\1def defineCharacteristics(self):",data)
        #d=regex.sub(r"\1def defineCharacteristics(self)",data)

    if out:
        with open(filename, 'w') as myfile:
            myfile.write(out)

            if False:
                d = r.group()  # print(r.groups())
                #print(d)
                d = regex.sub(r"\1def name(self):\n\2return '\3'\n\n\1def group(self):\n\2return '\5'\n", d)
                print(d)

#    def group(self):
#        return '[GDAL] Analysis'  d)
