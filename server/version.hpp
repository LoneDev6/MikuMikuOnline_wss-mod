#pragma once

#ifdef NDEBUG
#include "buildversion.hpp"
#endif

#define MMO_VERSION_TOSTRING_(val) #val
#define MMO_VERSION_TOSTRING(val) MMO_VERSION_TOSTRING_(val)

#define MMO_VERSION_MAJOR 0
#define MMO_VERSION_MINOR 1
#define MMO_VERSION_REVISION 6

#define MMO_PROTOCOL_VERSION 2

#ifdef MMO_VERSION_BUILD
#define MMO_VERSION_BUILD_TEXT " Build " MMO_VERSION_TOSTRING(MMO_VERSION_BUILD)
#else
#define MMO_VERSION_BUILD_TEXT
#endif

#define MMO_VERSION_TITLE "Miku Miku Online Server"

#define MMO_VERSION_TEXT (MMO_VERSION_TITLE " " MMO_VERSION_TOSTRING(MMO_VERSION_MAJOR) "." \
							MMO_VERSION_TOSTRING(MMO_VERSION_MINOR) "." MMO_VERSION_TOSTRING(MMO_VERSION_REVISION)\
							MMO_VERSION_BUILD_TEXT)
