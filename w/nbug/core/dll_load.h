#pragma once

void * hex_dll_open(const char * _name);
bool   hex_dll_close(void * _handle);
const char * hex_dll_get_error();
void * hex_dll_get_symbol(void * _handle, const char * _symbol);
