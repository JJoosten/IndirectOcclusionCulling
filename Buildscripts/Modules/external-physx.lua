if cfc_module_disable_physx then
	return
end

cfc_module_physx = true
project "CFC.Module.PhysX"
	targetname  "CFC.Module.PhysX"
	language    "C++"
	kind        "StaticLib"
	flags       { "No64BitChecks", "StaticRuntime" } -- disabled: "ExtraWarnings",
	includedirs { ext_root .. "Dependencies/Include" }
	defines { "DELAYIMP_INSECURE_WRITABLE_HOOKS", "PX_PHYSX_STATIC_LIB", "PX_PHYSX_CORE_STATIC_LIB", "PX_FOUNDATION_NO_EXPORTS", "PX_PHYSX_COMMON_EXPORTS"}
	
	ext_add_cpp_files (ext_root .. "Source/Shared/external/physx")
	ext_set_project_defaults()
	ext_add_include_markers(ext_root .. "Source/Shared/external/physx")
	