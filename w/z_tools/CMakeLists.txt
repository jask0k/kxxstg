project(z_tools)

add_executable(apng apng.cc)
target_link_libraries(apng nbug)

add_executable(nblang nblang.cc)
target_link_libraries(nblang nbug)

add_executable(nbpack nbpack.cc)
target_link_libraries(nbpack nbug)


#if(WIN32)
#	add_executable(harmonic harmonic.cc)
#	target_link_libraries(harmonic nbug comctl32)
#endif()
