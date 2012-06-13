#############
About B-Suite
#############
The B-Suite (pronounced *Be Sweet*) is a conglomerate of maya plugins that I wrote because I really wanted to try something.
The respective plugins where written with quality in mind. Additionally you will find auto-tests, code-docs and end-user docs.
  
###########
Lidar Tools
###########

.. image:: http://old.byronimo.de/download/lidvismaya_ui.png
    :width: 900 px

The lidar visualization tools consist of a locator which efficiently reads LAS files and displays them. Various options allow you to set a tradeoff between display performance and memory consumption.
    
The lidar tools work for Maya 2008 to Maya 2013, on Linux and OSX.

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

PTexVis works for Maya 2008 to Maya 2013, on Linux and OSX.

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

.. note:: Tested only on Linux and OSX currently, but should be usable on Windows as well.

#######
Testing
#######

In short:

 * Make sure cmake knows where your tmrv executable is. Its part of the `mrv development framework<https://github.com/mrv-developers/mrv>`
 * run **ctest** to invoke the test-suite.

########
License
########
New BSD License
