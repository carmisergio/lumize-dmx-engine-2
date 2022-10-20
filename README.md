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

#### Debian/Rasperry Pi OS

```bash
sudo apt-get update
sudo apt-get install build-essential libftdi1-dev
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
- `pushbutton_fade_reset_delay`: seconds to wait before resetting the direction after a pushbutton fade. Default: 10
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

## TCP protocol definition

The Lumize DMX Engine 2 is controlled via a custom TCP protocol. By default, it listens on port 8056. This protocol is also used by the Lumize DMX Engine 2 Home Assistant integration to control it.

### Message structure

Every message is composed of some number of comma separated sections in this fashion:

```
command,parameter1,parameter2...
```

Every message must start with the command.

The first parameter after commands that require specifying a channel is always the channel:

```
command,channel,parameter1,parameter2...
```

The order of parameters is uninportant, as parameters are identified with a letter.

```
command,a[parameter value],b[parameter_value]...
```

For example, the command fo turning on light channel 5 at brightness 150 with a transition of 1500ms would be

```
on,5,b150,t1000
```

### Available Commands

#### Light Turn ON Command

Turns on a light

```
on,[channel]
```

Parameters:

- `b`: brightness (0-255)
- `t`: transition (>=0)

Full example:

```
on,0,b100,t500
```

This command will turn on channel 0 at brightness 100, with a transition of 500 ms.

#### Light Turn OFF Command

Turns off a light

```
off,[channel]
```

Parameters:

- `t`: transition (>=0)

Full example:

```
off,0,t500
```

This command will turn off channel 0 with a transition of 500 ms.

#### Pushbutton Fade Start Command

Starts fading a channel up and down indefinitely to select new brightness.

```
pfstart,[channel]
```

Parameters:

- `d`: direction (0: down, 1: up)

Full example:

```
pfstart,0,d0
```

This command will start a pushbutton fade on channel 0 in the downwards direction.

#### Pushbutton Fade End Command

Stop fading and save current brightness.

```
pfend,[channel]
```

Full example:

```
pfend,0
```

This command will start a pushbutton fade on channel 0 in the downwards direction.

#### Light States Update Request

Request a ligh state update from the Engine.

```
sreq
```

Response:

```
sres,[state0]-[brightness0],[state1]-[brightness1],...
```

The state of the number of channels in configuration is represented sequentially.

Full response example:

```
sres,0-255,1-100,0-150
```

This is the status update response for 3 channels. The first channel is off and se to 255 brightness, the second is on and set to 100 and the third is off and set to 150.

echo "blacklist module_name" >> /etc/modprobe.d/local-dontload.conf
echo "install module_name /bin/false" >> /etc/modprobe.d/local-dontload.conf
