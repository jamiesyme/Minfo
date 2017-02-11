#pragma once

// A module is a rendered component that's controlled and managed by the hub.
// You can think of a module as a widget in MInfo. We will have a module to
// display the time, another module to display the date, etc.

// ModuleType is stored in a separate file because some other files (namely
// minfo.c and minfo-msg.c) only require the module type information.
#include "module-type.h"
#include "time-utils.h"

typedef struct Surface Surface;
typedef struct Module Module;

typedef void (*ModuleRenderFunc)(Module*, Surface*);
typedef void (*ModuleFreeFunc)(Module*);

// A module can be in one of three states:
//   MS_OFF:         Nothing is rendered.
//                   Related command: `msg-minfo hide`
//   MS_ON_CONSTANT: Module is rendered with a constant/full opacity.
//                   Related command: `msg-minfo show`
//   MS_ON_DYNAMIC:  This is for module updates. The module is faded in, rendered
//                   at full opacity for some duration, and then faded out.
//                   Related command: `msg-minfo show-update ...`
typedef enum ModuleState {
  MS_OFF,
  MS_ON_CONSTANT,
  MS_ON_DYNAMIC
} ModuleState;

// The module opacity behaviour changes depending on the module's state:
//   MS_OFF:         Opacity is 0.
//   MS_ON_CONSTANT: Opacity is `full`.
//   MS_ON_DYNAMIC:  Uses the specified durations and time accumulator to
//                   calculate the current opacity. Opacity is in the range
//                   [0, full].
typedef struct ModuleOpacity {
  float full;
  Milliseconds fadeInDuration;
  Milliseconds holdDuration;
  Milliseconds fadeOutDuration;
  Milliseconds timeAccum;
} ModuleOpacity;

struct Module {
  // Read-only after init.
  // Used to identify module when running `minfo-msg show-update ...`
  // Default: MT_UNKNOWN
  ModuleType type;

  // Read-only after init.
  // Used to determine window size, as well as to calculate module position.
  // Default: 0
  unsigned int width;
  unsigned int height;

  // Opacity depends on state, so use setModuleState() to change this.
  // Default: MS_OFF
  ModuleState state;

  // Do not set timeAccum directly: it is controlled by setModuleState() and
  // updateModuleOpacity().
  // Defaults:
  //   full: 0.8
  //   fadeInDuration: 50
  //   holdDuration: 1000
  //   fadeOutDuration: 500
  //   timeAccum: 0
  ModuleOpacity opacityObj;

  // Called once per frame to render the module if state is not MS_OFF.
  // Function should respect opacityObj.
  // Default: NULL
  ModuleRenderFunc renderFunc;

  // Called once during teardown to free the extraData member.
  // Default: NULL
  ModuleFreeFunc freeFunc;

  // Optional module-specific data.
  // Default: NULL
  void* extraData;
};


// Sets the default values specified above.
void initModule(Module* module);

// Sets the state and updates the opacity time accumulator.
void setModuleState(Module* module, ModuleState state);

// Adds delta to opacity time accumulator. Sets state to MS_OFF when module has
// finished fading out.
// Returns updated opacity.
float updateModuleOpacity(Module* module, Milliseconds delta);

// Calculates the current opacity based on the rules outlined at the
// ModuleOpacity definition.
float getModuleOpacity(Module* module);
