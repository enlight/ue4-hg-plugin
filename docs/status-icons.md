---
layout: docs
title: Status Icons
permalink: /docs/status-icons/
---

When the Mercurial plugin is enabled in UnrealEd the following overlay icons will be displayed on asset icons in the Content Browser (provided that Mercurial hasn't been instructed to ignore those assets via .hgignore).

{% assign icons_path = site.github.repository_url | append:"/raw/master/Art" %}

<div class="row">
	<div class="col-md-4">
		<div class="thumbnail">
			<img src="{{ icons_path }}/AddedStatusIcon.png" alt="Added Status Icon">
			<div class="caption">The asset has been marked for adding to the repository, but hasn't been checked-in yet.</div>
		</div>
	</div>
	<div class="col-md-4">
		<div class="thumbnail">
			<img src="{{ icons_path }}/CleanStatusIcon.png" alt="Clean Status Icon">
			<div class="caption">No changes were saved to disk since the last checkout.</div>
		</div>
	</div>
	<div class="col-md-4">
		<div class="thumbnail">
			<img src="{{ icons_path }}/ModifiedStatusIcon.png" alt="Modified Status Icon"> 
			<div class="caption">Changes were saved to disk since the last checkout.</div>
		</div>
	</div>
</div>
<div class="row">
	<div class="col-md-4">
		<div class="thumbnail">
			<img src="{{ icons_path }}/NotTrackedStatusIcon.png" alt="Not Tracked Status Icon"> 
			<div class="caption">Changes to the asset are not tracked by the repository.</div>
		</div>
	</div>
	<div class="col-md-4">
		<div class="thumbnail">
			<img src="{{ icons_path }}/MissingStatusIcon.png" alt="Missing Status Icon">
			<div class="caption">The asset has been deleted from disk without updating the repository.</div>
		</div>
	</div>
	<div class="col-md-4">
		<div class="thumbnail">
			<img src="{{ icons_path }}/RemovedStatusIcon.png" alt="Removed Status Icon">
			<div class="caption">The asset has been marked for removal from the repository.</div>
		</div>
	</div>
</div>

Generally the ![MissingStatusIcon]({{ icons_path }}/MissingStatusIcon.png) and the ![RemovedStatusIcon]({{ icons_path }}/RemovedStatusIcon.png) icons shouldn't show up in UnrealEd, unless there's a glitch.