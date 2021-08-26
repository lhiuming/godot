#include "renderdoc.h"

#include "core/error/error_macros_v.h"
#include "core/string/ustring.h"
#include "core/string/print_string.h"
#include "core/os/os.h"
#include "thirdparty/renderdoc/renderdoc_app.h"

#include <process.h>
#include <shlobj.h>

inline void printf_verbose(String format, const Variant& p0) {
	print_verbose(vformat(format, p0));
}

namespace RenderDoc {

RENDERDOC_API_1_1_2 *GRDocAPI = NULL;

void LoadAPI() {
	const CharString subKey = "SOFTWARE\\Classes\\RenderDoc.RDCCapture.1\\DefaultIcon\\";
	const CharString dllName = "renderdoc.dll";

	// Query the installed renderdoc path via registery
	String rdocDirPath;
	{
		HKEY key;
		if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, subKey, 0, KEY_READ | KEY_WOW64_64KEY, &key) == ERROR_SUCCESS) {
			DWORD size;
			if (RegQueryValueEx(key, "", NULL, NULL, NULL, &size) == ERROR_SUCCESS) {
				CharString rdocPath;
				rdocPath.resize(size);
				if (RegQueryValueExA(key, "", NULL, NULL, (LPBYTE)rdocPath.get_data(), &size) == ERROR_SUCCESS) {
					rdocDirPath = String(rdocPath).get_base_dir();
				}
			}
		}
	}

	if (!!rdocDirPath.length()) {
		const String rdocPath = rdocDirPath.plus_file(String(dllName));
		if (HMODULE hModule = LoadLibraryW((LPCWSTR)rdocPath.utf16().get_data())) {
			printf_verbose("Load RenderDoc from %s.", rdocPath);
			pRENDERDOC_GetAPI RENDERDOC_GetAPI = (pRENDERDOC_GetAPI)GetProcAddress(hModule, "RENDERDOC_GetAPI");
			int ret = RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_1_2, (void **)&GRDocAPI);
			if (ret != 1) {
				WARN_PRINTF("\tFailed to get RenderDoc API (ret=%d).", ret);
			}
		} else {
			WARN_PRINT("Failed to load RenderDoc Library; make sure the renderdoc is properly installed.");
		}
	} else {
		WARN_PRINT("Failed to load RenderDoc; it is not installed on you system, or the installation is broken.");
	}
}

void SetupFilePath() {
	if (!GRDocAPI)
		return;

}

// Load the RenderDoc API, and do some setup.
void init() {
	LoadAPI();

	if (GRDocAPI) {
		// Setup file path
		String desiredPath = OS::get_singleton()->get_resource_dir() + "/renderdoc/example.rdc";
		GRDocAPI->SetCaptureFilePathTemplate(desiredPath.utf8().get_data());
		printf_verbose("Set renderdoc capture file path to %s", desiredPath);

		// Setup overlay
		GRDocAPI->MaskOverlayBits(RENDERDOC_OverlayBits::eRENDERDOC_Overlay_None, 0);
	}
}

void trigger_capture() {
	if (GRDocAPI) {
		GRDocAPI->TriggerCapture();

		// Open the UI
		if (!GRDocAPI->IsTargetControlConnected()) {
			GRDocAPI->LaunchReplayUI(1, nullptr);
		}
	}
}

} // namespace RenderDoc
