workspace "yoBoy"
   configurations { "Debug", "Release" }
   warnings "Extra"

project "yoboy"
   kind "ConsoleApp"

   language "C++"
   cppdialect "C++14"

   targetdir ("build/%{cfg.longname}")
   location ("build")

   files { "src/**.h", "src/**.cc" }

   filter "configurations:Debug"
      defines { "DEBUG" }
      symbols "On"

   filter "configurations:Release"
      defines { "NDEBUG" }
      optimize "On"
