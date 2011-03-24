<?
$srs = null;
$i = 0;
$j = 1;
$srs_sync = 0;
$srs_nosync = 0;

$f = fopen('epsg',r);
while (!feof($f)) {
 $line = trim(fgets($f));
 if ($line[0] != '#' && $line != '') {
  $chunks = explode('>',$line);
  $srs[$i]['id'] = substr($chunks[0],1);
  $srs[$i]['param'] = trim(substr($chunks[1],0,-1));
  $srs[$i]['linenb'] = $j; //debugging info
  $i++;
 }
 $j++;
}
fclose($f);

$f = fopen('update.sql',w);
$sqlread = new SQLite3('srs.db');
$sqlwrite = new SQLite3('srs-sync.db');

$r = $sqlread->query("SELECT srid, parameters FROM tbl_srs");
while ($row = $r->fetchArray(SQLITE3_ASSOC)) {
 foreach($srs as $s) {
  if ($row['srid'] == $s['id']) {
   if ($row['parameters'] != $s['param']) {
    $sqlwrite->query("UPDATE tbl_srs SET parameters='".$s['param']."' WHERE srid='".$s['id']."'");
    fwrite($f,"UPDATE tbl_srs SET parameters='".$s['param']."' WHERE srid='".$s['id']."'".chr(13).chr(10));
    $srs_sync++;
   } else { $srs_nosync++; }
   break;
  }
 }
}

$sqlread->close();
$sqlwrite->close();
fclose($f);

echo $srs_sync.' -- '.$srs_nosync;
?>