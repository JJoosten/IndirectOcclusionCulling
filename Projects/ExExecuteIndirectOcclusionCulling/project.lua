project ("CFC.Project." .. ext_project_name)
	targetname  ("CFC.Project." .. ext_project_name)
	language    "C++"
	kind        "ConsoleApp"
	flags       { "No64BitChecks", "StaticRuntime" } -- disabled: "ExtraWarnings", 

	debugargs   { "" }
	debugdir    ( "Content" )
	
	ext_add_libraries()
	ext_add_cpp_files(".")
	ext_set_project_defaults()