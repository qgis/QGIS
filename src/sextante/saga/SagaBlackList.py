class SagaBlackList:

    @staticmethod
    def isBlackListed(name, group):

        group = group.lower()

        if "lectures" == group:
            return True

        if "io_" == group:
            return True

        if "pointcloud" in group:
            return True

        if group.startswith("docs"):
            return True
