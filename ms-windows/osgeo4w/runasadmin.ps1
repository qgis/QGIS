#-RunAsAdministrator
Write-Output ($args -join ' ')
$cmd, $args = $args
Start-Process $cmd -Wait -ArgumentList $args -NoNewWindow
