# TalkState
Sends active talking state to Memory. Supposed to be used in junction with the [Positional Audio Mod for GTFO](https://github.com/WWYDF/OpenPA/).

⚠️ Linux and MacOS support is unknown! ⚠️

## Installation Guide
### Configure OpenPA
1. Make sure you have my Positional Audio Mod for GTFO installed in r2modman.
2. Run the game modded once, then close it.
3. Open r2modman, and go to the **Config Editor**.
4. Click on `BepInEx\config\net.devante.gtfo.positionalaudio.cfg`, then **Edit Config.**
5. Change `Enabled` to `true`.

### Configure Mumble
1. Download the latest `.dll` on the [Releases page](https://github.com/WWYDF/TalkState/releases). 
2. Open Mumble and open Settings.
3. Click on the Plugins tab.
4. Click `Install plugin...`
5. Select the downloaded `.dll`.
6. Click `Yes`.
7. Scroll down, and make sure **TalkState** is enabled.
8. Click `Apply`, then `OK`.
9. Connect to a Server, and launch your game.


> As long as the plugin is loaded, it will be sending your voice state to memory. Twice each time you speak; once for `talking`, once for `not talking`.

---

## Integrating with a different mod
If you would like to use this to send talking data to your own mod, you certainly can.

It uses a Memory Linked File @ `%temp%\gtfo_posaudio_mumlink` to store a string of the current state. My mod uses the following code to pull the data from the MLF to the BepInEx Plugin in C#.
```cs
static void ReadMemoryMappedFile()
{
    var character = Player.PlayerManager.GetLocalPlayerAgent();
    const string memoryMappedFileName = "posaudio_mumlink";
    const int dataSize = 1024;
    Console.WriteLine($"Initiated RMMF!");

    while (true)
    {
        using (var mmf = MemoryMappedFile.OpenExisting(memoryMappedFileName))
        {
            using (var accessor = mmf.CreateViewAccessor(0, dataSize))
            {
                byte[] dataBytes = new byte[dataSize];
                accessor.ReadArray(0, dataBytes, 0, dataSize);

                string receivedData = Encoding.UTF8.GetString(dataBytes).TrimEnd('\0');
                // Console.WriteLine($"Received Data: {receivedData}");

                if (receivedData == "Talking")
                {
					character.ForcePlayerNoiseChange(Agents.Agent.NoiseType.Walk);
                }
            }
        }
		// Console.WriteLine($"Sleeping...");
        Thread.Sleep(6); // Sleep for 6 milliseconds.
		// Adjust update frequency if CPU performance is bad.
    }
}
```
**I would 100% change the way this updates.** GTFO is stinky and requires you to update the values faster than the game in order for it to work, so you shouldn't have to take such an aggressive approach like I did.


---
# Known Issues

- None. Please report any on the [Issue Tracker](https://github.com/WWYDF/TalkState/issues).
---
