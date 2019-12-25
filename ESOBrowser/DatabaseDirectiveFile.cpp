#include "DatabaseDirectiveFile.h"

DatabaseDirectiveFile::DatabaseDirectiveFile() : m_state(State::Global), m_buildingStructure(nullptr), m_buildingEnum(nullptr) {

}

DatabaseDirectiveFile::~DatabaseDirectiveFile() = default;

void DatabaseDirectiveFile::processLine(std::vector<std::string>& tokens) {
	static const std::unordered_map<std::string, FieldType> fieldTypes{
		{ "UINT16", FieldType::UInt16 },
		{ "UINT32", FieldType::UInt32 },
		{ "UINT64", FieldType::UInt64 },
		{ "STRING", FieldType::String },
		{ "ENUM", FieldType::Enum }
	};

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
		else if (tokens[0] == "ENUM") {
			auto& enumd = m_enums.emplace_back();
			m_buildingEnum = &enumd;
			m_state = State::Enum;

			if (tokens.size() != 2)
				parseError("One extra token expected for ENUM");

			enumd.name = std::move(tokens[1]);
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
			auto it = fieldTypes.find(tokens[0]);
			if (it == fieldTypes.end()) {
				parseError("Unexpected token '" + tokens[0] + "' in structure context");
			}

			auto& field = m_buildingStructure->fields.emplace_back();
			field.type = it->second;

			auto tokenIt = tokens.begin();
			++tokenIt;

			if (field.type == FieldType::Enum) {
				if (tokenIt == tokens.end())
					parseError("Expected type name after ENUM");

				field.typeName = *tokenIt;
				++tokenIt;
			}

			if (tokenIt != tokens.end()) {
				field.name = *tokenIt;
				++tokenIt;
			}

			if (tokenIt != tokens.end()) {
				parseError("Unexpected token: " + *tokenIt);
			}
		}
		break;

	case State::Enum:
		if (tokens[0] == "END") {
			m_state = State::Global;
			m_buildingEnum = nullptr;
		}
		else if (tokens[0] == "VALUES") {
			if (tokens.size() != 3)
				parseError("Exactly two extra tokens expected for VALUES");

			auto firstValue = std::stoi(tokens[1]);
			auto lastValue = std::stoi(tokens[2]);

			for (auto i = firstValue; i <= lastValue; i++) {
				m_buildingEnum->values.emplace_back(i);
			}
		}
		else {
			parseError("Unexpected token '" + tokens[0] + "' in enum context");
		}
		break;
	}
}
