#ifndef DATABASE_DIRECTIVE_FILE_H
#define DATABASE_DIRECTIVE_FILE_H

#include "DirectiveFile.h"

#include <string>
#include <unordered_map>

class DatabaseDirectiveFile final : public DirectiveFile {
public:
	struct Structure {
		unsigned int defIndex;
		std::string name;
		unsigned int version;
	};

	struct Enum {
		std::string name;
		std::vector<int32_t> values;
		std::unordered_map<int32_t, std::string> valueNames;
	};

	DatabaseDirectiveFile();
	~DatabaseDirectiveFile();

	inline std::vector<Structure>& structures() { return m_structures; }
	inline std::vector<Enum>& enums() { return m_enums; }
	inline std::vector<Structure>& defs() { return m_defs; }

protected:
	void processLine(std::vector<std::string>& tokens) override;

private:
	enum class State {
		Global,
		Structure,
		Enum
	};

	State m_state;
	std::vector<Structure> m_structures;
	std::vector<Structure> m_defs;
	std::vector<Enum> m_enums;
	Structure* m_buildingStructure;
	Enum* m_buildingEnum;
};

#endif
