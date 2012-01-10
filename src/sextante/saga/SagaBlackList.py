class SagaBlackList:

    @staticmethod
    def isBlackListed(name, group):

        if "lectures_introduction" == group:
            return True

        if "pointcloud_viewer" == group:
            return True

        if group.startswith("docs"):
            return True
