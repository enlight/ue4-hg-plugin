ue4-hg-plugin (WIP)
=============

A basic MIT licensed Mercurial source control plugin for Unreal Engine 4.

Overview
----------
The UE4 editor has built-in visual diffing for Blueprint assets, but that currently only works if you have it hooked up to an SVN or Perforce repository.  I'd like to store my Blueprints along with my code in a Mercurial repository, so the plan is to write a bare-bones source control provider plugin for Mercurial that will allow me to diff assets within the editor.

Building & Testing
--------------------
As far as I could figure out in order to build a plugin it must be within a UE4 project, so here's what you need to do:

1. Create a new **Basic Code (C++)** project from the UE4 editor, e.g. **MyProject**, close the editor.
2. Create a new subdirectory called **Plugins** in your root project directory, e.g. **MyProject/Plugins**.
3. Checkout the code for this plugin into a subdirectory within the **Plugins** directory, e.g. **MyProject/Plugins/MercurialSourceControl**.
4. On Windows right-click on the **.uproject** file (e.g. **MyProject.uproject**) in your root project directory and select **Generate Visual Studio Files**.
5. On Windows open the generated Visual Studio solution file (e.g. **MyProject.sln**) and build it.
6. Launch the UE4 editor and open the project you created in step 1.
7. Open **Window->Plugins** from the main menu, navigate to the **Installed/Editor** category and you should see the **Mercurial** plugin in the list.

If you'd like to build this plugin within an existing project just skip step 1, note that your existing project must have a **Source** subdirectory with a couple of **.Target.cs** files in it.