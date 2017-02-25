Indirect Occlusion Culling

Since VR and the trend in big virtual open worlds continues to grow, occlusion culling stays a hot topic of research. This project is an implementation of the occlusion culling technique created by Evgeny Makarov from NVIDIA described in the NVIDIA Advanced Scene Graph Rendering Pipeline presented in 2013 at GPU Technology Conference slides. http://on-demand.gputechconf.com/gtc/2013/presentations/S3032-Advanced-Scenegraph-Rendering-Pipeline.pdf

The demo loads and renders 16 sponza scenes and lets the user toggle full GPU driven occlusion culling to see the performance difference. Even on an intel iris 5100 it should run in realtime using execute indirect occlusion culling.

The CFC engine is an experimental framework that provides aquick and easy render pipeline prototyping possibilities. Currently only DX12 is supported.

Build:

run Buildscripts/generate_vs2015_win64.bat
open Build/windows-64/CFC.sln
compile and run the project in either debug or release
A huge thanks to the makers of the following libs, content and tools:

premake4 (genie fork)
microprofile
imgui
libcollision
stb
utf8dec
crytek sponza (I downsized the textures for ease of download)
