 @uncore  sudo cat /etc/udev/rules.d/99-idevice.rules
SUBSYSTEM=="usb", ATTR{idVendor}=="05ac", MODE="0666"

 ✘  Sun  6 Jul - 14:29  ~ 
 @uncore  sudo groupadd idevice


 Sun  6 Jul - 14:30  ~ 
 @uncore  sudo usermod -aG idevice $USER


 Sun  6 Jul - 14:30  ~ 
 @uncore  sudo udevadm control --reload-rules
sudo udevadm trigger