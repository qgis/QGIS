<?php
	// ---------------------------------------------------
	// LAST RECORD OPTION SELECTED
	// ---------------------------------------------------
	//if ($selectedResourceOption == ">>")
	function lastResource(&$ResID, &$ResName, &$ResDescription, &$ResHost, &$ResHandle)
	{

			$maxIDQuery = "SELECT MAX(id) FROM resources;";
			$maxIDResult = mysql_query($maxIDQuery) or die ("Invalid query");
			$lastID = mysql_fetch_row($maxIDResult);

			$query = "SELECT * FROM resources WHERE id = $lastID[0]";
			$result = mysql_query($query) or die ("Invalid query");

			$queryrow = mysql_fetch_row($result);

			$ResID = $queryrow[0];
			$ResName = $queryrow[1];
			$ResDescription = $queryrow[2];
			$ResHost = $queryrow[3];
			$ResHandle = $queryrow[4];
	}



	// ---------------------------------------------------
	// NEXT RECORD OPTION SELECTED
	// ---------------------------------------------------
	//if ($selectedResourceOption == ">")
	function nextResource(&$ResID, &$ResName, &$ResDescription, &$ResHost, &$ResHandle)
	{
		$rowquery = "SELECT * FROM resources WHERE id > $_POST[fldResID] ORDER BY id ASC";
		$result = mysql_query($rowquery);

		if ($result)
		{
			//there are more resources that can be displayed
			$queryrow = mysql_fetch_row($result);

			$ResID = $queryrow[0];
			$ResName = $queryrow[1];
			$ResDescription = $queryrow[2];
			$ResHost = $queryrow[3];
			$ResHandle = $queryrow[4];
		} else
		{
			$ResID = $_POST[fldResName];
			$ResName = $_POST[$fldResName];
			$ResDescription = $_POST[$fldResDescription];
			$ResHost = $_POST[$fldResHost];
			$ResHandle = $_POST[$fldResHandle];
		}
	}


	// ---------------------------------------------------
	// PREVIOUS RECORD OPTION SELECTED
	// ---------------------------------------------------
	//if ($selectedResourceOption == "<")
	//select previous entry
	function previousResource(&$ResID, &$ResName, &$ResDescription, &$ResHost, &$ResHandle)
	{
		$rowquery = "SELECT * FROM resources WHERE id < $_POST[fldResID] ORDER BY id DESC";
		$result = mysql_query($rowquery);

		if ($result)
		{
			//there are more resources that can be displayed
			$queryrow = mysql_fetch_row($result);

			$ResID = $queryrow[0];
			$ResName = $queryrow[1];
			$ResDescription = $queryrow[2];
			$ResHost = $queryrow[3];
			$ResHandle = $queryrow[4];
		} else
		{
			$ResID = $_POST[fldResName];
			$ResName = $_POST[$fldResName];
			$ResDescription = $_POST[$fldResDescription];
			$ResHost = $_POST[$fldResHost];
			$ResHandle = $_POST[$fldResHandle];
		}
	}



	// ---------------------------------------------------
	// VIEW ALL RESOURCES
	// ---------------------------------------------------
	//if ($selectedResourceOption == "XML for All Ops")
	//this feature is not currently available. Will do this in future version
	//as a placeholder for this feature, this method prints out all current resources

	function viewAllResources()
	{
		echo "<b>List of all current resources:</b><p>";

		$rowquery = "SELECT * FROM resources";
		$result = mysql_query($rowquery) or die ("Invalid query");

			//loop through all returned records and print out the entry
		while ($queryrow = mysql_fetch_row($result))
		{
			print "Entry ID: $queryrow[0]<br>";
			print "Resource name: $queryrow[1]<br>";
			print "Description: $queryrow[2]<br>";
			print "End point: $queryrow[3]<br>";
			print "Handle: $queryrow[4]<br>";

			print "<p>";
		}
	}


	// ---------------------------------------------------
	// FIRST RECORD OPTION SELECTED
	// ---------------------------------------------------
	//if ($selectedResourceOption == "<<")
	//select first entry
	function firstResource(&$ResID, &$ResName, &$ResDescription, &$ResHost, &$ResHandle)
	{
		//$rowquery = "SELECT * FROM resources ORDER BY id ASC";

		$minIDQuery = "SELECT MIN(id) FROM resources;";
		$minIDResult = mysql_query($minIDQuery) or die ("Invalid query");
		$firstID = mysql_fetch_row($minIDResult);

		$query = "SELECT * FROM resources WHERE id = $firstID[0]";
		$result = mysql_query($query) or die ("Invalid query");

		$queryrow = mysql_fetch_row($result);

		$ResID = $queryrow[0];
		$ResName = $queryrow[1];
		$ResDescription = $queryrow[2];
		$ResHost = $queryrow[3];
		$ResHandle = $queryrow[4];
	}

	// ---------------------------------------------------
	// SAVE OPTION SELECTED
	// ---------------------------------------------------
	//if ($selectedResourceOption == "Save")
	//add new resource to database
	//currently adds a new record, so if an old record is edited it is saved again as a new record.
	//this is therefore a poor implementation, so must be improved as soon as possible
	function saveResource(&$ResID, &$ResName, &$ResDescription, &$ResHost, &$ResHandle)
	{

		//does this record already exist?
		//if it exists $fldResID will hold the current ID
		if($ResID == "")
		{
			//insert new row into resource table
    		$query = "INSERT INTO resources(name, description,host, handle) VALUES ('$ResName', '$ResDescription', '$ResHost', '$ResHandle');";
			$result = mysql_query($query) or die ("Invalid query");
			echo "<br>New record: $resourceName added<br>";

		} else {
			//update current record
			$query = "UPDATE resources SET name ='$ResName', description='$ResDescription', host='$ResHost', handle = '$ResHandle' WHERE id='$ResID';";
			$result = mysql_query($query) or die ("Invalid query");
			echo "<br>Update record: $resourceName updated<br>";
		}

		//get resource ID for new/updated record

   		$query = "SELECT * FROM resources WHERE name = '$ResName';";
		$result = mysql_query($query) or die ("Invalid query");
		$queryrow = mysql_fetch_row($result);
		$ResID = $queryrow[0];
	}

	// ---------------------------------------------------
	// NEW OPTION SELECTED
	// ---------------------------------------------------
	//if ($selectedResourceOption == "New")
	//empty fields for new entry

	function clearResourceFields(&$ResID, &$ResName, &$ResDescription, &$ResHost, &$ResHandle)
	{
		$ResID="";
		$ResName = "";
		$ResDescription = "";
		$ResHost = "";
		$ResHandle = "";
	}

	// ---------------------------------------------------
	// DELETE OPTION SELECTED
	// ---------------------------------------------------
	//if ($selectedResourceOption == "Delete")
	//Delete currently displayed entry

	function deleteResource(&$ResID)
	{
		$query = "DELETE FROM resources WHERE id = '$ResID';";
		$result = mysql_query($query) or die ("Invalid query");
		echo "<br>Delete record: $ResID deleted<br>";
	}



?>
