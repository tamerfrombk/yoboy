workspace "yoBoy"
   configurations { "Debug", "Release" }

project "yoboy"
   kind "ConsoleApp"
   language "C++"
   targetdir ("build/%{cfg.longname}")
   location ("build")

   files { "src/**.h", "src/**.cc" }

   filter "configurations:Debug"
      defines { "DEBUG" }
      symbols "On"

   filter "configurations:Release"
      defines { "NDEBUG" }
      optimize "On"