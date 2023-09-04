<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="content-type" content="text/html; charset=utf-8" />
<title>
<?php 
echo urldecode($_GET['title']); 
?>
</title>
<meta name="description" content="Joy's Recipes" />
<script type="text/javascript" src="scripts/footer.js" ></script>
</head>
<body>
<?php
	$file = urldecode($_GET['file']);

	$handle = fopen ($file,'r');
	$first = true;
	$beginpar = true;
	while (($line = fgets($handle)) !== false) 
	{
		if ($first)
		{
			print( "<p ><span style='font-size:24.0pt; font-weight:bold;'>");
			print ($line);
			print ("</span></p>");
			$first = false;
		}
		else
		{
			if ($beginpar)
			{
				print ("<p ><span style='font-size:14.0pt; font-weight:normal;'>");
				$beginpar = false;
			}
			if (ctype_space ($line) && $line[0] != 0x09)
			{
				print ("</span></p>");
				$beginpar = true;
			}
			else
			{
				$line = iconv("windows-1256", "utf-8//TRANSLIT//IGNORE", $line);
				print ($line);
				print ("<br />");
			}
		}

    }
	fclose ($handle);

?>
<script type="text/javascript">buildJoyFooter();</script>

</body>
</html>
