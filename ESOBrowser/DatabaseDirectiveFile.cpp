#include "DatabaseDirectiveFile.h"

DatabaseDirectiveFile::DatabaseDirectiveFile() : m_state(State::Global), m_buildingStructure(nullptr) {

}

DatabaseDirectiveFile::~DatabaseDirectiveFile() = default;

void DatabaseDirectiveFile::processLine(std::vector<std::string>& tokens) {
	switch (m_state) {
	case State::Global:
		if (tokens[0] == "STRUCT") {
			auto& structure = m_structures.emplace_back();
			m_buildingStructure = &structure;
			m_state = State::Structure;

			if (tokens.size() != 2)
				parseError("One extra token expected for STRUCT");
			
			structure.name = std::move(tokens[1]);
		}
		else if (tokens[0] == "DEF") {
			auto& structure = m_defs.emplace_back();
			m_buildingStructure = &structure;
			m_state = State::Structure;

			if (tokens.size() < 3)
				parseError("At least two extra tokens expected for DEF");

			structure.defIndex = std::stoul(tokens[1]);
			structure.name = std::move(tokens[2]);
			structure.version = 0;

			for (auto it = tokens.begin() + 3; it < tokens.end(); it++) {
				if (*it == "VERSION") {
					if (tokens.end() - it < 2) {
						parseError("No value is specified for VERSION");
					}
					structure.version = std::stoul(*++it);
				}
				else {
					parseError("Unknown extended token in DEF: '" + *it + "'");
				}
			}
		}
		else {
			parseError("Ether 'STRUCT' or 'DEF' expected, got '" + tokens[0] + "'");
		}
		break;

	case State::Structure:
		if (tokens[0] == "END") {
			m_state = State::Global;
			m_buildingStructure = nullptr;
		}
		else {
			parseError("Unexpected token '" + tokens[0] + "' in structure context");
		}
		break;
	}
}
