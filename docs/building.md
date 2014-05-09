---
layout: docs
title: Building
permalink: /docs/building/
---

###From Scratch

The following steps explain how to build the plugin as part of a new (mostly) empty project, in case you hit any issues while attempting to build it as part of an existing project, or as an engine plugin. 

1. Create a new **Basic Code (C++)** project from the UE4 editor, e.g. **MyProject**, close the editor.
2. Create a new subdirectory called **Plugins** in your root project directory, e.g. **MyProject/Plugins**.
3. Clone or extract the code for this plugin into a subdirectory within the **Plugins** directory, e.g. **MyProject/Plugins/MercurialSourceControl**.
4. On Windows right-click on the **.uproject** file (e.g. **MyProject.uproject**) in your root project directory and select **Generate Visual Studio Files**.
5. On Windows open the generated Visual Studio solution file (e.g. **MyProject.sln**) and build it.
6. Launch the UE4 editor and open the project you created in step 1.
7. Open **Window->Plugins** from the main menu, navigate to the **Installed/Editor** category and you should see the **Mercurial** plugin in the list.

If you'd like to build this plugin within an existing project just skip step 1, note that your existing project must have a **Source** subdirectory with a couple of **.Target.cs** files in it.
