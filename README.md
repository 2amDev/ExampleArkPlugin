Example plugin for Ark: Survival Evolved Steam Servers
====================
This plugin isn't very special and simply shows as a demo of how you can hook ShooterGameServer.exe functions
to implement a basic dependency-free server plugin for Steam & Epic games. The advantage of creating a plugin by
hooking game functions is immense, as it allows you not only far more power and customizability, it doesn't
require players or admins to download any mods. 

This plugin simply tracks and stores the IPs of all connecting and disconnecting players, and would serve to
either implement an additional IP banning system or prevent players from using two alt accounts
at the same time if that's something you want...

Usage
---------
If this is something you want to implement, you'd need to do a couple of things
* Ensure your server is running on a Windows machine
* Compile and somehow call LoadLibrary on this dll, or implement an additional binary that remotely creates a thread and calls LL in ShooterGameServer.exe
* Change the current implementation of simply storing connected player's IPs internally, to using a shared database of your choice
* Add your function hooks on more interesting functions, and AShooterPlayerController::ServerKickPlayer
* ...etc
