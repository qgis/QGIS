<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
<xsl:output method="xml" indent="yes"/>

	<xsl:template match="/Site">
		<testsuite>
			<xsl:variable name="BuildName"><xsl:value-of select="@BuildName"/></xsl:variable>
			<xsl:variable name="BuildStamp"><xsl:value-of select="@BuildStamp"/></xsl:variable>
			<xsl:variable name="Name"><xsl:value-of select="@Name"/></xsl:variable>
			<xsl:variable name="Generator"><xsl:value-of select="@Generator"/></xsl:variable>
			<xsl:variable name="CompilerName"><xsl:value-of select="@CompilerName"/></xsl:variable>
			<xsl:variable name="OSName"><xsl:value-of select="@OSName"/></xsl:variable>
			<xsl:variable name="Hostname"><xsl:value-of select="@Hostname"/></xsl:variable>
			<xsl:variable name="OSRelease"><xsl:value-of select="@OSRelease"/></xsl:variable>
			<xsl:variable name="OSVersion"><xsl:value-of select="@OSVersion"/></xsl:variable>
			<xsl:variable name="OSPlatform"><xsl:value-of select="@OSPlatform"/></xsl:variable>
			<xsl:variable name="Is64Bits"><xsl:value-of select="@Is64Bits"/></xsl:variable>
			<xsl:variable name="VendorString"><xsl:value-of select="@VendorString"/></xsl:variable>
			<xsl:variable name="VendorID"><xsl:value-of select="@VendorID"/></xsl:variable>
			<xsl:variable name="FamilyID"><xsl:value-of select="@FamilyID"/></xsl:variable>
			<xsl:variable name="ModelID"><xsl:value-of select="@ModelID"/></xsl:variable>
			<xsl:variable name="ProcessorCacheSize"><xsl:value-of select="@ProcessorCacheSize"/></xsl:variable>
			<xsl:variable name="NumberOfLogicalCPU"><xsl:value-of select="@NumberOfLogicalCPU"/></xsl:variable>
			<xsl:variable name="NumberOfPhysicalCPU"><xsl:value-of select="@NumberOfPhysicalCPU"/></xsl:variable>
			<xsl:variable name="TotalVirtualMemory"><xsl:value-of select="@TotalVirtualMemory"/></xsl:variable>
			<xsl:variable name="TotalPhysicalMemory"><xsl:value-of select="@TotalPhysicalMemory"/></xsl:variable>
			<xsl:variable name="LogicalProcessorsPerPhysical"><xsl:value-of select="@LogicalProcessorsPerPhysical"/></xsl:variable>
			<xsl:variable name="ProcessorClockFrequency"><xsl:value-of select="@ProcessorClockFrequency"/></xsl:variable>
			<properties>
				<property name="BuildName" value="{$BuildName}" />
				<property name="BuildStamp" value="{$BuildStamp}" />
				<property name="Name" value="{$Name}" />
				<property name="Generator" value="{$Generator}" />
				<property name="CompilerName" value="{$CompilerName}" />
				<property name="OSName" value="{$OSName}" />
				<property name="Hostname" value="{$Hostname}" />
				<property name="OSRelease" value="{$OSRelease}" />
				<property name="OSVersion" value="{$OSVersion}" />
				<property name="OSPlatform" value="{$OSPlatform}" />
				<property name="Is64Bits" value="{$Is64Bits}" />
				<property name="VendorString" value="{$VendorString}" />
				<property name="VendorID" value="{$VendorID}" />
				<property name="FamilyID" value="{$FamilyID}" />
				<property name="ModelID" value="{$ModelID}" />
				<property name="ProcessorCacheSize" value="{$ProcessorCacheSize}" />
				<property name="NumberOfLogicalCPU" value="{$NumberOfLogicalCPU}" />
				<property name="NumberOfPhysicalCPU" value="{$NumberOfPhysicalCPU}" />
				<property name="TotalVirtualMemory" value="{$TotalVirtualMemory}" />
				<property name="TotalPhysicalMemory" value="{$TotalPhysicalMemory}" />
				<property name="LogicalProcessorsPerPhysical" value="{$LogicalProcessorsPerPhysical}" />
				<property name="ProcessorClockFrequency" value="{$ProcessorClockFrequency}" />
			</properties>
			<xsl:apply-templates select="Testing/Test"/>
			
			<system-out>
				BuildName: <xsl:value-of select="$BuildName" />
				BuildStamp: <xsl:value-of select="$BuildStamp" />
				Name: <xsl:value-of select="$Name" />
				Generator: <xsl:value-of select="$Generator" />
				CompilerName: <xsl:value-of select="$CompilerName" />
				OSName: <xsl:value-of select="$OSName" />
				Hostname: <xsl:value-of select="$Hostname" />
				OSRelease: <xsl:value-of select="$OSRelease" />
				OSVersion: <xsl:value-of select="$OSVersion" />
				OSPlatform: <xsl:value-of select="$OSPlatform" />
				Is64Bits: <xsl:value-of select="$Is64Bits" />
				VendorString: <xsl:value-of select="$VendorString" />
				VendorID: <xsl:value-of select="$VendorID" />
				FamilyID: <xsl:value-of select="$FamilyID" />
				ModelID: <xsl:value-of select="$ModelID" />
				ProcessorCacheSize: <xsl:value-of select="$ProcessorCacheSize" />
				NumberOfLogicalCPU: <xsl:value-of select="$NumberOfLogicalCPU" />
				NumberOfPhysicalCPU: <xsl:value-of select="$NumberOfPhysicalCPU" />
				TotalVirtualMemory: <xsl:value-of select="$TotalVirtualMemory" />
				TotalPhysicalMemory: <xsl:value-of select="$TotalPhysicalMemory" />
				LogicalProcessorsPerPhysical: <xsl:value-of select="$LogicalProcessorsPerPhysical" />
				ProcessorClockFrequency: <xsl:value-of select="$ProcessorClockFrequency" />
			</system-out>
		</testsuite>
	</xsl:template>

    <xsl:template match="Testing/Test">
        <xsl:variable name="testcasename"><xsl:value-of select= "Name"/></xsl:variable>
		<xsl:variable name="exectime">
			<xsl:for-each select="Results/NamedMeasurement">
				<xsl:if test="@name = 'Execution Time'">
					<xsl:value-of select="."/>
				</xsl:if>
			</xsl:for-each>
		</xsl:variable>
		
			<testcase name="{$testcasename}" classname="TestSuite" time="{$exectime}">
            <xsl:if test="@Status = 'passed'">
            </xsl:if>
            <xsl:if test="@Status = 'failed'">
				<xsl:variable name="failtype">
					<xsl:for-each select="Results/NamedMeasurement">
						<xsl:if test="@name = 'Exit Code'">
							<xsl:value-of select="."/>
						</xsl:if>
					</xsl:for-each>
				</xsl:variable>
				<xsl:variable name="failcode">
					<xsl:for-each select="Results/NamedMeasurement">
						<xsl:if test="@name = 'Exit Value'">
							<xsl:value-of select="."/>
						</xsl:if>
					</xsl:for-each>
				</xsl:variable>
                <error message="{$failtype} ({$failcode})"><xsl:value-of select="Results/Measurement/Value/text()" /></error>
            </xsl:if>
            <xsl:if test="@Status = 'notrun'">
                <skipped><xsl:value-of select="Results/Measurement/Value/text()" /></skipped>
            </xsl:if>
	    <system-out><xsl:value-of select="Results/Measurement/Value/text()" /></system-out>
        </testcase>
    </xsl:template>

</xsl:stylesheet>

