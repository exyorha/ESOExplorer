#ifndef DATABASE_DIRECTIVE_FILE_H
#define DATABASE_DIRECTIVE_FILE_H

#include "DirectiveFile.h"

#include <string>
#include <unordered_map>

class DatabaseDirectiveFile final : public DirectiveFile {
public:
	enum class FieldType {
		Int8,
		Int16,
		Int32,
		Int64,
		UInt8,
		UInt16,
		UInt32,
		UInt64,
		Float,
		Enum, // Physically Int32
		String,
		Array,
		ForeignKey,
		AssetReference,
		Boolean,
		Struct,
		PolymorphicReference
	};

	struct StructureField {
		FieldType type;
		FieldType arrayType;
		std::string typeName;

		std::string name;
	};

	struct Structure {
		unsigned int defIndex;
		std::string name;
		unsigned int version;
		std::vector<StructureField> fields;
	};

	struct DefAlias {
		unsigned int defIndex;
		std::string name;
		std::string targetName;
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
	inline std::vector<DefAlias>& defAliases() { return m_defAliases; }

protected:
	void processLine(std::vector<std::string>& tokens) override;

private:
	enum class State {
		Global,
		Structure,
		Enum
	};

	void parseFieldType(std::vector<std::string>::const_iterator& it, const std::vector<std::string>::const_iterator& endIt, DatabaseDirectiveFile::StructureField& field, bool inArray);

	State m_state;
	std::vector<Structure> m_structures;
	std::vector<Structure> m_defs;
	std::vector<DefAlias> m_defAliases;
	std::vector<Enum> m_enums;
	Structure* m_buildingStructure;
	Enum* m_buildingEnum;
};

#endif
