.. image:: http://old.byronimo.de/download/ptexmaya.png
    :width: 900 px

PtexMaya is a conglomerate of tools to handle ptex textures within maya. For now it is only used as research project and to obtain an understanding on how ptex works.

PtexMaya works for Maya 2008 to Maya 2012.

########
Features
########
* **Ptexture Viewport Visualization (ptxVisNode)**

 * Display ptextures directly in the viewport using opengl points
 
  * 3 different sampling modes, *Texel* mode displays raw texel tiles without the need for a mesh.
  
 * Provide ptexture meta information of the loaded file
 * *Limitations*
 
  * Can currently only sample Triangle meshes when *not* only displaying pure texel samples. 

########
Building
########
PtexMaya uses cmake to setup its build environment. 
For a successful build, you need the headers for Ptex (https://github.com/wdas/ptex) as well as a compiled static or dynamic library. Additionally you require a maya installation (version 2008 to 2012).

First, you generate your build configuration, then you perform the actual build::
    
    cd ptexmaya
    cmake .
    # this fails as you have to setup the paths to ptex at least
    cmake-gui .
    # setup the PTEX variables, otherwise the configuration will fail
    # Hit generate in the UI or execute cmake . 
    
    # Linux|OSX: build the plugin for all maya versions
    make
    
    # Windows: Open visual studio and build from there
    
Adjust the plugin and script path for maya to find your newly compiled plugin::
    
    export MAYA_SCRIPT_PATH=src/mel
    export MAYA_PLUG_IN_PATH=src/2012/lib

Launch maya and load the plugin::
    
    loadPlugin("libptexmaya")
    
    # create a node and connect a mesh (which matches the the ptex file)
    # See http://ptex.us for examples
    createNode "ptexVisNode";
    connectAttr "mymesh.outMesh" "ptexVisNode1.inMesh"
    
    # In the attribute editor, select a ptx texture to display.
    # You will see error messages in the viewport if something doesn't work.

.. note:: Tested only on linux currently, but should be usable on Windows and OSX as well.


########
License
########
New BSD License
