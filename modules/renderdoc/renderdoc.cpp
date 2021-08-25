#include "renderdoc.h"

#include "core/error/error_macros_v.h"
#include "core/string/ustring.h"
#include "core/string/print_string.h"
#include "thirdparty/renderdoc/renderdoc_app.h"

#include <process.h>
//#include <regstr.h>
#include <shlobj.h>

inline void printf_verbose(String format, const Variant& p0) {
	print_verbose(vformat(format, p0));
}

namespace RenderDoc {

RENDERDOC_API_1_1_2 *rdoc_api = NULL;

void LoadAPI() {
	const char *subKey = "SOFTWARE\\Classes\\RenderDoc.RDCCapture.1\\DefaultIcon\\";
	const char *dllName = "renderdoc.dll";
	const int dllNameSize = strlen(dllName);

	// Query the installed renderdoc path via registery
	char *rdgPath = nullptr;
	int dirEnd = 0;
	{
		HKEY key;
		if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, subKey, 0, KEY_READ | KEY_WOW64_64KEY, &key) == ERROR_SUCCESS) {
			DWORD size;
			if (RegQueryValueEx(key, "", NULL, NULL, NULL, &size) == ERROR_SUCCESS) {
				rdgPath = new char[size + dllNameSize];
				if (RegQueryValueEx(key, "", NULL, NULL, (LPBYTE)rdgPath, &size) == ERROR_SUCCESS) {
					// Get directory
					dirEnd = size - 1;
					while ((dirEnd >= 0) && (rdgPath[dirEnd] != '\\')) {
						--dirEnd;
					}
				}
			}
		}
	}

	if (!!rdgPath && (dirEnd > 0)) {

		// Append the dll file name to the directory path
		for (int i = 0; i <= dllNameSize; ++i) {
			rdgPath[dirEnd + 1 + i] = dllName[i];
		}
		// Load the dll
		if (HMODULE mod = LoadLibrary(rdgPath)) {
			printf_verbose("Load RenderDoc from %s.", rdgPath);
			pRENDERDOC_GetAPI RENDERDOC_GetAPI = (pRENDERDOC_GetAPI)GetProcAddress(mod, "RENDERDOC_GetAPI");
			int ret = RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_1_2, (void **)&rdoc_api);
			if (ret != 1) {
				WARN_PRINTF("\tFailed to get RenderDoc API (ret=%d).", ret);
			}
		} else {
		}
	} else {
		WARN_PRINT("Failed to load RenderDoc; it is not installed on you system, or the installation is broken.");
	}

	if (!!rdgPath) {
		delete[] rdgPath;
	}
}

// Load the RenderDoc API, and do some setup.
void Init() {
	LoadAPI();


}

void TriggerCapture() {
	if (rdoc_api) {
		rdoc_api->TriggerCapture();
	}
}

} // namespace RenderDoc
