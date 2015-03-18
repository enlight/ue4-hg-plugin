#ue4-hg-plugin (alpha)

A basic Mercurial source control plugin for Unreal Engine 4, available under the MIT license.
The master branch can be built against UE 4.7.

##Overview

The Unreal Editor has built-in visual diffing for Blueprint assets, this feature relies on a **Source Control Provider** plugin to interface with the repository your assets are stored in. Currently UE4 ships with built-in source control provider plugins for SVN and Perforce. This source control provider plugin allows the Unreal Editor to interact with a Mercurial repository, thus unlocking all the built-in asset diffing goodness for those of us who prefer to use Mercurial.

##Supported Operating Systems

The plugin should work on any OS the Unreal Editor can run on, however, I do all my development on Windows so if something isn't working right on another OS please let me know how to fix it :).

##Installation

###Prerequisites
You need Mercurial installed on your system, preferably a standalone version that doesn't rely on Python (though that may work too, I just haven't tried). On Windows I'd recommend installing [TortoiseHg](http://tortoisehg.bitbucket.org/), the plugin will then auto-detect the location of the relevant Mercurial executable. 

###Using the Binary UE4 Release
When using a binary release of UE4 you can extract a binary release of the plugin (if available) to either of the following locations:
>```<UE4 Installation Location>/4.7/Engine/Plugins/Editor/MercurialSourceControl/```
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

Note that your existing project must have a **Source** subdirectory with a couple of **.Target.cs** files in it, if it doesn't you may need to follow the steps in the **Building from Scratch** section below and then copy the built plugin into your project(s).

###Editor Configuration
Once you've got a binary version of the plugin (either by building or downloading) follow these steps:

1. Open **Window->Plugins** from the main menu of the Unreal Editor.
2. Navigate to the **Built-in/Editor/Source Control** or the **Installed/Editor/Source Control** sub-category, you should see the **Mercurial** plugin in the list.
3. Enable the plugin and restart the editor if requested to do so.
4. Click on the **circular red icon** in the top right corner of the Unreal Editor.
5. Select **Mercurial** from the drop-down.
6. If you installed [TortoiseHg](http://tortoisehg.bitbucket.org/) the **Mercurial Executable** should've been auto-detected, otherwise you need to specify the location of the Mercurial executable (hg.exe on Windows, may be just hg elsewhere).
7. Press the **Accept Settings** button to enable the Mercurial source control provider. 

##Building from Scratch

The following steps explain how to build the plugin as part of a new (mostly) empty project, in case you hit any issues while attempting to build it as part of an existing project, or as an engine plugin. 

1. Create a new **Basic Code (C++)** project from the UE4 editor, e.g. **MyProject**, close the editor.
2. Create a new subdirectory called **Plugins** in your root project directory, e.g. **MyProject/Plugins**.
3. Clone or extract the code for this plugin into a subdirectory within the **Plugins** directory, e.g. **MyProject/Plugins/MercurialSourceControl**.
4. On Windows right-click on the **.uproject** file (e.g. **MyProject.uproject**) in your root project directory and select **Generate Visual Studio Files**.
5. On Windows open the generated Visual Studio solution file (e.g. **MyProject.sln**) and build it.
6. Launch the UE4 editor and open the project you created in step 1.
7. Open **Window->Plugins** from the main menu, navigate to the **Installed/Editor** category and you should see the **Mercurial** plugin in the list.

If you'd like to build this plugin within an existing project just skip step 1, note that your existing project must have a **Source** subdirectory with a couple of **.Target.cs** files in it.