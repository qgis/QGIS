<?php

session_start();
$_SESSION[userid] = session_id();
$_SESSION[ipadd] = $REMOTE_ADDR;

$_SESSION['resName'] = $fldResName;

$thispage =$SERVER['PHP_SELF'];


include("MDRcode.php");

$form_str= <<< EOFORMSTR


<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<!-- saved from url=(0039)http://www.bdworld.org/mockup/index.htm -->
<HTML><HEAD><TITLE>BDWorld Metadata Database</TITLE>
<META http-equiv=Content-Type content="text/html; charset=iso-8859-1"><LINK
href="BDWMDR_template_css.css" type=text/css
rel=stylesheet>
<META content="MSHTML 6.00.2900.2523" name=GENERATOR></HEAD>
<BODY text=#000000 bgColor=#ffffff>

<H1 align=center>BDWorld Metadata</H1>
<H2 align=center>Reource Information Entry Form</H2>

<TABLE align=center border=0>
  <TBODY>
  <TR>
    <TD>
      <form action="$thispage" method="POST">


      <!--Nav Bar-->
      <TABLE width=490 bgColor=gray border=0>
        <TBODY>
        <TR>
        <TD align=middle>


<INPUT class=button id=first type=submit value="<<" name=resourceFrmBtn>
<INPUT class=button id=previous type=submit value="<" name=resourceFrmBtn>
<INPUT class=button id=next type=submit value=">" name=resourceFrmBtn>
<INPUT class=button id=last type=submit value=">>" name=resourceFrmBtn>
<INPUT class=button id=save type=submit value=Save name=resourceFrmBtn>
<INPUT class=button id=insert type=submit value=New name=resourceFrmBtn>
<INPUT class=button id=delete type=submit value=Delete name=resourceFrmBtn>
<INPUT class=button id=xmlall type=submit value="View All Resources" name=resourceFrmBtn>


      </TD></TR></TBODY></TABLE><BR>
      <DIV class=small>Resource Details:</DIV>
      <HR width=490>
      <!-- Resource Form-->
      <TABLE width=490 border=0>
        <TBODY>
        <TR vAlign=top>
          <TD class=fieldLabel>Unique ID:</TD>
          <TD>&nbsp;<INPUT size=40 value='$fldResID' name=fldResID></TD></TR>
        <TR vAlign=top>
          <TD class=fieldLabel>Name:</TD>
          <TD><INPUT size=40 value='$fldResName' name=fldResName></TD></TR>
        <TR vAlign=top>
          <TD class=fieldLabel>Description:</TD>
          <TD><TEXTAREA name=fldResDescription rows=5 cols=40>$fldResDescription</TEXTAREA>
          </TD></TR>
        <TR vAlign=top>
          <TD class=fieldLabel>End point:</TD>
          <TD>
			<INPUT class=wideField size=40
            value='$fldResHost'
            name=fldResHost> </TD></TR>
        <TR vAlign=top>
          <TD class=fieldLabel>Handle:</TD>
          <TD>
			<INPUT class=wideField size=40
            value='$fldResHandle'
            name=fldResHandle></TD></TR></TBODY></TABLE>&nbsp;<p><BR>
      </p>
      <DIV class=small>Operations for this resource:</DIV>
      <HR width=490>
      <!-- Operations Form-->
      <TABLE width=490 bgColor=lightgrey border=1>
        <TBODY>
        <TR>
          <TD>
            <TABLE width=490 border=0>
              <TBODY>
              <TR>
                <TD colSpan=2>
                  <TABLE align=center border=0>
                    <TBODY>
                    <TR>
                      <TD align=middle>
                        <DIV class=small>[Operation 1 of
							<INPUT class=small value='$fldOpCount' name=fldOpCount size="3">]</DIV></TD></TR>
                    <TR>
                      <TD align=middle>

<INPUT class=button id=first type=submit value="<<" name=optionFrmBtn>
<INPUT class=button id=previous type=submit value="<" name=optionFrmBtn>
<INPUT class=button id=next type=submit value=">" name=optionFrmBtn>
<INPUT class=button id=last type=submit value=">>" name=optionFrmBtn>
<INPUT class=button id=save type=submit value=Save name=optionFrmBtn>
<INPUT class=button id=insert type=submit value=New name=optionFrmBtn>
<INPUT class=button id=delete type=submit value=Delete name=optionFrmBtn>&nbsp;

               </TD></TR></TBODY></TABLE></TD></TR>
              <TR vAlign=top>
                <TD class=fieldLabel>Unique ID:</TD>
                <TD>
				<INPUT class=wideField value='$fldOpID' name=fldOpID size="40"></TD></TR>
              <TR vAlign=top>
                <TD class=fieldLabel>Name:</TD>
                <TD>
				<INPUT class=wideField value='$fldOpName' name=fldOpName size="40">
                </TD></TR>
              <TR vAlign=top>
                <TD class=fieldLabel>Description:</TD>
                <TD><TEXTAREA name=fldOpDescription rows=5 cols=40>$fldOpDescription</TEXTAREA>
                </TD></TR>
              <TR vAlign=top>
              	<TD class=fieldLabel>Inputs and Outputs</TD>
              	<TD>(Not yet available)</TD>
              </TR>
</TBODY></TABLE>
</TABLE>

</form>

<p>
<HR width=490>
MS Word document about limitations and usage of this web GUI: <a target="_blank" href="BDWorldMDR.doc">BDWorldMDR.doc</a>
</p>

</BODY></HTML>


EOFORMSTR;
echo $form_str;




?>