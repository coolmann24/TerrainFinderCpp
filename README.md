This is a C++ library that mimics the world generation of Minecraft Java Edition across a host of versions. It was originally meant to be used as an optimized replacement for my Terrain Formation Finder project. I haven't done a user-friendly GUI app for this yet (still plan to eventually), but the library is basically fully functional and tested (minus maybe a few small bugs). You should be able link against this and use the interface in Search.h to do searches similar to Terrain Formation Finder. There is a few search options to choose, as well as the new "heightmap search", which only analyzes block chunk height and ignores block type. 

This currently links against a modded version of Cubitect's 'cubiomes' library: github.com/IceTank/cubiomes

### How to get this thing working in visual studio

1. Git clone this
2. Git clone this cubiomes fork https://github.com/IceTank/cubiomes into a different folder
3. Open this project folder in VS
4. File > New > Project from existing code
5. Follow the instructions Porject type C++ select cloned repositiory and give it a name for instance 'TerrainFinder' APPLICATION TYPE Console application Release options leave empty then press finish.
6. File > New > Project from existing code. Select cubiomes cloned repository name: 'cubiomes' type C++ Application type 'Static linked library' leave release options empty. 
7. Close the cubiomes project and reopen the TerrainFinder Project
8. Solution Folder 'TerrainFinder' Right click > Propertys Select all Configurations All Platforms > c/c++ > Additional include directorys > Add the cubiomes project folder
9. Solution Folder 'Solution 'THIS PROJECT (x of x projects)'' (at the top) Right click > Add > Existion Project select the cubiomes project file created from the cubiomes project
10. Solution Folder TerrainFinder > Add > Reference Select cubiomes hit ok
	If you get an error COM connection failed or something simelar try this guide: https://developercommunity.visualstudio.com/t/add-a-reference-raise-error-error-hresult-e-fail-h/260196#T-N273537
11. FormationFinder propertys Select all Platforms and all Targets > Linker > System > SubSystem > CONSOLE(SUBSYSTEM:/CONSOLE)
12. FormationFinder propertys Select Release all Platforms Linker > Input > Additionally Dependencys > Add <THIS PROJECT>/Release/cubiomes.lib
	This makes it so FormationFinder looks in its own release Folder for the library. 
13. Try running the project in release mode for x86 architecture.
