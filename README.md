# Building
 Just download the repository and make sure to run on **Relsease** mode and **x64** configuration. 
## Custom build tool
 If you are not going to compile shaders or haven't downloaded the [Vulkan SDK](https://www.lunarg.com/vulkan-sdk/) just remove the command line arguments in ``Project Settings/Custom Build Tool``.

# Additional links

* [Trello board](https://trello.com/b/PIi3gXiu/elementalworld)
* [Invite to the Discord server](https://discord.gg/edRSfvSw)

# Commit policy
 Every commit to this repository should have a **UPDATE.md** file to it with proper formatting divided in a few sections in the following order:

* Changes
* Bugfixes
* New Features/Additions

 Every one of these sections should have one of the following sub-sections:

 * Rendering/Graphics
 * Sound/Audio
 * Physics
 * Gameplay 
 * Optimization
 * Other

 ## Engine commitment
 For new features for the engine you should write a little ``README.md`` file explaining how does the new system work and how to use it properly.

# Navigation
 ``\bin`` - This is the location of the output executable file.

 ``\scripts`` - Gameplay related scripts. Should be removed in the future when **Modmaker** is introduced.

 ``\src`` - Source code. It mainly consists of
 * ``\world`` - Main gameplay stuff is here.
 * ``\shaders`` - Shader source code.
 * ``\Rendering`` - Rendering code.
 * ``Application.h`` - Application entry point.
 * ``main.cpp`` - C++ entry point.

 ``\vendor`` - Dependencies
 * ``\Include`` - All library include files.
 * ``\Libraries`` - All the **.lib** files that are required for compiling the project.

# Releases
 Download releases from [here](https://github.com/myri4/Elementalworld/releases).

# TODO
 * Other projects (Mod maker, Networking Server Project, etc.)
 * ``premake`` project build scripts.