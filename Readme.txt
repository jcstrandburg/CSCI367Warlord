Client:
There is a gui implemented. By default if the client is in auto mode the interface is turned off and the client runs in silent text mode.
In manual mode the client runs in verbose mode with an interface. There is also a logging mode which can be enabled as described below.

By default the client will play automatically. The client can be set to manual mode with the -m flag

The interface can be turned off or on with the following flag: -i x
Where x is one of the following:
0: text interface
1: graphical interface

The output of the client can be tuned with the following flag: -v x
Where x is one of the following:
0: Silent mode
1: Quiet mode
2: Verbose mode

The flag --log will set the client to logging mode. This will cause the client to create a log file where it will print all incoming and outgoing
traffic on its socket. This log file will be placed in the director "./clogs/" This directory must
exist or no logs will be created.

By default the client will will attempt to connect via port 36738. This can be changed with the flag -p x, where x is the port to connect to

By default the the client will attempt to connect to localhost. This can be changed witht eh flag -p x, where x is the address of the server

If the client is running in manual mode the player will be prompted for a name before connecting, in auto mode a player name can be specified with the flag -u x, where x is the desired name




Server:
There is a gui implemented. By default the server will run in silent text mode. This can be modified with the following flags. 
There is also a logging mode which can be enabled as described below.

The interface can be turned off or on with the following flag: -i x
Where x is one of the following:
0: text interface
1: graphical interface

The output of the server can be tuned with the following flag: -v x
Where x is one of the following:
0: Silent mode
1: Quiet mode
2: Verbose mode

The flag --log will set the server to logging mode. This will cause the server to create a log file for each and every socket opened on the server,
 where it will print all incoming and outgoing traffic on that socket. This log file will be placed in the director "./slogs/" This directory must
exist or no logs will be created.

By default the server will listen for connections on port 36738. This can be changed with the flag -p x, where x is the port to listen on

The lobby timeout can be set with the -l x, where x is the timeout in seconds.
The play timeout can be set with -t x, where x is the timeout in seconds.
The minimum player count can be set with -m x, where x is the minimum number of players to start a game.

