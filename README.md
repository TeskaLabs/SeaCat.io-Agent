# SeaCat.io Agent

The agent application for https://SeaCat.io/ which is IoT device management.

```
   #####                 #####
  #     # ######   ##   #     #   ##   #####      #  ####
  #       #       #  #  #        #  #    #        # #    #
   #####  #####  #    # #       #    #   #        # #    #
        # #      ###### #       ######   #        # #    #
  #     # #      #    # #     # #    #   #   ##   # #    #
   #####  ###### #    #  #####  #    #   #   ##   #  ####
```


TeskaLabs Ltd


## SSH client configuration for a user access

In order to access a shell of the device remotely via SeaCat.io, you need to configure your local SSH client to use a seacat.io access gateway.

For OpenSSH client, append a following snipplet into your ~/.ssh/config, create that file if it doesn't exists.

	Host *.seacat.io
		ProxyCommand socat stdio SOCKS4A:link.seacat.io:%h:%p,socksport=12367

Then use a following line to access your device:

	$ ssh user@d434df9b763ec138fe1a524504e994287e50caaed0a9f8b13f4c0797314bafa74ad939.seacat.io

_Note: the address is copied from an admin.seacat.io_
