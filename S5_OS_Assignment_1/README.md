# S5_OS_Assignment_1 
This Assignment is regarding a latest version of linux kernel compilation and dual booting it with our current version of linux.

## Dual booting Linux
All steps used during the process are listed below

1. Get the latest Linux kernel source code

2. Download the zip file from kernel.org

3. Extract tar.xz file
```bash
   $ tar –xvf linux-6.5.1.tar.xz
```

4. Configure the Linux kernel features and modules
```bash
   $ cd linux-6.5.1
   $ cp -v /boot/config-$(uname -r) .config
```

5. Install the required compilers and other tools
```bash
    $ sudo dnf install git gcc fakeroot make ncurses-devel bison flex openssl-devel dwarves elfutils-libelf-devel
```

6. How to compile a Linux Kernel
```bash
    $ yes “” | make -j $(nproc)
```

7. Install the Linux kernel modules
```bash
    $ sudo make modules_install
```

8. Install the Linux kernel
```bash
   $ sudo make install
```