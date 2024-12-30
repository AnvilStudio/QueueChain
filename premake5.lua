workspace "QueueChain"
    configurations {"Debug", "Release"}    

project "QueueChain"
    language "C++"
    cppdialect "C++20"
    kind "ConsoleApp"

    targetdir "build/bin/"
    objdir "build/obj/"

    files
    {
        "**.cpp",
        "**.hpp"
    }

