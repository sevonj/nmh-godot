# NMH Viewer

Demo that loads NMH assets into Godot game engine.

(Partially) Supported formats:
- FLCG collision models (.gcl)
- GMF2 models (.gm2)
- RMHG archives (.rsl, .pak)

![image](https://github.com/user-attachments/assets/b5d98aa8-c142-423c-b345-126605d656c6)
![Screenshot from 2024-09-14 16-01-11](https://github.com/user-attachments/assets/8824ca56-60ce-43ae-8c2a-37e3c8b6d852)

Go to [releases](https://github.com/sevonj/nmh-godot/releases) to download

# Building
Godot version: 4.3  
Build the C++ source using scons.
1. run `scons` at repository root. The binary should appear at `project/bin`.
2. Export godot project like normal.

Read more about C++ extensions at [Godot Docs](https://docs.godotengine.org/en/stable/tutorials/scripting/gdextension/what_is_gdextension.html)
