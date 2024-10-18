# Project Pumpkin Spice

## Getting Started
Welcome to Project Pumpkin Spice!

To begin, right click on PumpkinSpice.uproject and select 'Generate Visual Studio Project Files'

This should generate the files you need to open the project through Visual Studio or another IDE

## Running In Steam

To run Project PumpkinSpice as a steam client, you need to do a couple of things. 

1. ensure that steam is open on your machine and you are logged into your steam account
2. load up the editor
3. in the play settings, left click on the three dots next to the Play button and click
4. select 'Standalone Game' 

The editor will then launch a client into the 3rd person level. If you open Steam, you'll notice that the active game you're playing is
'SpaceWar'. This means that PumpkinSpice is connected to Steam

## Testing Multiplayer

You can test multiplayer in a couple different ways

1. Via editor, you will not be using steam sessions but you can boot up multiple clients through the play window to test replication code
2. Go into the editor and "Package" your build. You should see a `PumpkinSpice.exe` that you run, and if you have steam open you will sign into your steam account. Please note that you cannot test multiple steam clients at once on the same PC. 

### Using Windows Sandbox for multiple Steam clients

If you want to test multiplayer using Steam, there's a way around this with Windows Sandbox.

Under `Sandbox` youll find a file you can use to launch a windows sandbox instance. From there you can copy the build from the shared directory and launch multiple steam clients