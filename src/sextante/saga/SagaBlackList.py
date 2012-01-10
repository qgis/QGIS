class SagaBlackList:

    @staticmethod
    def isBlackListed(name, group):

        if "Lectures" == group:
            return True

        if "pointcloud_viewer" == group:
            return True

        if group.startswith("docs"):
            return True
