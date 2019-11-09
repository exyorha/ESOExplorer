#include "FilamentTypeHelpers.h"

#include <filament/Engine.h>

void FilamentEngineDeleter::operator()(filament::Engine* engine) const {
	filament::Engine::destroy(&engine);
}
