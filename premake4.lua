ext_root = _WORKING_DIR .. "/"

print("Parsing settings.. ");
dofile("settings.ini");
print("Done.");

newoption {
	trigger 	= "64bit",
	description = "Generate 64 bit projects."
}

newoption {
	trigger 	= "platform-windows",
	description = "Generate windows projects."
}

newoption {
	trigger     = "no-dx12",
	description = "Disable DirectX 12 renderer."
}

function ext_set_windows_sdk()
	local windows_sdk_path = os.outputof("\"" .. path.getabsolute(ext_root .. "Buildscripts/Tools/Windows/util.exe") .. "\" --windows-sdk")
	if(windows_sdk_path == nil or windows_sdk_path == "") then
		windows_sdk_path = "C:\\Program Files (x86)\\Windows Kits\\10\\"
	end

	local strlen = string.len(windows_sdk_path .. "Lib/") + 1
	local sdks = os.matchdirs(windows_sdk_path .. "Lib/*");
	local lastSDK = nil
	for i,sdk in ipairs(sdks) do
		lastSDK = string.sub(sdk, strlen)
	end
	if(lastSDK == nil) then
		print("No DirectX 12 support (Windows 10 SDK) detected. Please install a suitable windows 10 SDK (https://dev.windows.com/en-us/downloads/windows-10-sdk).")
		os.exit(0)
	end

	if(winDetected == false) then
		winDetected=true
		print("* Using windows target platform " .. lastSDK)
	end

	-- set windows sdk
	if(windowstargetplatform ~= nil) then
		windowstargetplatform (lastSDK)
	elseif(windowstargetplatformversion ~= nil) then
		windowstargetplatformversion (lastSDK)
	end
end

function ext_build_ident()
	local ident = ""
	if _OPTIONS["platform-windows"] then
		ident = ident .. "windows"
	end

	if _OPTIONS["64bit"] then
		ident = ident .. "-64"
	else
		ident = ident .. "-32"
	end
	return ident
end


function ext_add_cpp_files(folder)
	files
	{
		folder .. "/**.cpp",
		folder .. "/**.h",
		folder .. "/**.hpp",
		folder .. "/**.inl",
		folder .. "/**.c",
		folder .. "/**.hlsl"
	}
end

function ext_add_include_markers(folder)
	local markers = os.matchfiles(folder .. "/**include.marker");
	for i,marker in ipairs(markers) do
		local markerpath = string.sub(marker, 1, marker:len() - (string.len("/include.marker")))
		includedirs { markerpath }
	end
end

function ext_add_libraries()
	links { "CFC.Core.Dependencies"}
	links { "CFC.Core.Engine"}
	if _OPTIONS["platform-windows"] then
		links { "CFC.Platform.Windows" }
	end
end

function ext_set_project_defaults()
	if _OPTIONS["64bit"] then
		platforms { "x64" }
		defines "PLATFORM_64BIT"
	else
		defines "PLATFORM_32BIT"
	end

	-- directx 12 support
	if os.get() == "windows" and not _OPTIONS["no-dx12"] then
		ext_set_windows_sdk()
	end

	includedirs { ext_root .. "Source/Shared/engine", ext_root .. "Source/Shared" }

	configuration "Debug"
		targetdir   (ext_root .. "Build/" .. ext_build_ident() .. "/bin.Debug")
		defines     "_DEBUG"
		flags       { "Symbols" }

	configuration "Release"
		targetdir   (ext_root .. "Build/" .. ext_build_ident() .. "/bin.Release")
		defines     "NDEBUG"
		flags       { "Symbols", "OptimizeSpeed" }

	configuration "windows"
		links       { "ole32", "ws2_32" }
end

-- clean option
if _ACTION == "clean" then
	os.rmdir("bin")
	os.rmdir("build")
end

solution "CFC"
	if _OPTIONS["64bit"] then
		platforms {"x64" }
	end

	configurations { "Release", "Debug" }
	location ("Build/" .. ext_build_ident())
	--characterset "MBCS"  -- unicode is supported, no need for this

	-- global settings across the solution
	defines 		{ "" }

	configuration "vs*"
		defines		{"_CRT_SECURE_NO_DEPRECATE", "_CRT_SECURE_NO_WARNINGS", "_CRT_NONSTDC_NO_WARNINGS" }
		defines 	"PLATFORM_VISUALSTUDIO"
		buildoptions {"/MP"}

	configuration "windows"
		defines 	"PLATFORM_WINDOWS"
		defines		{ "WIN32" }

	configuration "Debug"
		defines		"PLATFORM_DEBUG"

	configuration "Release"
		defines		"PLATFORM_RELEASE"

	configuration { "macosx", "gmake" }
		buildoptions { "-mmacosx-version-min=10.4" }
		linkoptions  { "-mmacosx-version-min=10.4" }
		
	project "CFC.Core.Dependencies"
		targetname  "CFC.Core.Dependencies"
		language    "C++"
		kind        "StaticLib"
		flags       { "No64BitChecks", "StaticRuntime" } -- disabled: "ExtraWarnings",
		includedirs { "Dependencies/Include" }

		ext_add_cpp_files "Source/Shared/dependencies"
		ext_set_project_defaults()

	project "CFC.Core.Engine"
		targetname  "CFC.Core.Engine"
		language    "C++"
		kind        "StaticLib"
		flags       { "No64BitChecks", "StaticRuntime" } -- disabled: "ExtraWarnings",
		links 		{ "CFC.Core.Dependencies"}
		includedirs { "Dependencies/Include" }

		ext_add_cpp_files "Source/Shared/engine"
		ext_set_project_defaults()

if _OPTIONS["platform-windows"] then
	project "CFC.Platform.Windows"
		targetname  "CFC.Platform.Windows"
		language    "C++"
		kind        "StaticLib"
		flags       { "No64BitChecks", "StaticRuntime" } -- disabled: "ExtraWarnings",

		ext_add_cpp_files "Source/Windows"
		ext_set_project_defaults()
end

-- Execute modules
group "Modules"
local subprojects = os.matchfiles("Buildscripts/Modules/**.lua");
for i,subproject in ipairs(subprojects) do
	print("Including module: " .. subproject)
	dofile(subproject)
end


-- include sub projects
group "Projects"

local subprojects = os.matchfiles("Projects/**project.lua");
for i,subproject in ipairs(subprojects) do
	print("Including project: " .. subproject)
	ext_project_folder = string.sub(subproject, 1, subproject:len() - (string.len("/project.lua")))
	ext_project_name = string.sub(ext_project_folder, string.len("Projects/")+1)
	dofile(subproject)
end
