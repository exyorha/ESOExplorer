#include "Granny2TypeHelpers.h"

#include <granny.h>

void GrannyFileDeleter::operator()(granny_file* file) const {
	GrannyFreeFile(file);
}
