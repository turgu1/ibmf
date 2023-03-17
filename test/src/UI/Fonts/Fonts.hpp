#pragma once

#include <cinttypes>

#include "Font.hpp"
//---
#include "IBMFDriver/IBMFFont.hpp"
#include "IBMFDriver/IBMFFontLow.hpp"

// For the IBMF Fonts, a single file contains multiple "face" sizes. A single
// object is created using the IBMFFontLow class that manage the whole .h file
// and "faces" are hooked through the IBMFFont class.

extern IBMFFontLow EC_Regular_75; // Instanciated in Fonts.cpp

extern IBMFFont FONT_EC_REGULAR_75BPI12PT; // Instanciated in Fonts.cpp
extern IBMFFont FONT_EC_REGULAR_75BPI14PT; // Instanciated in Fonts.cpp
// extern IBMFFont FONT_EC_REGULAR_75BPI17PT; // Instanciated in Fonts.cpp

extern IBMFFontLow ECSans_Regular_75; // Instanciated in Fonts.cpp

extern IBMFFont FONT_ECSANS_REGULAR_75BPI12PT; // Instanciated in Fonts.cpp
extern IBMFFont FONT_ECSANS_REGULAR_75BPI14PT; // Instanciated in Fonts.cpp
// extern IBMFFont FONTSANS_EC_REGULAR_75BPI17PT; // Instanciated in Fonts.cpp

extern IBMFFontLow EC_Regular_100; // Instanciated in Fonts.cpp

extern IBMFFont FONT_EC_REGULAR_100BPI12PT; // Instanciated in Fonts.cpp
// extern IBMFFont FONT_EC_REGULAR_100BPI14PT; // Instanciated in Fonts.cpp
// extern IBMFFont FONT_EC_REGULAR_100BPI17PT; // Instanciated in Fonts.cpp

extern IBMFFontLow ECSans_Regular_100; // Instanciated in Fonts.cpp

extern IBMFFont FONT_ECSANS_REGULAR_100BPI12PT; // Instanciated in Fonts.cpp
// extern IBMFFont FONT_ECSANS_REGULAR_100BPI14PT; // Instanciated in Fonts.cpp
// extern IBMFFont FONT_ECSANS_REGULAR_100BPI17PT; // Instanciated in Fonts.cpp