<?xml version="1.0"?>
<!--
This stylesheet generates 'tabs' at the top left of the screen.
See the imported tab2menu.xsl for details.
-->

<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

  <xsl:import href="../../../common/xslt/html/tab2menu.xsl"/>

  <xsl:template match="tabs">
    <div class="tab">
      <table cellspacing="0" cellpadding="0" border="0" summary="tab bar">
        <tr>
          <xsl:call-template name="base-tabs"/>
        </tr>
      </table>
    </div>
  </xsl:template>

  <xsl:template name="pre-separator">
    <xsl:call-template name="separator"/>
  </xsl:template>

  <xsl:template name="post-separator">
  </xsl:template>

  <xsl:template name="separator">
    <td width="6">
      <img src="{$root}skin/images/spacer.gif" width="6" height="8" alt=""/>
    </td>
  </xsl:template>

  <xsl:template name="selected">
    <td valign="bottom">
      <table cellspacing="0" cellpadding="0" border="0"  style="height: 1.8em" summary="selected tab">
        <tr>
          <td bgcolor="#4C6C8F" width="5" valign="top">
            <img src="{$skin-img-dir}/tabSel-left.gif" alt="" width="5" height="5" />
          </td>
          <td bgcolor="#4C6C8F" valign="middle">
            <font face="Arial, Helvetica, Sans-serif" size="2" color="#ffffff">
              <b>
                <xsl:call-template name="base-selected"/>
              </b>
            </font>
          </td>
          <td bgcolor="#4C6C8F" width="5" valign="top">
            <img src="{$skin-img-dir}/tabSel-right.gif" alt="" width="5" height="5" />
          </td>
        </tr>
      </table>
    </td>
  </xsl:template>

  <xsl:template name="not-selected">
    <td valign="bottom">
      <table cellspacing="0" cellpadding="0" border="0" style="height: 1.6em" summary="non selected tab">
        <tr>
          <td bgcolor="#B2C4E0" width="5" valign="top">
            <img src="{$skin-img-dir}/tab-left.gif" alt="" width="5" height="5" />
          </td>
          <td bgcolor="#B2C4E0" valign="middle">
            <xsl:call-template name="base-not-selected"/>
          </td>
          <td bgcolor="#B2C4E0" width="5" valign="top">
            <img src="{$skin-img-dir}/tab-right.gif" alt="" width="5" height="5" />
          </td>
        </tr>
        <tr>
          <td height="1" colspan="3">
          </td>
        </tr>
      </table>
    </td>
  </xsl:template>

</xsl:stylesheet>
