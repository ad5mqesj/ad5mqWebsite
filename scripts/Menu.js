function buildMenu(){
	document.write (
	'<div id="header-wrapper">\n\
		<div id="header">\n\
			<div id="logo">\n\
				<h1><a href="#">Ed Johnson</a></h1>\n\
				<p>AD5MQ</p>\n\
			</div>\n\
		</div>\n\
		<div id="menu-wrapper">\n\
			<div id="menu">\n\
				<ul id="nav" class="dropdown dropdown-horizontal">\n\
					<li class="first, current_page_item" ><a href="index.html">Home</a></li>\n\
					<li class="dir"><a href="#">About</a>\n\
						<ul>\n\
							<li class="first, submenu"><a href="resume3.pdf" target="_blank">My Resume</a></li>\n\
							<li class="submenu last"><a href="https://plus.google.com/113133089979540164531">Blog</a></li>\n\
						</ul>\n\
					</li>\n\
					<li class="dir"><a href="#">Amateur Radio</a>\n\
						<ul>\n\
							<li class="first, submenu"><a href="dr1x.html">Allstar on DR-1X</a></li>\n\
							<li class="first, submenu"><a href="preamp.html">Satelite Preamps</a></li>\n\
							<li class="submenu"><a href="Ams.html">AmSat Tracker</a></li>\n\
							<li class="submenu"><a href="AR.html">AD5MQ Analog Radio</a></li>\n\
							<li class="submenu"><a href="SBand.html">S Band Converter</a></li>\n\
							<li class="submenu"><a href="HFA.html">HF Ampifier</a></li>\n\
							<li class="submenu"><a href="ElectronicTheory.pdf">Amateur Extra Training Presentation</a></li>\n\
							<li class="last, submenu"><a href="HamHistory.pdf">Amateur Radio History Presentation</a></li>\n\
						</ul>\n\
					</li>\n\
					<li><a href="#">Other Projects</a>\n\
						<ul>\n\
							<li class="first, submenu"><a href="1948TVRestor.html">1948 Motorola VT71</a></li>\n\
						</ul>\n\
					</li>\n\
					<li><a href="work.html">Work</a></li>\n\
					<li><a href="pets.html">Pets</a></li>\n\
					<li class="last"><a href="recipes.php">Joy\'s Recipes</a></li>\n\
				</ul>\n\
			</div>\n\
		</div>\n\
		<!-- end #menu -->\n\
	</div>\n');
}

