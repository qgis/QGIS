# The following has been generated automatically from src/3d/qgscameracontroller.h
try:
    QgsCameraController.__attribute_docs__ = {'cameraChanged': 'Emitted when camera has been updated\n', 'navigationModeChanged': 'Emitted when the navigation mode is changed using the hotkey ctrl + ~\n', 'cameraMovementSpeedChanged': 'Emitted whenever the camera movement speed is changed by the controller.\n', 'setCursorPosition': 'Emitted when the mouse cursor position should be moved to the specified ``point``\non the map viewport.\n', 'requestDepthBufferCapture': 'Emitted to ask for the depth buffer image\n\n.. versionadded:: 3.24\n', 'cameraRotationCenterChanged': 'Emitted when the camera rotation center changes\n\n.. versionadded:: 3.24\n'}
    QgsCameraController.__signal_arguments__ = {'navigationModeChanged': ['mode: Qgis.NavigationMode'], 'cameraMovementSpeedChanged': ['speed: float'], 'setCursorPosition': ['point: QPoint'], 'cameraRotationCenterChanged': ['position: QVector3D']}
except (NameError, AttributeError):
    pass
