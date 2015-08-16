# D2-Maphack-for-Linux
These are a collection of programs that can be used to reveal the maps in Diablo 2 LOD
version 1.13d running under Wine. The maphack included does a number of other things
like displaying the item level of dropped items as well as displaying monsters and attacks
in the map.

# Install
To build, run:

`make`

This will build snoogans maphack as well as the module injector, surgeon.

To install the program and scripts, run:

`sudo make install`

You will need to setup GCC for multilib to compile 32bit. In Arch, this is accomplished
by installing gcc-multilib.

# Revealing Maps
Start Diablo using Wine and create a new game. Inject the maphack using the following:

`d2-inject`

This will load the maphack module into the currently running game. Any new game created
or joined will have the maphack already loaded so you only need to do this once.

# Unloading Module
Unload the maphack module at anytime using the following:

`d2-unload`

This is only necessary if you would like to go back to the stock game.

# FAQ
**Question**: How do I look at another players gear?
**Answer**: Hover over the player and press the inventory key.

**Question**: How does the maphack work?
**Answer**: The maphack is compiled into a shared library which is injected into the
running process. The injection script is used to get the process id of the game
then uses the injection program to load the static library into the running
game.

**Question**: Did you write this maphack?
**Answer**: I did not write this maphack. I am not taking credit for its creation. I am
only providing a convenient means of installing and running it.
