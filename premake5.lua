location("build")

function mgp_files(shared)
	includedirs("include")
	files("src/common/*.c")
	files("src/stb/*.c")
	filter({"system:linux"})
		files("src/linux/**.c")
		includedirs("src/linux/libevdev")
	filter({"system:windows"})
		files("src/winmm/**.c")
	if shared then
		links("winmm")
	end
	filter({})
end

project("minigamepad-static")
	kind("StaticLib")
	targetdir("build")
	targetname("minigamepad")
	mgp_files(false)

project("minigamepad-dynamic")
	kind("SharedLib")
	targetdir("build")
	targetname("minigamepad")
	mgp_files(true)
