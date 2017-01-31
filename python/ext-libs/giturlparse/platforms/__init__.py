# Imports
from .base import BasePlatform
from .github import GitHubPlatform
from .bitbucket import BitbucketPlatform
from .friendcode import FriendCodePlatform
from .assembla import AssemblaPlatform


# Supported platforms
PLATFORMS = (
    # name -> Platform object
    ('github', GitHubPlatform()),
    ('bitbucket', BitbucketPlatform()),
    ('friendcode', FriendCodePlatform()),
    ('assembla', AssemblaPlatform()),

    # Match url
    ('base', BasePlatform()),
)

PLATFORMS_MAP = dict(PLATFORMS)