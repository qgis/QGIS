class SextanteResults():

    results = []

    @staticmethod
    def addResult(name, result):
        SextanteResults.results.append(Result(name, result))

    @staticmethod
    def getResults():
        return SextanteResults.results


class Result():

    def __init__(self, name, filename):
        self.name = name
        self.filename = filename





