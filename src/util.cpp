#include "util.h"
#include <maya/MFnAttribute.h>

void setup_output(MFnAttribute& attr)
{
	attr.setStorable(false);
	attr.setWritable(false);
}



