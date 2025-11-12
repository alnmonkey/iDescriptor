<p align="center" style="text-align: center">
  <img src="./resources/icons/app-icon/icon.png" width="25%"><br/>
</p>

<p align="center">
    Cross platform, open-source and free idevice management tool written in C++
  <br/>
  <br/>
  <a href="https://github.com/wailsapp/wails/blob/master/LICENSE">
    <img alt="GitHub" src="https://img.shields.io/github/license/wailsapp/wails"/>
  </a>
  <a href="https://github.com/wailsapp/wails/issues">
    <img src="https://img.shields.io/badge/contributions-welcome-brightgreen.svg?style=flat" alt="CodeFactor" />
  </a>
  <a href="https://app.fossa.com/projects/git%2Bgithub.com%2Fwailsapp%2Fwails?ref=badge_shield" alt="FOSSA Status">
    <img src="https://app.fossa.com/api/projects/git%2Bgithub.com%2Fwailsapp%2Fwails.svg?type=shield" />
  </a>
  <a href="https://github.com/avelino/awesome-go" rel="nofollow">
    <img src="https://cdn.rawgit.com/sindresorhus/awesome/d7305f38d29fed78fa85652e3a63e154dd8e8829/media/badge.svg" alt="Awesome" />
  </a>
  <a href="https://discord.gg/BrRSWTaxVK">
    <img alt="Discord" src="https://img.shields.io/discord/1042734330029547630?logo=discord"/>
  </a>
  <br/>
  <a href="https://github.com/wailsapp/wails/actions/workflows/build-and-test.yml" rel="nofollow">
    <img src="https://img.shields.io/github/actions/workflow/status/wailsapp/wails/build-and-test.yml?branch=master&logo=Github" alt="Build" />
  </a>
  <a href="https://github.com/wailsapp/wails/tags" rel="nofollow">
    <img alt="GitHub tag (latest SemVer pre-release)" src="https://img.shields.io/github/v/tag/wailsapp/wails?include_prereleases&label=version"/>
  </a>
</p>

<p align="center">
    <img src="./resources/repo/crossplatform.png"><br/>
</p>

## Features

### Connection
| Feature                     | Status               | Notes                                                                                       |
| --------------------------- | -------------------- | ------------------------------------------------------------------------------------------- |
| USB Connection              | ✅ Implemented       | Fully supported on Windows, macOS, and Linux.                                               |
| Wireless Connection (Wi‑Fi) | ⚠️ To be implemented | - |

### Tools
| Feature                                                          | Status               | Notes                                                                                       |
| ---------------------------------------------------------------- | -------------------- | ------------------------------------------------------------------------------------------- |
| AirPlay Mirroring                                                 | ✅ Implemented       | Cast your device screen to your computer.                                                   |
| [Download & Install Apps From Apple Store](#app-store)            | ✅ Implemented       | Download and install apps directly from the Apple Store.                                    |
| [Virtual Location](#virtual-location)                             | ✅ Implemented       | Simulate GPS location. Requires a mounted Developer Disk Image. **( iOS 6 - iOS 16)**       |
| [iFuse Filesystem Mount](#ifuse-filesystem-mount)                 | ✅ Implemented       | Mount the device's filesystem. (Windows & Linux only)                                       |
| Gallery                                            | ✅ Implemented       | -                       |
| File Explorer                                            | ✅ Implemented       | Explore the device's filesystem.                       |
| Wireless Gallery Import                                            | ✅ Implemented       | Import photos wirelessly (requires the Shortcuts app on the iDevice).                       |
| [Cable Info](#cable-info)                                         | ✅ Implemented       | Check authenticity of connected USB cables and more.                                        |
| [Network Device Discovery](#network-device-discovery)             | ✅ Implemented       | Discover and monitor devices on your local network.                                         |
| [SSH Terminal](#ssh-terminal)  **(Jailbroken)**                   | ✅ Implemented       | Open up a terminal on your iDevice.                                                          |
| Query MobileGestalt                                               | ✅ Implemented       | Read detailed hardware and software information from the device.                            |
| Live Screen                                                       | ✅ Implemented       | View your device's screen in real-time **(wired)**.                                         |
| Developer Disk Images                                             | ✅ Implemented       | Manage and mount developer disk images. **( iOS 6 - iOS 16)**                               |

### Device Actions
| Feature                     | Status               | Notes                                                                                       |
| --------------------------- | -------------------- | ------------------------------------------------------------------------------------------- |
| Restart Device              | ✅ Implemented       |                                                                                             |
| Shutdown Device             | ✅ Implemented       |                                                                                             |
| Enter Recovery Mode         | ✅ Implemented       |                                                                                             |

## Fully Theme Aware
<p align="center">
    <img src="./resources/repo/macos-theme.gif"><br/>
</p>
<p align="center">
    <img src="./resources/repo/ubuntu-theme.gif"><br/>
</p>

## App Store
### Download and Install Apps directly from the Apple Store!

You need to sign in with your Apple ID to use this feature.

<p align="center">
    <img src="./resources/repo/ipatool.png"><br/>
</p>

## Virtual Location
### Simulate GPS location on your iDevice! (iOS 6 - iOS 16)
<p align="center">
    <img src="./resources/repo/virtual-location.png"><br/>
</p>

## iFuse Filesystem Mount

### Use your iDevice as a regular DRIVE!

Literally mount your iDevice filesystem and use it as a regular drive , read and write are both allowed. Don't try to import photos or videos because it won't work that way, use the Gallery Import feature for that.

#### Windows

<p align="center">
    <img src="./resources/repo/win-ifuse.gif"><br/>
</p>


#### Ubuntu / Linux
<p align="center">
    <img src="./resources/repo/ifuse.gif"><br/>
</p>

### Gallery
<p align="center">
    <img src="./resources/repo/gallery.png"><br/>
</p>


### File Explorer
<p align="center">
    <img src="./resources/repo/file-explorer.png"><br/>
</p>

## Cable Info

<p align="center">
    <img src="./resources/repo/cable-info-genuine.png"><br/>
</p>

## Network Device Discovery
<p align="center">
    <img src="./resources/repo/network-devices.png"><br/>
</p>

## SSH Terminal

### Open up a terminal on your Jailbroken iDevice!

<p align="center">
    <img src="./resources/repo/ssh-terminal.gif"><br/>
</p>



## **Authentication Required** ?

 You might get this pop-up on any platform this is because this app uses secure backends to retrieve and store your Apple credentials. You can disabled this in settings but it is not recommended and not safe for your Apple account. Also if you leave this enabled and sign in you can use the same credentials in ipatool without signing in again.
<p align="center">
    <img src="./resources/repo/authentication-required.png"><br/>
</p>

## Become a Sponsor

If you like this project and want to support its development, consider becoming a sponsor on [GitHub Sponsors](https://github.com/sponsors/yourusername).

## Thanks

- [libimobiledevice](https://libimobiledevice.org/) - Cross-platform software library that talks the protocols to support iPhone, iPad, iPod Touch devices.
- [ipatool](https://github.com/majd/ipatool) - A command-line tool for managing iOS apps and devices.


@uncore  sudo cat /etc/udev/rules.d/99-idevice.rules
SUBSYSTEM=="usb", ATTR{idVendor}=="05ac", MODE="0666"

✘  Sun 6 Jul - 14:29  ~ 
@uncore  sudo groupadd idevice

Sun 6 Jul - 14:30  ~ 
@uncore  sudo usermod -aG idevice $USER

Sun 6 Jul - 14:30  ~ 
@uncore  sudo udevadm control --reload-rules
sudo udevadm trigger
