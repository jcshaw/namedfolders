<?xml version="1.0" encoding="windows-1251"?>
<xsl:stylesheet
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  version="1.0"
>
<xsl:output method="xml" omit-xml-declaration="no" indent="yes"  encoding="windows-1251"/>
<!--
	Copyright 2004-2010 by Victor Derevyanko, dvpublic0@gmail.com
	http://code.google.com/p/namedfolders/
	$Id: whatsnew.eng.wwww.xslt 102 2005-05-29 10:19:36Z dv $
-->

<xsl:template match="/">

<pages>
<page id="0"
title="Named Folders 2.0"
next_page="1"
date="$Date$"
>
<par/>
Second version of the plugin has been developed since 2002.
In comparison with
<link page="tag=nf_history&amp;id=1">first version,</link>
second version contains considerably more features, commands 
and improvements in usability. Please look 
<link page="tag=nf_doc">documentation</link>
for more information.

<h1>What's new in Named Folders 2.x</h1>
<history_list>
<hli type="!">Important</hli>
<hli type="+">New</hli>
<hli type="-">Fixed error</hli>
<hli type="*">Changes</hli>
</history_list>

<xsl:for-each select="whatsnew/version">
	<xsl:element name="history_version">
		<xsl:attribute name="product_name">Named Folders</xsl:attribute>
		<xsl:attribute name="version"><xsl:value-of select="@version"/></xsl:attribute>
		<xsl:attribute name="date"><xsl:value-of select="@date"/></xsl:attribute>
	</xsl:element>
	<xsl:element name="history_list">
  	<xsl:for-each select="group">
  		<xsl:element name="history_group">
  			<xsl:value-of select="@title"/>
  		</xsl:element>
  		<xsl:call-template name="put-items"/>
  	</xsl:for-each>
 		<xsl:call-template name="put-items"/>
	</xsl:element>
</xsl:for-each>

</page>
<page id="1"
title="Named Folders 1.0"
prev_page="0"
>
<h1>Named Folders 1.x.</h1>

<h2>Some key features of Named Folders 1.0</h2>
<ul>
<li>Creation of shortcuts for local, network and virtual directories.</li>
<li>Commands of command line: cd:, cd::, cd:+, cd:-</li>
<li>Virtual panel with list of shortcuts.</li>
<li>Support of filters (instead of catalogs).</li>
<li>Autocompletion of entered shortcut name (suggestion of the list
of all appropriated shortcuts).</li>
</ul>

<link page="tag=nf_history&amp;id=0">Second verstion</link> of the NamedFolders is 
available now.

</page>
</pages>

</xsl:template>

<xsl:template name="put-items">	
	<xsl:for-each select="item">
		<xsl:element name="hli">
			<xsl:attribute name="type"><xsl:value-of select="@type"/></xsl:attribute>
			<xsl:apply-templates/>
		</xsl:element>
	</xsl:for-each>	
</xsl:template>


</xsl:stylesheet>
