#ifndef ESO_DATABASE_RECORD_H
#define ESO_DATABASE_RECORD_H

#include <variant>
#include <unordered_map>
#include <string>

class ESODatabaseRecord {
public:
	using Value = std::variant<std::monostate, unsigned long long>;

	ESODatabaseRecord();
	~ESODatabaseRecord();

	ESODatabaseRecord(const ESODatabaseRecord& other);
	ESODatabaseRecord& operator =(const ESODatabaseRecord& other);

	ESODatabaseRecord(ESODatabaseRecord&& other);
	ESODatabaseRecord& operator =(ESODatabaseRecord&& other);

	Value& addField(const std::string& name);

private:
	std::unordered_map<std::string, Value> m_fields;
	std::vector<std::string> m_fieldOrder;
};

#endif
