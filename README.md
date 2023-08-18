# ![ikureader icon](https://github.com/awkitsune/IkuReader/blob/main/data/icon.bmp) IkuReader 
an e-book reader for Nintendo DS

# About
I'm not an author of this cource code! I just decided to update it to be able to use it on my Nintendo DSi without any quirks. All original info is in [info.txt](https://github.com/awkitsune/IkuReader/blob/main/info.txt) file.
License is in [License.txt](https://github.com/awkitsune/IkuReader/blob/main/Licence.txt) file 

# Compilation
Should be working with latest devkitPro and libnds.

To build it do following:
- Open MSYS2 in project folder
- Print `make` in console
  
  If you have following error in `arm7` make, just relaunch `make` without cleaning. I will fix it when I will find solution
  ```make
  arm-none-eabi-gcc.exe: warning: g: linker input file unused because linking not done
  arm-none-eabi-gcc.exe: error: g: linker input file not found: No such file or directory
  ```
- Wait until software will be compiled, if some other errors will appear, try use `make clean`

# Contact
Email - [vladimir.kosickij@gmail.com](mailto:vladimir.kosickij@gmail.com)

Telegram - [t.me/awkitsune](https://t.me/awkitsune)

Discord - @ikainky
# Thanks to
chintoi - original author of this software
devkitPro
