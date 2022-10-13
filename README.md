# Lumize Dmx Engine 2

The Lumize Dmx Engine is a lighting controller software that enables network control of DMX dimmers. 

## Installation

### Dependencies
- build-essentials
- [LibFTDI](https://github.com/lipro/libftdi)

Before building, make sure you have all the necessary dependencies installed.

#### Arch Linux
```bash
sudo pacman -Sy base-devel libftdi
```

### Building
Clone this repository to your local file system and enter its directory

```bash
git clone https://github.com/carmisergio/lumize-dmx-engine-2.git
cd lumize-dmx-engine-2
```

Build the main binary
```bash
make 
```

### Installing
Copy binary and all required files into place, register service with systemd
```bash
sudo make install
```
