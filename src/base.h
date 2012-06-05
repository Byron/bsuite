#ifndef BASE_H
#define BASE_H

#ifdef WIN32
	#define EXPORT __declspec( dllexport )
#else
	#define EXPORT extern "C" __attribute__ ((visibility("default")))
#endif

#endif // BASE_H
