#ifndef MAYA_UTIL_H
#define MAYA_UTIL_H


class MFnAttribute;
class PtexCache;

extern PtexCache* gCache;	//!< Global static cache to be used by all facilities that need ptextures

void setup_output(MFnAttribute& attr);

#endif // MAYA_UTIL_H
