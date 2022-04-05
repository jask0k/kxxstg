#include <nbug/core/dll_load.h>

#ifdef _WIN32
#	include <windows.h>

void * hex_dll_open(const char * _name)
{
	return LoadLibraryA(_name);
}

bool hex_dll_close(void * _handle)
{
	return FreeLibrary((HMODULE)_handle) ? true : false;
}

const char * hex_dll_get_error()
{
	// return dlerror();
	return "";
}

void * hex_dll_get_symbol(void * _handle, const char * _symbol)
{
	return (void*)GetProcAddress((HMODULE)_handle, _symbol);
}

#else
#	include <dlfcn.h>

void * hex_dll_open(const char * _name)
{
	return dlopen(_name, RTLD_NOW);
}

bool hex_dll_close(void * _handle)
{
	return dlclose(_handle) == 0;
}

const char * hex_dll_get_error()
{
	return dlerror();
}

void * hex_dll_get_symbol(void * _handle, const char * _symbol)
{
	return dlsym(_handle, _symbol);
}

#endif
