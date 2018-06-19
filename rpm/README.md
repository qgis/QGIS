## QGIS RPM generator

### Build RPM using mock

```bash
$ ./buildrpms.sh
```

### Generate a valid spec file from template

```bash
$ ./buildrpms.sh -c
```

### Build SRPM using mock

```bash
$ ./buildrpms.sh -s
```

### Rebuild last generated RPM using mock

```bash
$ ./buildrpms.sh -b
```

### Build an unstable release

```bash
$ ./buildrpms.sh [flags] -u
```

This generates an RPM with release `.git<short commit ID>`


### MOCK old chroot

It is possible to execute `mock` using the 'old chroot' behavior (which does not uses `systemd-nspawn`) setting
the environment variable `_MOCK_OLD_CHROOT` before running `buildrpms.sh`:

```bash
export _MOCK_OLD_CHROOT=1
```

This may be useful when running `mock` inside environments that do not play well with `systemd-nspawn` (LXC/LXD, Docker...).
