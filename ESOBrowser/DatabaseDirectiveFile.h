#ifndef DATABASE_DIRECTIVE_FILE_H
#define DATABASE_DIRECTIVE_FILE_H

#include "DirectiveFile.h"
#include <string>

class DatabaseDirectiveFile final : public DirectiveFile {
public:
	struct Structure {
		unsigned int defIndex;
		std::string name;
		unsigned int version;
	};

	DatabaseDirectiveFile();
	~DatabaseDirectiveFile();

	inline std::vector<Structure>& structures() { return m_structures; }
	inline std::vector<Structure>& defs() { return m_defs; }

protected:
	void processLine(std::vector<std::string>& tokens) override;

private:
	enum class State {
		Global,
		Structure
	};

	State m_state;
	std::vector<Structure> m_structures;
	std::vector<Structure> m_defs;
	Structure* m_buildingStructure;
};

#endif
