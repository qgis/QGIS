# The following has been generated automatically from src/3d/qgs3dmapscene.h
Qgs3DMapScene.Ready = Qgs3DMapScene.SceneState.Ready
Qgs3DMapScene.Updating = Qgs3DMapScene.SceneState.Updating
try:
    Qgs3DMapScene.__attribute_docs__ = {'terrainEntityChanged': 'Emitted when the current terrain entity is replaced by a new one\n', 'terrainPendingJobsCountChanged': "Emitted when the number of terrain's pending jobs changes\n", 'totalPendingJobsCountChanged': 'Emitted when the total number of pending jobs changes\n\n.. versionadded:: 3.12\n', 'sceneStateChanged': "Emitted when the scene's state has changed\n", 'fpsCountChanged': 'Emitted when the FPS count changes\n', 'fpsCounterEnabledChanged': 'Emitted when the FPS counter is activated or deactivated\n', 'viewed2DExtentFrom3DChanged': 'Emitted when the viewed 2D extent seen by the 3D camera has changed\n\n.. versionadded:: 3.26\n', 'gpuMemoryLimitReached': "Emitted when one of the entities reaches its GPU memory limit\nand it is not possible to lower the GPU memory use by unloading\ndata that's not currently needed.\n"}
    Qgs3DMapScene.openScenes = staticmethod(Qgs3DMapScene.openScenes)
    Qgs3DMapScene.__signal_arguments__ = {'fpsCountChanged': ['fpsCount: float'], 'fpsCounterEnabledChanged': ['fpsCounterEnabled: bool'], 'viewed2DExtentFrom3DChanged': ['extent: List[QgsPointXY]']}
except (NameError, AttributeError):
    pass
