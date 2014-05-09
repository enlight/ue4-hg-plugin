---
layout: docs
title: Installation
permalink: /docs/installation/
---

###Prerequisites
You need Mercurial installed on your system, preferably a standalone version that doesn't rely on Python (though that may work too, I just haven't tried). On Windows I'd recommend installing [TortoiseHg](http://tortoisehg.bitbucket.org/), the plugin will then auto-detect the location of the relevant Mercurial executable. 

###Using the Binary UE4 Release
When using a binary release of UE4 you can extract a binary release of the plugin (if available) to either of the following locations:

>```<UE4 Installation Location>/4.0/Engine/Plugins/Editor/MercurialSourceControl/```
>
>```<Your Project Location>/Plugins/Editor/MercurialSourceControl/```

If you extract the plugin binaries into your project's plugins directory it will only be available for that project.

Alternatively, you can either clone or extract the plugin source to your project's plugins directory, which is covered next. Note that placing the plugin source into the engine plugins directory probably won't work because I don't think the binary UE4 release is configured to build engine plugins from source (but I haven't tried yet).

###Using the GitHub UE4 Release
If you'd like the plugin to be available for all your UE4 projects you need to clone or extract the plugin source to:

>```<UE4 Clone Location>/Engine/Plugins/Editor/MercurialSourceControl/```

Then follow these steps on Windows (adjust as needed on other OSes):

1. Run **GenerateProjectFiles.bat** in the UE4 source directory.
2. Open the generated **UE4.sln** Visual Studio solution file and build it.
3. Launch the Unreal Editor, open any project, and follow the instructions in the next section.

Alternatively, if you only want to make the plugin available for a single project clone or extract the plugin source to:

>```<Your Project Location>/Plugins/Editor/MercurialSourceControl/```

Then follow these steps on Windows (adjust as needed on other OSes):

1. Right-click on the **.uproject** file in Windows Explorer (e.g. **MyProject.uproject**) in your root project directory and select **Generate Visual Studio Files**.
2. Open the generated Visual Studio solution file (e.g. **MyProject.sln**) and build it.
3. Launch the Unreal Editor, open the project you've just built, and follow the instructions in the next section.

Note that your existing project must have a **Source** subdirectory with a couple of **.Target.cs** files in it, if it doesn't you may need to follow the steps in [Building from Scratch]({{ site.baseurl }}/docs/building/) and then copy the built plugin into your project(s).
