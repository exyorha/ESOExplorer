#include "DirectiveFile.h"

#include <fstream>

DirectiveFile::DirectiveFile() = default;

DirectiveFile::~DirectiveFile() = default;

void DirectiveFile::parseFile(const std::filesystem::path& path) {
	m_filePath = path;

	std::ifstream stream;
	stream.exceptions(std::ios::badbit | std::ios::failbit);
	stream.open(path);
	stream.exceptions(std::ios::badbit);


	enum {
		Normal,
		String,
		Escaped,
		Comment
	} lexerState = Normal;
	std::vector<std::string> tokens;
	std::string tokenBuffer;

	char character;
	bool tokenBufferActive = false;

	while (true) {
		stream.get(character);

		if (stream.fail())
			break;

		switch (lexerState) {
		case Normal:
			if (character == '"') {
				tokenBufferActive = true;
				lexerState = String;
			}
			else if (character == ';') {
				lexerState = Comment;
			}
			else if (isspace((unsigned char)character)) {
				if (tokenBufferActive) {
					tokens.push_back(tokenBuffer);
					tokenBuffer.clear();
					tokenBufferActive = false;
				}

				if (character == '\n' && tokens.size() != 0) {
					processLine(tokens);
					tokens.clear();
				}
			}
			else {
				tokenBuffer.push_back(character);
				tokenBufferActive = true;
			}

			break;

		case String:
			if (character == '\\')
				lexerState = Escaped;
			else if (character == '"')
				lexerState = Normal;
			else
				tokenBuffer.push_back(character);

			break;

		case Escaped:
			tokenBuffer.push_back(character);
			lexerState = String;

			break;

		case Comment:
			if (character == '\n') {
				if (tokenBufferActive) {
					tokens.push_back(tokenBuffer);
					tokenBuffer.clear();
					tokenBufferActive = false;
				}

				if (tokens.size() != 0) {
					processLine(tokens);
					tokens.clear();
				}

				lexerState = Normal;
			}

			break;
		}
	}

	if (lexerState != Normal)
		parseError("End of file reached before closing quote");

	if (tokenBufferActive || !tokens.empty())
		parseError("No newline at the end of file");
}

[[noreturn]] void DirectiveFile::parseError(const std::string& error) {
	throw std::runtime_error(m_filePath.string() + ": " + error);
}
