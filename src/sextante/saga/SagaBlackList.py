class SagaBlackList:

    @staticmethod
    def isBlackListed(name, group):

        group = group.lower()

        if "tin_tools" == group:
            return True

        if "lectures" in group:
            return True

        if "io_" in group:
            return True

        if "pointcloud" in group:
            return True

        if group.startswith("docs"):
            return True
