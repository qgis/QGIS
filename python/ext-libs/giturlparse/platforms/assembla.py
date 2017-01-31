# Imports
from .base import BasePlatform


class AssemblaPlatform(BasePlatform):
    DOMAINS = ('git.assembla.com',)
    PATTERNS = {
        'ssh': r'git@(?P<domain>.+):(?P<repo>.+).git',
        'git': r'git://(?P<domain>.+)/(?P<repo>.+).git',
    }
    FORMATS = {
        'ssh': r'git@%(domain)s:%(repo)s.git',
        'git': r'git://%(domain)s/%(repo)s.git',
    }
    DEFAULTS = {
        '_user': 'git'
    }
