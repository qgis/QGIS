<?php

	// clears entry boxes for operations
	// can be used in place of NEW method - does the same thing
	function clearOperationFields(&$OpID, &$OpName, &$OpDescription)
	{
		$OpID = "";
		$OpName = "";
		$OpDescription = "";
	}


	// ---------------------------------------------------
	// NEW OPERATION METHOD
	// ---------------------------------------------------
	// clear entry boxes for new operation

	function newOperation(&$OpID, &$OpName, &$OpDescription)
	{
		$OpID = "";
		$OpName = "";
		$OpDescription = "";

	}



	// ---------------------------------------------------
	// DISPLAY FIRST OPERATION METHOD
	// ---------------------------------------------------
	// get operations related to current resource id
	function firstOperation(&$ResID, &$OpID, &$OpName, &$OpDescription)
	{
		$rowquery = "SELECT * FROM operations WHERE resourceid = '$ResID' ORDER BY id ASC";
		$result = mysql_query($rowquery);

		if ($result)
		{
			//operations exist for this resource so can be displayed
			$queryrow = mysql_fetch_row($result);

			$OpID = $queryrow[0];
			$OpName = $queryrow[1];
			$OpDescription = $queryrow[2];
		} else
		{
			//there are no operations related to this resource
			$OpID = "";
			$OpName = "";
			$OpDescription = "";
		}
	}



	// ---------------------------------------------------
	// LAST OPERATION OPTION SELECTED
	// ---------------------------------------------------
	function lastOperation(&$ResID, &$OpID, &$OpName, &$OpDescription)
	{
			$maxIDQuery = "SELECT MAX(id) FROM operations WHERE resourceid = '$ResID';";
			$maxIDResult = mysql_query($maxIDQuery) or die ("Invalid query");

			if ($maxIDResult)
			{
				$lastID = mysql_fetch_row($maxIDResult);

				$query = "SELECT * FROM operations WHERE id = $lastID[0]";
				$result = mysql_query($query) or die ("Invalid query");

				$queryrow = mysql_fetch_row($result);

				$OpID = $queryrow[0];
				$OpName = $queryrow[1];
				$OpDescription = $queryrow[2];

			} else
			{
				//there are no operations related to this resource
				$OpID = "";
				$OpName = "";
				$OpDescription = "";
			}
	}


	// ---------------------------------------------------
	// DISPLAY NEXT OPERATION METHOD
	// ---------------------------------------------------
	function nextOperation(&$ResID, &$OpID, &$OpName, &$OpDescription)
	{
		$rowquery = "SELECT * FROM operations WHERE id > '$OpID' AND resourceid = '$ResID' ORDER BY id ASC";
		$result = mysql_query($rowquery);

		if ($result)
		{
			//there are more resources that can be displayed
			$queryrow = mysql_fetch_row($result);

			$OpID = $queryrow[0];
			$OpName = $queryrow[1];
			$OpDescription = $queryrow[2];
		} else
		{
		//at last operation, so do not change
			$OpID = $OpID;
			$OpName = $OpName;
			$OpDescription = $OpDescription;
		}
	}

	// ---------------------------------------------------
	// DISPLAY PREVIOUS OPERATION METHOD
	// ---------------------------------------------------
	function previousOperation(&$ResID, &$OpID, &$OpName, &$OpDescription)
	{
		$rowquery = "SELECT * FROM operations WHERE id < '$OpID' AND resourceid = '$ResID' ORDER BY id DESC";
		$result = mysql_query($rowquery);

		if ($result)
		{
			//there are more resources that can be displayed
			$queryrow = mysql_fetch_row($result);

			$OpID = $queryrow[0];
			$OpName = $queryrow[1];
			$OpDescription = $queryrow[2];
		} else
		{
		//at last operation, so do not change
			$OpID = $OpID;
			$OpName = $OpName;
			$OpDescription = $OpDescription;
		}
	}


	// ---------------------------------------------------
	// SAVE OPTION SELECTED
	// ---------------------------------------------------
	//add new operation to database
	//currently adds a new record, so if an old record is edited it is saved again as a new record.
	//this is therefore a poor implementation, so must be improved as soon as possible
	function saveOperation(&$ResID, &$OpID, &$OpName, &$OpDescription)
	{
		//does this record already exist?
		//if it exists $fldResID will hold the current ID
		if($OpID == "")
		{
			//insert new row into resource table
    		$query = "INSERT INTO operations(name, description, resourceid) VALUES ('$OpName', '$OpDescription', '$ResID');";
			$result = mysql_query($query) or die ("Invalid query");
			echo "<br>New operation: $OpName added<br>";

		} else {
			//update current record
			$query = "UPDATE operations SET name ='$OpName', description='$OpDescription', resourceid = '$ResID' WHERE id='$OpID';";
			$result = mysql_query($query) or die ("Invalid query");
			echo "<br>Update operation: $OpName updated<br>";
		}

		//get operation ID for new/updated record

   		$query = "SELECT * FROM operations WHERE name = '$OpName';";
		$result = mysql_query($query) or die ("Invalid query");
		$queryrow = mysql_fetch_row($result);
		$OpID = $queryrow[0];
	}

	// ---------------------------------------------------
	// DELETE OPTION SELECTED
	// ---------------------------------------------------
	function deleteOperation(&$ResID, &$OpID, &$OpName, &$OpDescription)
	{
		$query = "DELETE FROM operations WHERE id = '$OpID' AND resourceid = '$ResID';";
		$result = mysql_query($query) or die ("Invalid query");
		echo "<br>Delete record: $OpID deleted<br>";
	}


	// GET NUM OF OPERATIONS
	//  ***************** not complete ****************
	function numOfOperations(&$ResID, &$OpCount)
	{
		$query = "SELECT * FROM operations WHERE resourceid = '$ResID' ORDER BY id DESC";
		$result = mysql_query($query);
	}




?>