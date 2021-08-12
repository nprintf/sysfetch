# sysfetch

sysfetch is a tool to display your hardware specifications, software and OS logo or any other logo of your choice. And a clone of \*fetch programs (neofetch, screenfetch) in C.

Unlike other \*fetch programs, this program does not require much resources and very speedy.

**Please note that this program isn't fully stable and you'll likely get weird quirks outputted on different machines.**

## Comparasion

This table compares other \*fetch programs to this program by speed on one run. This test was carried out on a Intel i7-4790K system with Artix Linux 64-bit install using `time` utility and comparasion based on `real` value.

sysfetch 3.0 | neofetch 7.1.0 | screenfetch 3.9.1 | pfetch 0.6.0 |
------------ | -------------- | ----------------- | ------------ |
0m0.016s     | 0m0.174s       | 0m0.683s          | 0m0.023s     |

You'll notice a big difference if you run this on older systems :)

## Install

```
$ git clone https://github.com/ecksemzee/sysfetch.git
$ cd sysfetch
$ make
$ <doas/sudo> make install
```

Program can be removed by `<doas/sudo> make uninstall`

## Screenshots

![no scrcap yet](https://raw.githubusercontent.com/ecksemzee/sysfetch/master/screenshots/artix-realmachine.png)

## More documentation

More documentation is in man page.
