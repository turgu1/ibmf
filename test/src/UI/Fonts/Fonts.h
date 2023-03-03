#pragma once

#include <cinttypes>

#include "Font.h"
//---
#include "IBMFDriver/IBMFFont.hpp"
#include "IBMFDriver/IBMFFontLow.hpp"

// For the IBMF Fonts, a single file contains multiple "face" sizes. A single
// object is created using the IBMFFontLow class that manage the whole .h file
// and "faces" are hooked through the IBMFFont class.

extern IBMFFontLow EC_Regular_75; // Instanciated in Fonts.cpp

extern IBMFFont FONT_EC_REGULAR_75BPI12PT; // Instanciated in Fonts.cpp
extern IBMFFont FONT_EC_REGULAR_75BPI14PT; // Instanciated in Fonts.cpp
extern IBMFFont FONT_EC_REGULAR_75BPI17PT; // Instanciated in Fonts.cpp