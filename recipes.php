<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<?php
$dir = 'recipes/'; //directory to pull from
$skip = array('.','..'); //a few directories to ignore
$dp = opendir($dir); 
$files = array();
$num_cols = 3;

if ($dp) 
{
    while ($file = readdir($dp)) 
	{
        if (in_array($file, $skip)) continue;
        $arr = explode('.', $file);
      	if (strtolower($arr[count($arr) - 1]) == 'txt') 
			{
            $files[] = $file;
			}
   	}
}
?>
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="content-type" content="text/html; charset=utf-8" />
<title>Ed Johnson - AD5MQ</title>
<meta name="description" content="Personal website" />
<meta name="keywords" content="Amateur Radio, Motorcycles, pets, Lions Club" />
<link href="http://fonts.googleapis.com/css?family=Oswald" rel="stylesheet" type="text/css" />
<link href='http://fonts.googleapis.com/css?family=Arvo' rel='stylesheet' type='text/css'>
<link href="css/style.css" rel="stylesheet" type="text/css" media="screen" />
<link href="css/dropdown/dropdown.css" media="screen" rel="stylesheet" type="text/css" />
<link href="css/dropdown/themes/flickr.com/default.css" media="screen" rel="stylesheet" type="text/css" />
<script type="text/javascript" src="scripts/footer.js" ></script>
<script type="text/javascript" src="scripts/Menu.js" ></script>
</head>
<body>
<div id="wrapper">
	<script type="text/javascript">buildMenu();</script>
	<!-- end #header -->
	<div id="page">
		<div id="page-bgtop">
			<div id="page-bgbtm">
				<div id="page-content">
					<div id="content">
						<div class="post">
						<?php
						print ("<table>");
						asort ($files);
					    $col = 0;
						foreach ($files as $file) 
						   { 
						   $parts = explode ('.',$file);
						   if ($col % $num_cols == 0)
								print ("<tr>");
                           $file_path = urlencode ($dir . $file);
						   $title=urlencode ($parts[0]);
						   $href = "recipe.php?title=$title&file=$file_path";
						   print ("<td><a href='$href' />$parts[0] &nbsp;&nbsp;</td>");
                           $col++;
                           if ($col == $num_cols)
                               $col = 0;
						   if ($col % $num_cols == 0)
						      print ("</tr>");
						   } 
						print ("</table>");
						?>
						</div>
						<div style="clear: both;">&nbsp;</div>  
					</div>
					<!-- end #content -->
				</div>
				<div style="clear: both;">&nbsp;</div> 
			</div>
		</div>
	</div>
	<!-- end #page -->
</div>
<script type="text/javascript">buildFooter();</script>
<!-- end #footer -->
</body>
</html>
