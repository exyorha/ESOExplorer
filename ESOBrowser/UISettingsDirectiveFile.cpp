#include "UISettingsDirectiveFile.h"

UISettingsDirectiveFile::UISettingsDirectiveFile() = default;

UISettingsDirectiveFile::~UISettingsDirectiveFile() = default;

void UISettingsDirectiveFile::processLine(std::vector<std::string> & tokens) {
	if (tokens.front() == "DEF_FIELD_AS_COLUMN") {
		if (tokens.size() != 2) {
			parseError("SEF_FIELD_AS_COLUMN: one more token expected");
		}

		defFieldsAsColumns.emplace_back(std::move(tokens[1]));
	}
	else {
		parseError("DEF_FIELD_AS_COLUMN expected");
	}
}
