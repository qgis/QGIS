import os
from sextante.gui.InteractiveHTMLViewerDialog import InteractiveHTMLViewerDialog
class ExternalAppsConfigurer():

    def configure(self):
        #TODO find a way of automating this, not having to add each provider here manually
        settings = {}
        folders = self.findFolder("c:\\","saga")
        settings["SAGA binaries folder"] = folders
        folders['SAGA'] = settings

        html = ""
        for key in settings.keys():
            html += self.createHTMLSection(settings[key], key)

        dialog = InteractiveHTMLViewerDialog(html, self)
        dialog.exec_()

    def findFolder(self, head, name):
        name = name.upper()
        found = []
        for root, dirs, files in os.walk(head):
            for d in dirs:
                if d.upper().endswith(name):
                    found.append(os.path.join(root, d))
        return found

    def createHTMLSection(self, settings, name):
        html = "<h2>" + name.upper() + "</h2>\n"
        html += "<ul>\n"
        for key, setting in settings.items():
            html += "<li>" + key + " : " + setting[0] + "</li>\n"
            if len(setting) > 1:
                html += "<ul>\n"
                for i in range(1, len(setting)):
                    html += "<li><a href=\"" + key + "|" + str(i) + ">" + setting[i] + "</a></li>\n"
                html += "<ul>\n"
        html += "<ul>\n"