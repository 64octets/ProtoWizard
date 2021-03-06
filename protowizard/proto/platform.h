#pragma once

#include <string>

namespace protowizard
{
	void ShowMessageBox( const std::string &message, const std::string &title );
	std::string GetExePath();
	std::string OpenFileDlg(const std::string &path, const std::string &extension);
}
