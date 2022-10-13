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
Run installation script
```bash
sudo make install
```

## Uninstallation
To uninstall, just run
```bash
sudo make uninstall
```



## Configuration
The Lumize DMX Engine 2 stores its configuration in `/etc/lumizedmxengine2.conf`

Every line of the configuration file starting with `#` is a comment and will be ignored.

### Config options
- `port`: TCP port on which to listen for commands. Default: 8056
- `channels`: amount of channels to output via DMX (1-512). Default: 25
- `fps`: how many frames of DMX data to ouptut per second. Default: 50 _ATTENTION: as the DMX protocol has a limit on bytes sent per second, the fps value is directly correlated with the amount of channels outputed. For example, while outputing 512 channels the max fps value is about 40. If you are encountering issues with light output, I suggest lowering your fps to 10 and then increasing it slowly._
- `default_transition`: default transition value in milliseconds. 0 for no transition (>=0). Default: 1000
- `pushbutton_fade_delta`: amount the engine should increment or decrement a channel value in a second during a pusbutton fade. Default: 25.
- `pushbutton_fade_pause`: milliseconds of pause at full brightness during a pusbhutton fade. 0 for no pause. Default: 500
- `ushbutton_fade_reset_delay`: seconds to wait before resetting the direction after a pushbutton fade. Default: 10
- `enable_persistency`: enable persistency of light states after power failure. Default: false
- `persistency_file_path`: path of the file where to save the light states data. Default: /var/lib/lumizedmxengine2/persistency
- `persistency_write_interval`: delay between periodic persistency writes in seconds. Default: 600
- `log_debug`: enable debug logging. Default: false. _ATTENTION: enabling this option will make the engine log every single command from every client and will generate pretty lenghty logs_


### Config file example
Example of a `lumizedmxengine2.conf` with some options changed:

```conf
##########################################
# Lumize DMX Engine 2 Configuration file #
##########################################

### TCP server port
# port = 8056

### Channels rendered
channels = 50 

### DMX frames outputed per second
fps = 100

### Default transition (milliseconds)
default_transition = 500

### Pushbutton fade increment amount
pushbutton_fade_delta = 50

### Pushbutton fade full brightness pause (milliseconds)
# pushbutton_fade_pause = 500

### Pushbutton fade reset timeout (seconds)
# pushbutton_fade_reset_delay = 10 

### Enable light states persistency
enable_persistency = true

### Persistency file path
# persistency_file_path = /var/lib/lumizedmxengine2/persistency

### Persistency periodic write interval (seconds)
# persistency_write_interval = 600 

### Enable debug logging
# log_debug = false
```






