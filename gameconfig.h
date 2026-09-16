#pragma once

#include "tier0/wchartypes.h"

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

class CModule;

class CGameConfig
{
public:
	CGameConfig(const std::string& path);
	~CGameConfig();

	bool Init(char *conf_error, int conf_error_size);
	const std::string GetPath();
	const char *GetLibrary(const std::string& name);
	const char *GetSignature(const std::string& name);
	const char* GetSymbol(const char *name);
	CModule **GetModule(const char *name);
	bool IsSymbol(const char *name);
	void *ResolveSignature(const char *name);
	static std::string GetDirectoryName(const std::string &directoryPathInput);
	static int ParseHexNibble(char c);
	static bool ParsePatternBytes(const char *pattern, std::vector<uint8_t> &bytes, std::vector<uint8_t> &mask);
	static bool IDASigToPattern(const char *signature, std::vector<uint8_t> &bytes, std::vector<uint8_t> &mask);

private:
	std::string m_szPath;
	std::unordered_map<std::string, std::string> m_umSignatures;
	std::unordered_map<std::string, std::string> m_umLibraries;
};