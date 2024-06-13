set NAME=washing-machine

adb push \workspace\F35\tina\out\r11-R11_pref1\compile_dir\target\washing-machine\src\washing-machine /tmp/%NAME%

::adb push \R11\tina\package\minigui\washing-machine\src\res\menu\video.png /tmp/
::adb push \R11\tina\package\minigui\washing-machine\src\res\menu\sensor.png /tmp/

adb shell chmod 777 /tmp/%NAME%
::adb shell chmod 777 /tmp/video.png
::adb shell chmod 777 /tmp/sensor.png

::adb shell %NAME%

pause
