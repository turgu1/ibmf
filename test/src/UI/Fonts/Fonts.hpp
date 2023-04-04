#pragma once

#include <cinttypes>

#include "Font.hpp"
//---
#include "IBMFDriver/IBMFFont.hpp"
#include "IBMFDriver/IBMFFontLow.hpp"

// For the IBMF Fonts, a single file contains multiple "face" sizes. A single
// object is created using the IBMFFontLow class that manage the whole .h file
// and "faces" are hooked through the IBMFFont class.

extern IBMFFontLow tahoma75; // Instanciated in Fonts.cpp

extern IBMFFont fontTahoma75BPI9PT;  // Instanciated in Fonts.cpp
extern IBMFFont fontTahoma75BPI10PT; // Instanciated in Fonts.cpp
extern IBMFFont fontTahoma75BPI12PT; // Instanciated in Fonts.cpp
extern IBMFFont fontTahoma75BPI14PT; // Instanciated in Fonts.cpp
