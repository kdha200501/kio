# How to

##### Install dependencies

```shell
$ sudo dnf install libxslt-devel kf6-karchive-devel.x86_64
```



##### Compile and install `kio`

```shell
$ rpm -qa | grep kf6-kio-6
$ git checkout customize/v<x.y.z>
$ mkdir build
$ cd build
$ cmake ..
$ make -j$(nproc)
$ sudo make install
```





# Customization

##### One focus at a time



##### Disable mouse over effect



##### Use list view in file dialog



##### Customize shortcuts

This is part of an effort to bring back the Mac OS 9 desktop environment

