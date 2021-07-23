# sysfetch

sysfetch is a tool to display your hardware specifications, software and OS logo or any other logo of your choice. And a clone of \*fetch programs (neofetch, screenfetch) in C.

Unlike other \*fetch programs, this program does not require much resources and very speedy.

Currently this program only works on most Linux systems and more portability will be implemented such as particular OS support.

## Comparasion

This table compares other \*fetch programs to this program by speed on one run. This test was carried out on a Intel i7-4790K system with Artix Linux 64-bit install using `time` utility on instance.

Using `real` time value

sysfetch 1.0 | neofetch 7.1.0 | screenfetch 3.9.1 | pfetch 0.6.0 |
------------ | -------------- | ----------------- | ------------ |
0.015s       | 0.168s         | 0.474s            | 0.016s       |

## Install

```
$ git clone https://github.com/ecksemzee/sysfetch.git
$ cd sysfetch
$ make
$ <doas/sudo> make install
```

Program can be removed by `<doas/sudo> make uninstall`

## Screenshots

![no scrcap yet](aa)
