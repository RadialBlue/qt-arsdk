# qt-arsdk
Qt QML ARSDK Implementation for controlling Parrot drones.

## Building

```
$ git clone https://github.com/RadialBlue/qt-arsdk.git
$ cd qt-arsdk
$ mkdir ARSDK
$ cd ARSDK
$ repo init -u https://github.com/Parrot-Developers/arsdk_manifests.git
$ repo sync
$ cd ..
$ qmake && make && make install
```

## Examples

Example usage can be found in the https://github.com/RadialBlue/qt-arsdk-examples.git repository.

