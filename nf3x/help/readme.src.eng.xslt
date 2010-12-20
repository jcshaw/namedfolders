<?xml version="1.0" encoding="windows-1251"?>
<xsl:stylesheet
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  version="1.0"
>
<!--
	Copyright 2004-2005 by Victor Derevyanko, wingspan@yandex.ru
	http:/www.rammus.ru
	$Id: readme.src.eng.xslt 71 2004-08-06 13:05:59Z dv $
-->

<xsl:output method="text" omit-xml-declaration="yes" indent="yes"  encoding="windows-1251"/>

<xsl:template match="/data">

<xsl:text>Plugin for FAR: Named Folders.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
</xsl:text>

<xsl:text>author: </xsl:text>
<xsl:value-of select="//ENG/AUTHOR"/>
<xsl:text>&#xD;</xsl:text>

<xsl:text>e-mail: </xsl:text>
<xsl:value-of select="//CONTACT_EMAIL"/>
<xsl:text>&#xD;</xsl:text>

<xsl:text>www: </xsl:text>
<xsl:value-of select="//RAMMUS_WWW"/>
<xsl:text>&#xD;</xsl:text>

<xsl:text>Russia, Krasnoyarsk</xsl:text>
<xsl:text>&#xD;</xsl:text>


<xsl:text>&#xD;</xsl:text>
<xsl:value-of select="/ENG/VERSION"/>
<xsl:text>&#xD;</xsl:text>
<xsl:value-of select="/ENG/DATE"/>

<xsl:text>DISTRIBUTION: FREEWARE.

DISCLAIMER: The author is not responsible for any consequences of using
this product, though in testing he didn't find anything terrible in it :)

0.  Installation and running
	Delete the catalog with previous version of this plugin (if exists).
    Copy NamedFolders.dll, to the Plugins subdirectory of your 
	FAR directory, or to any subdirectory under Plugins. 
	Start, if needed, reg files.
	Restart FAR.

1.	New versions of the plugin and documentation 
    are available on </xsl:text><xsl:value-of select="//RAMMUS_WWW"/>	

</xsl:template>

</xsl:stylesheet>
