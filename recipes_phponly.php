<?php
$dir = 'recipes/'; //directory to pull from
$skip = array('.','..'); //a few directories to ignore

$dp = opendir($dir); //open a connection to the directory
$files = array();

if ($dp) {
    while ($file = readdir($dp)) {
        if (in_array($file, $skip)) continue;

//        if (is_dir("$dir$file")) {
//            $innerdp = opendir("$dir$file");

//            if ($innerdp) {
 //               while ($innerfile = readdir($innerdp)) {
//                    if (in_array($innerfile, $skip)) continue;

                    $arr = explode('.', $file);
                    if (strtolower($arr[count($arr) - 1]) == 'doc') {
                        $files[] = $file;
                    }
  //              }
  //          }
  //      }
    }
}


print ("<table>");
        foreach ($files as $file) 
	{ 
	$parts = explode ('.',$file);
        print ("<tr>");
            print ("<td>Recipe:  <a src='$file' />$parts[0] </td>");
        print ("</tr>");
        } 
print ("</table>");
?>