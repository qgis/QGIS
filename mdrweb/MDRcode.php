<?php
	include("dbdetails.php");    //holds database information
	include("resources.php");	 //methods for resource data
	include("operations.php");	 //methods for operation data

	//show selected option on screen - for debugging
	$selectedResourceOption = $_POST[resourceFrmBtn];
	$selectedOperationOption = $_POST[optionFrmBtn];
	//echo "<p>Option selected: <b>$selectedResourceOption $selectedOperationOption</b></p>";


	getDatabaseSetting($loc, $user, $pss, $db);
	// Connect to mysql database
	$link = mysql_connect($loc, $user, $pss) or die ("Could not Connect to database!");
	mysql_select_db($db) or die ("Error: Could not connect to database!");


	// Initialise variables

	$ResID = trim($_POST[fldResID]);
	$ResName = trim($_POST[fldResName]);
	$ResDescription = trim($_POST[fldResDescription]);
	$ResHost = trim($_POST[fldResHost]);
	$ResHandle = trim($_POST[fldResHandle]);

	$OpCount = trim($_POST[fldOpCount]);
	$OpID = trim($_POST[fldOpID]);
	$OpName = trim($_POST[fldOpName]);
	$OpDescription = trim($_POST[fldOpDescription]);
	$OpInputs = $_POST[operationInputs];



	switch($selectedResourceOption)
	{
		case "Save":
			saveResource($ResID, $ResName, $ResDescription, $ResHost, $ResHandle);
			break;
		case "New":
			clearResourceFields($ResID, $ResName, $ResDescription, $ResHost, $ResHandle);
			break;
		case "Delete":
			deleteResource(&$ResID);
			clearResourceFields($ResID, $ResName, $ResDescription, $ResHost, $ResHandle);
			break;
		case ">>":
			lastResource($ResID, $ResName, $ResDescription, $ResHost, $ResHandle);
			firstOperation($ResID, $OpID, $OpName, $OpDescription);
			break;
		case ">":
			nextResource($ResID, $ResName, $ResDescription, $ResHost, $ResHandle);
			firstOperation($ResID, $OpID, $OpName, $OpDescription);
			break;
		case "<":
			previousResource($ResID, $ResName, $ResDescription, $ResHost, $ResHandle);
			firstOperation($ResID, $OpID, $OpName, $OpDescription);
			break;
		case "<<":
			firstResource($ResID, $ResName, $ResDescription, $ResHost, $ResHandle);
			firstOperation($ResID, $OpID, $OpName, $OpDescription);
			break;
		case "View All Resources":
			viewAllResources();
			clearResourceFields($ResID, $ResName, $ResDescription, $ResHost, $ResHandle);
			break;
	}

	switch($selectedOperationOption)
	{
		case "Save":
			saveOperation($ResID, $OpID, $OpName, $OpDescription);
			break;
		case "New":
			clearOperationFields($OpID, $OpName, $OpDescription);
			break;
		case "Delete":
			deleteOperation($ResID, $OpID, $OpName, $OpDescription);
			clearOperationFields($OpID, $OpName, $OpDescription);
			break;
		case ">>":
			lastOperation($ResID, $OpID, $OpName, $OpDescription);
			break;
		case ">":
			nextOperation($ResID, $OpID, $OpName, $OpDescription);
			break;
		case "<":
			previousOperation($ResID, $OpID, $OpName, $OpDescription);
			break;
		case "<<":
			firstOperation($ResID, $OpID, $OpName, $OpDescription );
			break;
	}


	// UPDATE DISPLAY
	$fldResID= $ResID;
	$fldResName = $ResName;
	$fldResDescription = $ResDescription;
	$fldResHost = $ResHost;
	$fldResHandle = $ResHandle;

	$fldOpCount = $OpCount;
	$fldOpID = $OpID;
	$fldOpName = $OpName;
	$fldOpDescription = $OpDescription;


?>
