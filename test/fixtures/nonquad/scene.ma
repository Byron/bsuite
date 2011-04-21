//Maya ASCII 2008 scene
//Name: scene.ma
//Last modified: Thu, Apr 21, 2011 06:04:28 PM
//Codeset: UTF-8
requires maya "2008";
currentUnit -l centimeter -a degree -t film;
fileInfo "application" "maya";
fileInfo "product" "Maya Unlimited 2008";
fileInfo "version" "2008 Service Pack 1 x64";
fileInfo "cutIdentifier" "200802242307-718079";
fileInfo "osv" "Linux 2.6.32-29-generic #58-Ubuntu SMP Fri Feb 11 20:52:10 UTC 2011 x86_64";
createNode transform -s -n "persp";
	setAttr ".v" no;
	setAttr ".t" -type "double3" -15.237088826610465 18.043519375188119 38.095439788079588 ;
	setAttr ".r" -type "double3" -23.738352729603591 -21.79999999999988 0 ;
createNode camera -s -n "perspShape" -p "persp";
	setAttr -k off ".v" no;
	setAttr ".fl" 34.999999999999993;
	setAttr ".coi" 44.82186966202994;
	setAttr ".imn" -type "string" "persp";
	setAttr ".den" -type "string" "persp_depth";
	setAttr ".man" -type "string" "persp_mask";
	setAttr ".hc" -type "string" "viewSet -p %camera";
createNode transform -s -n "top";
	setAttr ".v" no;
	setAttr ".t" -type "double3" 0 100.1 0 ;
	setAttr ".r" -type "double3" -89.999999999999986 0 0 ;
createNode camera -s -n "topShape" -p "top";
	setAttr -k off ".v" no;
	setAttr ".rnd" no;
	setAttr ".coi" 100.1;
	setAttr ".ow" 30;
	setAttr ".imn" -type "string" "top";
	setAttr ".den" -type "string" "top_depth";
	setAttr ".man" -type "string" "top_mask";
	setAttr ".hc" -type "string" "viewSet -t %camera";
	setAttr ".o" yes;
createNode transform -s -n "front";
	setAttr ".v" no;
	setAttr ".t" -type "double3" 0 0 100.1 ;
createNode camera -s -n "frontShape" -p "front";
	setAttr -k off ".v" no;
	setAttr ".rnd" no;
	setAttr ".coi" 100.1;
	setAttr ".ow" 30;
	setAttr ".imn" -type "string" "front";
	setAttr ".den" -type "string" "front_depth";
	setAttr ".man" -type "string" "front_mask";
	setAttr ".hc" -type "string" "viewSet -f %camera";
	setAttr ".o" yes;
createNode transform -s -n "side";
	setAttr ".v" no;
	setAttr ".t" -type "double3" 100.1 0 0 ;
	setAttr ".r" -type "double3" 0 89.999999999999986 0 ;
createNode camera -s -n "sideShape" -p "side";
	setAttr -k off ".v" no;
	setAttr ".rnd" no;
	setAttr ".coi" 100.1;
	setAttr ".ow" 30;
	setAttr ".imn" -type "string" "side";
	setAttr ".den" -type "string" "side_depth";
	setAttr ".man" -type "string" "side_mask";
	setAttr ".hc" -type "string" "viewSet -s %camera";
	setAttr ".o" yes;
