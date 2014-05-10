---
layout: docs
title: Status Icons
permalink: /docs/status-icons/
---

When the Mercurial plugin is enabled in UnrealEd the following overlay icons will be displayed on asset icons in the Content Browser (provided that Mercurial hasn't been instructed to ignore those assets via .hgignore).

{% assign icons_path = site.github.repository_url | append:"/raw/master/Art" %}

Icon | Description
|-
![AddedStatusIcon]({{ icons_path }}/AddedStatusIcon.png) | The asset has been marked for adding to the repository, but hasn't been checked-in yet.
![CleanStatusIcon]({{ icons_path }}/CleanStatusIcon.png) | No changes were saved to disk since the last checkout.
![ModifiedStatusIcon]({{ icons_path }}/ModifiedStatusIcon.png) | Changes were saved to disk since the last checkout.
![NotTrackedStatusIcon]({{ icons_path }}/NotTrackedStatusIcon.png) | Changes to the asset are not tracked by the repository.
![MissingStatusIcon]({{ icons_path }}/MissingStatusIcon.png) | The asset has been deleted from disk without updating the repository.
![RemovedStatusIcon]({{ icons_path }}/RemovedStatusIcon.png) | The asset has been marked for removal from the repository.

Generally the ![MissingStatusIcon]({{ site.baseurl }}/raw/master/Art/MissingStatusIcon.png) and ![RemovedStatusIcon]({{ icons_path }}/RemovedStatusIcon.png) icons shouldn't show up in UnrealEd, unless there's a glitch.