#pragma once

#include "shapes.h"

enum draw { CURRENT, BUFFER, BACKUP, RESTORE };
enum Tools { PEN, LINE, RECTANGLE, ELLIPSE, POLY, TEXT, NONE };

#define BACKUPS 50
#define DEFAULT_ZOOM 1
#define DELTA_ZOOM 1.1
