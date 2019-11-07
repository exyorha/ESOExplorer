#ifndef DIRECTIVE_FILE_H
#define DIRECTIVE_FILE_H

#include <filesystem>

class DirectiveFile {
protected:
	DirectiveFile();
	~DirectiveFile();

public:
	DirectiveFile(const DirectiveFile& other) = delete;
	DirectiveFile &operator =(const DirectiveFile& other) = delete;

	void parseFile(const std::filesystem::path& path);

	inline const std::filesystem::path& filePath() const {
		return m_filePath;
	}

protected:
	virtual void processLine(std::vector<std::string>& tokens) = 0;
	[[noreturn]] void parseError(const std::string& error);

private:
	std::filesystem::path m_filePath;
};

#endif
