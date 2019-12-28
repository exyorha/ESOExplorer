#ifndef ESO_DATABASE_RECORD_H
#define ESO_DATABASE_RECORD_H

#include <variant>
#include <unordered_map>
#include <string>

#include "DatabaseDirectiveFile.h"

class ESODatabaseDef;

struct ESOValueStruct;

struct ESOFieldContainer {
	struct ValueEnum {
		const DatabaseDirectiveFile::Enum* definition;
		int32_t value;
	};

	struct ValueArray;

	struct ValueForeignKey {
		std::string def;
		uint32_t id;
	};

	struct ValueAssetReference {
		uint32_t id;
	};

	using ValueStruct = ESOValueStruct;

	using Value = std::variant<std::monostate, long long, unsigned long long, ValueEnum, std::string, ValueArray, ValueForeignKey, bool, double, ValueAssetReference, ValueStruct>;

	struct ValueArray {
		std::vector<Value> values;
	};

	Value& addField(const std::string& name);
	const Value& findField(const std::string& name) const;

	inline const std::vector<std::string>& fieldOrder() const { return m_fieldOrder; }

private:
	std::unordered_map<std::string, Value> m_fields;
	std::vector<std::string> m_fieldOrder;
};

struct ESOValueStruct final : public ESOFieldContainer {

};

class ESODatabaseRecord final : public ESOFieldContainer {
public:


	ESODatabaseRecord();
	~ESODatabaseRecord();

	ESODatabaseRecord(const ESODatabaseRecord& other);
	ESODatabaseRecord& operator =(const ESODatabaseRecord& other);

	ESODatabaseRecord(ESODatabaseRecord&& other);
	ESODatabaseRecord& operator =(ESODatabaseRecord&& other);
};

#endif
