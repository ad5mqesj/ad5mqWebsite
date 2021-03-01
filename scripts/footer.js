function buildFooter(){
	var currentTime = new Date();
	var year = currentTime.getFullYear();
	document.write ('<div id="footer">\n\
	<p>Copyright &copy; '+year+ ' ad5mq.info All rights reserved.</p>\n\
	</div>\n');
}

function buildJoyFooter(){
	var currentTime = new Date();
	var year = currentTime.getFullYear();
	document.write ('<div id="footer"><center>\n\
	<p><a href="../recipes.php">Back to Recipes</a><br /></p>\n\
	<p>Unless otherwise noted above, Copyright &copy; '+year+ ' Joy Johnson All rights reserved.</p>\n\
	</center></div>\n');
}


 