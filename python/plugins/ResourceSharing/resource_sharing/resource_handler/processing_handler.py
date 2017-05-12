# coding=utf-8
import os
import fnmatch

from processing.script.ScriptAlgorithm import ScriptAlgorithm
from processing.script.WrongScriptException import WrongScriptException
from processing.script.ScriptUtils import ScriptUtils

from resource_sharing.resource_handler.base import BaseResourceHandler
from resource_sharing.utilities import qgis_version


class ProcessingScriptHandler(BaseResourceHandler):
    """Concrete class handler for processing script resource."""
    IS_DISABLED = False

    def __init__(self, collection_id):
        """Constructor of the base class."""
        BaseResourceHandler.__init__(self, collection_id)

    @classmethod
    def dir_name(cls):
        return 'processing'

    def install(self):
        """Install the processing scripts in the collection.

        We copy the processing scripts exist in the processing dir to the
        user's processing scripts directory (~/.qgis2/processing/scripts) and
        refresh the provider.
        """
        # Check if the dir exists, pass installing silently if it doesn't exist
        if not os.path.exists(self.resource_dir):
            return

        # Get all the script files under self.resource_dir
        processing_files = []
        for item in os.listdir(self.resource_dir):
            file_path = os.path.join(self.resource_dir, item)
            if fnmatch.fnmatch(file_path, '*.py'):
                processing_files.append(file_path)

        for processing_file in processing_files:
            # Install silently the processing file
            try:
                script = ScriptAlgorithm(processing_file)
            except WrongScriptException:
                continue

            script_file_name = os.path.basename(processing_file)
            script_name = '%s (%s).%s' % (
                os.path.splitext(script_file_name)[0],
                self.collection_id,
                os.path.splitext(script_file_name)[1],)
            dest_path = os.path.join(self.scripts_folder(), script_name)
            with open(dest_path, 'w') as f:
                f.write(script.script)

        self.refresh_script_provider()

    def uninstall(self):
        """Uninstall the processing scripts from processing toolbox."""
        # Remove the script files containing substring collection_id
        for item in os.listdir(self.scripts_folder()):
            if fnmatch.fnmatch(item, '*%s*' % self.collection_id):
                script_path = os.path.join(self.scripts_folder(), item)
                if os.path.exists(script_path):
                    os.remove(script_path)

        self.refresh_script_provider()

    def refresh_script_provider(self):
        """Refresh the processing script provider."""
        if qgis_version() < 21600:
            from processing.core.Processing import Processing
            Processing.updateAlgsList()
        else:
            from processing.core.alglist import algList
            algList.reloadProvider('script')

    def scripts_folder(self):
        """Return the default processing scripts folder."""
        # Copy the script
        if qgis_version() < 21600:
            return ScriptUtils.scriptsFolder()
        else:
            return ScriptUtils.defaultScriptsFolder()
