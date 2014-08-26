#############
About B-Suite
#############
The B-Suite (pronounced *Be Sweet*) is a conglomerate of maya plugins that I wrote because I really wanted to try something.
The respective plugins where written with quality in mind. Additionally you will find auto-tests, code-docs and end-user docs.

All plugins and tools work on Linux, OSX and Windows.

########
Download
########

The plugins have been compiled on *Linux*, *OSX* and *Windows*, for maya *2013*, *2012* and *2011*, and on some platforms for even older maya versions.

* *Linux*

 * https://dl.dropbox.com/u/17820808/bsuite/linux/bsuite_linux_x64.zip
 
* *OSX*

 * https://dl.dropbox.com/u/17820808/bsuite/osx/bsuite_osx_x64.zip
 
* *Windows*

 * https://dl.dropbox.com/u/17820808/bsuite/windows/bsuite_windows_x64.zip

============
Installation
============

Extract the archive destined for your operating system into a place of your choice.

In your *Maya.env* file, setup the *MAYA_PLUG_IN_PATH* environment variable to point to the plugin directory containing the plugins compiled for it.
Additionally, set the *MAYA_SCRIPT_PATH* to the *mel* subdirectory.

Start maya and load the plugins, which should be listed in your plugin editor.

An example Maya.env file might look like this::
    
    MAYA_PLUG_IN_PATH=/home/user/downloads/bsuite/2012
    MAYA_SCRIPT_PATH=/home/user/downloads/bsuite/mel
    
###########
Lidar Tools
###########

.. image:: http://old.byronimo.de/download/lidvismaya_ui.png
    :width: 900 px

The lidar visualization tools consist of a locator which efficiently reads LAS files and displays them. Various options allow you to set a tradeoff between display performance and memory consumption.
    
====================
Lidar Tools Features
====================

* **Lidar LAS-File Viewport Visualization (lidarVisNode)**

 * Visualize LAS files efficiently as *point cloud* in the viewport
 
  * Supports LAS file format 1.3 and point formats version 0 through 5
  
 * Show LAS file header information 
 * Choose from multiple colorization modes, which include
 
  * *No Color* (for maximum performance)
  * *Intensity*
  * *Classification*
  * *Classification with Intensity*
  * *Stored Color*
  
 * Display any amount of points without the fear of *out-of-memory* issues.
 * Speedup reading performance using *memory mapping* (currently POSIX only)
 * Speedup display performance using *system* or *GPU* caches.
   
########
PTexVis
########

.. image:: http://old.byronimo.de/download/ptexmaya_ui.png
    :width: 900 px

PTexVis is a locator which can display ptex textures s point clouds on top of the corresponding mesh. For now it is only used as research project and to obtain an understanding on how ptex works.

================
PTexVis Features
================
* **Ptexture Viewport Visualization (ptxVisNode)**

 * Display ptextures directly in the viewport using opengl points
 
  * 3 different sampling modes, *Texel* mode displays raw texel tiles without the need for a mesh.
  
 * Show ptexture meta information of the loaded file
 * Fast display of large amounts of samples using *GPU Caching*. Caching in main memory is supported as well.
 * *Multi-threaded* ptex sampling if interpolation mode is '*Point*'
 * *Limitations*
 
  * Can currently only sample *triangle* meshes when *not* only displaying pure texel samples. 
  

###################
Curvature HW Shader
###################

.. image:: http://old.byronimo.de/download/crvshader2.png
    :width: 900 px

Implements a (by now) legacy viewport hardware shader to display the curvature of triangle-vertices relative to the interpolated direction of their respective triangle.

It can help to estimate overall smoothness of a surface.

================
Features
================

* Display curvature on meshes in real-time
* supports display-based mesh-smoothing (as when pressing 1,2, 3 on the keyboard)
* flat-shaded mode and one that incorporates scene lights
* customize mapping of angles to colors using a ramp
* adapt the shader to your scene's scale using a simple scale slider (required on windows only)


#################
Byrons Poly Tools
#################

.. image:: http://old.byronimo.de/img/content/main/BPTImage.gif
.. image:: http://old.byronimo.de/img/content/products/BPT/mov_stt.gif
.. image:: http://old.byronimo.de/img/content/products/BPT/mov_chamfer.gif
.. image:: http://old.byronimo.de/img/content/products/BPT/mov_smartsplit.gif

This plugin is my very first C++ project, and was written more than 10 years ago. This, of course, shows in the code, which is a premium example on how to **not** write code. Code-wise, its clearly a sin of my youth, however, it still works and is usable, which is why I added it to the bsuite.

On the end-user side, you will find various tools to speed up your modeling and to make it more convenient.

The online documentation can be found here: http://old.byronimo.de/docs/BPT/1.0

Please note that this is **legacy** code which is entirely unsupported in case you want to use it.

============
BPT Features
============

* Tweak your Meshes in realtime and split trough it almost instantly.
* You can procedurally slide or push your split, or align it to it's boundary.
* Soft Selection functionality is directly integrated into the node and can be used in conjuctions with every action which changes the mesh's topology.
* The Tools will not be limited to a specific component type, and you decide whether you want to work with vertices, edges or faces. Additionally, Smart Split is actually 3 in 1: Split through geometry, poke faces or connect vertices.
* Byrons Poly Tools bring to you a chamfer and solid chamfer. Due to new algorithms it will give higher quality bevels compared to other packages.
 

########
Building
########
The B-Suite build system uses cmake to setup its build environment. 
For a successful build you need the headers for Ptex (https://github.com/wdas/ptex) as well as a compiled static or dynamic library. Additionally you require a maya installation (version 2008 to 2012).

For your convenience, ptex is included as git-submodule which can be retrieved rather easily.
After cloning the repository, make sure you run::
    
    # get a clone of ptex
    cd bsuite
    git submodule init
    git submodule update
    
    # create the ptex library
    make -C 3rdParty/ptex

First, you generate your build configuration, then you perform the actual build::
    
    mkdir -p build/release
    cd build/release
    cmake ../..
    
    # this fails as you have to setup some paths - see the error message
    # for more info and run the cmake gui to perform the configuration.
    cmake-gui .
    # setup the PTEX variables, otherwise the configuration will fail for PTexVis.
    # Hit generate in the UI or execute cmake . 
    
    # Linux|OSX: build the plugin for all configured maya versions
    make
    
    # Windows: Open visual studio and build from there
    
Adjust the maya plugin and script path for maya to find your newly compiled plugin(s)::
    
    export MAYA_SCRIPT_PATH=src/mel
    export MAYA_PLUG_IN_PATH=bin/release/2012

Launch maya and load the plugin::
    
    loadPlugin("ptexvis")
    
    # create a node and connect a mesh (which matches the the ptex file)
    # See http://ptex.us for examples
    createNode "ptexVisNode";
    connectAttr "mymesh.outMesh" "ptexVisNode1.inMesh"
    
    # In the attribute editor, select a ptx texture to display.
    # You will see error messages in the viewport if something doesn't work.


#######
Testing
#######

In short:

 * Make sure cmake knows where your tmrv executable is. Its part of the `mrv development framework<https://github.com/mrv-developers/mrv>`_
 * run **ctest** to invoke the test-suite.

########
License
########
New BSD License