createNode transform -n "mesh";
createNode mesh -n "meshShape" -p "mesh";
	setAttr -k off ".v";
	setAttr ".uvst[0].uvsn" -type "string" "map1";
	setAttr ".cuvs" -type "string" "map1";
	setAttr ".dcc" -type "string" "Ambient+Diffuse";
	setAttr -s 25 ".vt[0:24]"  -4.2950082 0 8.3088675 -1.176228 0 8.3088675 -1.176228 
		0 5.1578989 -4.2950082 0 5.1578989 1.942551 0 8.3088675 1.942551 0 5.1578989 5.0613298 
		0 8.3088675 5.0613298 0 5.1578989 8.1801109 0 8.3088675 8.1801109 0 5.1578989 -1.176228 
		0 2.0069301 -4.2950082 0 2.0069301 1.942551 0 2.0069301 5.0613298 0 2.0069301 8.1801109 
		0 2.0069301 -1.176228 0 -1.144039 -4.2950082 0 -1.144039 1.942551 0 -1.144039 5.0613298 
		0 -1.144039 8.1801109 0 -1.144039 -1.176228 0 -4.2950082 -4.2950082 0 -4.2950082 
		1.942551 0 -4.2950082 5.0613298 0 -4.2950082 8.1801109 0 -4.2950082;
	setAttr -s 41 ".ed[0:40]"  0 1 0 1 2 0 2 3 0 3 0 0 1 4 0 4 5 0 5 2 0 4 6 0 6 7 0 
		7 5 0 6 8 0 8 9 0 9 7 0 2 10 0 10 11 0 11 3 0 5 10 0 5 12 0 12 10 0 7 13 0 13 12 
		0 9 14 0 14 13 0 10 15 0 15 16 0 16 11 0 12 17 0 17 15 0 13 18 0 18 17 0 14 19 0 
		19 18 0 15 20 0 20 21 0 21 16 0 17 22 0 22 20 0 18 23 0 23 22 0 19 24 0 24 23 0;
	setAttr -s 66 ".n[0:65]" -type "float3"  0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 
		0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 
		1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 
		0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 
		0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 
		1 0 0 1 0 0 1 0 0 1 0 0 1 0;
	setAttr -s 17 ".fc[0:16]" -type "polyFaces" 
		f 4 0 1 2 3 
		f 4 4 5 6 -2 
		f 4 7 8 9 -6 
		f 4 10 11 12 -9 
		f 4 -3 13 14 15 
		f 3 -7 16 -14 
		f 3 -17 17 18 
		f 4 -10 19 20 -18 
		f 4 -13 21 22 -20 
		f 4 -15 23 24 25 
		f 4 -19 26 27 -24 
		f 4 -21 28 29 -27 
		f 4 -23 30 31 -29 
		f 4 -25 32 33 34 
		f 4 -28 35 36 -33 
		f 4 -30 37 38 -36 
		f 4 -32 39 40 -38 ;
	setAttr ".cd" -type "dataPolyComponent" Index_Data Edge 0 ;
	setAttr ".cvd" -type "dataPolyComponent" Index_Data Vertex 0 ;
createNode lightLinker -n "lightLinker1";
	setAttr -s 2 ".lnk";
	setAttr -s 2 ".slnk";
createNode displayLayerManager -n "layerManager";
createNode displayLayer -n "defaultLayer";
createNode renderLayerManager -n "renderLayerManager";
createNode renderLayer -n "defaultRenderLayer";
	setAttr ".g" yes;
createNode script -n "sceneConfigurationScriptNode";
	setAttr ".b" -type "string" "playbackOptions -min 1 -max 24 -ast 1 -aet 48 ";
	setAttr ".st" 6;
select -ne :time1;
	setAttr ".o" 1;
select -ne :renderPartition;
	setAttr -s 2 ".st";
select -ne :renderGlobalsList1;
select -ne :defaultShaderList1;
	setAttr -s 2 ".s";
select -ne :postProcessList1;
	setAttr -s 2 ".p";
select -ne :lightList1;
select -ne :initialShadingGroup;
	setAttr ".ro" yes;
select -ne :initialParticleSE;
	setAttr ".ro" yes;
select -ne :hardwareRenderGlobals;
	setAttr ".ctrs" 256;
	setAttr ".btrs" 512;
select -ne :defaultHardwareRenderGlobals;
	setAttr ".fn" -type "string" "im";
	setAttr ".res" -type "string" "ntsc_4d 646 485 1.333";
connectAttr ":defaultLightSet.msg" "lightLinker1.lnk[0].llnk";
connectAttr ":initialShadingGroup.msg" "lightLinker1.lnk[0].olnk";
connectAttr ":defaultLightSet.msg" "lightLinker1.lnk[1].llnk";
connectAttr ":initialParticleSE.msg" "lightLinker1.lnk[1].olnk";
connectAttr ":defaultLightSet.msg" "lightLinker1.slnk[0].sllk";
connectAttr ":initialShadingGroup.msg" "lightLinker1.slnk[0].solk";
connectAttr ":defaultLightSet.msg" "lightLinker1.slnk[1].sllk";
connectAttr ":initialParticleSE.msg" "lightLinker1.slnk[1].solk";
connectAttr "layerManager.dli[0]" "defaultLayer.id";
connectAttr "renderLayerManager.rlmi[0]" "defaultRenderLayer.rlid";
connectAttr "lightLinker1.msg" ":lightList1.ln" -na;
connectAttr "meshShape.iog" ":initialShadingGroup.dsm" -na;
// End of scene.ma
