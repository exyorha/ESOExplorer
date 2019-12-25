#ifndef UI_SETTINGS_DIRECTIVE_FILE_H
#define UI_SETTINGS_DIRECTIVE_FILE_H

#include "DirectiveFile.h"

class UISettingsDirectiveFile final : public DirectiveFile {
public:
	UISettingsDirectiveFile();
	~UISettingsDirectiveFile();

	std::vector<std::string> defFieldsAsColumns;

protected:
	void processLine(std::vector<std::string>& tokens) override;
};

#endif
