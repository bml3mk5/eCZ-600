/** @file loadlibrary.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2016.11.10 -

	@brief [ load dynamic library ]
*/

#ifndef LOADLIBRARY_H
#define LOADLIBRARY_H

#ifndef _WIN32
#include <dlfcn.h>
#endif

#if defined(_WIN32)

#define LOAD_LIB(handle, lib_base, version) { \
	char libname[64]; \
	strcpy(libname, lib_base); \
	if (version) { \
		sprintf(&libname[strlen(libname)], "-%d", version); \
	} \
	strcat(libname, ".dll"); \
	handle = LoadLibraryA(libname); \
	if (!handle) { \
		logging->out_logf(LOG_DEBUG, "Cannot load %s.", libname); \
		continue; \
	} else { \
		logging->out_logf(LOG_DEBUG, "Loaded %s.", libname); \
	} \
}
#define GET_ADDR(func, func_type, handle, func_name) { \
	func = (func_type) GetProcAddress(handle, func_name); \
	if (!func) { \
		logging->out_logf(LOG_DEBUG, "Cannot get address of %s.", func_name); \
		continue; \
	} \
}
#define GET_ADDR_OPTIONAL(func, func_type, handle, func_name) { \
	func = (func_type) GetProcAddress(handle, func_name); \
	if (!func) { \
		logging->out_logf(LOG_DEBUG, "Cannot get address of %s.", func_name); \
	} \
}
#define UNLOAD_LIB(handle) { \
	if (handle) FreeLibrary(handle); \
	handle = NULL; \
}

#else /* !_WIN32 */

#if defined(__APPLE__) && defined(__MACH__)
#define LOAD_LIB(handle, lib_base, version) { \
	char libname[64]; \
	strcpy(libname, "lib"); \
	strcat(libname, lib_base); \
	if (version) { \
		sprintf(&libname[strlen(libname)], ".%d", version); \
	} \
	strcat(libname, ".dylib"); \
	handle = dlopen(libname, RTLD_NOW | RTLD_GLOBAL); \
	if (!handle) { \
		logging->out_logf(LOG_DEBUG, _T("Cannot load %s."), libname); \
		continue; \
	} else { \
		logging->out_logf(LOG_DEBUG, _T("Loaded %s."), libname); \
	} \
}
#else
#define LOAD_LIB(handle, lib_base, version) { \
	char libname[64]; \
	strcpy(libname, "lib"); \
	strcat(libname, lib_base); \
	strcat(libname, ".so"); \
	if (version) { \
		sprintf(&libname[strlen(libname)], ".%d", version); \
	} \
	handle = dlopen(libname, RTLD_NOW | RTLD_GLOBAL); \
	if (!handle) { \
		logging->out_logf(LOG_DEBUG, _T("Cannot load %s."), libname); \
		continue; \
	} else { \
		logging->out_logf(LOG_DEBUG, _T("Loaded %s."), libname); \
	} \
}
#endif
#define GET_ADDR(func, func_type, handle, func_name) { \
	func = (func_type)dlsym(handle, func_name); \
	if (!func) { \
		logging->out_logf(LOG_DEBUG, _T("Cannot get address of %s."), func_name); \
		continue; \
	} \
}
#define GET_ADDR_OPTIONAL(func, func_type, handle, func_name) { \
	func = (func_type)dlsym(handle, func_name); \
	if (!func) { \
		logging->out_logf(LOG_DEBUG, _T("Cannot get address of %s."), func_name); \
	} \
}
#define UNLOAD_LIB(handle) { \
	if (handle) dlclose(handle); \
	handle = NULL; \
}

#endif

#define CHECK_VERSION(current, require, libname) { \
	if (current < require) { \
		logging->out_logf(LOG_DEBUG, "Cannot use %s because version is different from %d.", libname, (int)require); \
		continue; \
	} \
}

#endif /* LOADLIBRARY_H */
