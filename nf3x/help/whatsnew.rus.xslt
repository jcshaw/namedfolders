<?xml version="1.0" encoding="windows-1251"?>
<xsl:stylesheet
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  version="1.0"
>
<xsl:output method="text" omit-xml-declaration="yes" indent="yes"  encoding="windows-1251"/>
<!--
	Copyright 2004-2010 by Victor Derevyanko, dvpublic0@gmail.com
	http://code.google.com/p/namedfolders/
	$Id: whatsnew.rus.xslt 71 2004-08-06 13:05:59Z dv $
-->

<xsl:template match="/">

<xsl:text>                    Плагин "Named Folders"
                    ~~~~~~~~~~~~~~~~~~~~~~
                    </xsl:text>
<xsl:text>

[!] Важное замечание
[+] Нововведение
[-] Исправлена ошибка
[*] Изменения

</xsl:text>
<xsl:for-each select="whatsnew/version">
	<xsl:text>----------------------------------------------------------------------------</xsl:text>	
	<xsl:text>&#xD;</xsl:text>
	<xsl:text>Версия </xsl:text>
	<xsl:value-of select="@version"/>
	<xsl:text> [</xsl:text>
	<xsl:value-of select="@date"/>
	<xsl:text>]</xsl:text>
	<xsl:text>&#xD;</xsl:text>	
	<xsl:text>----------------------------------------------------------------------------</xsl:text>	
	<xsl:text>&#xD;</xsl:text>	

	<xsl:for-each select="group">
		<xsl:value-of select="@title"/>
		<xsl:text>&#xD;</xsl:text>
		<xsl:call-template name="put_n_chars">
			<xsl:with-param name="count"><xsl:value-of select="string-length(@title)"/></xsl:with-param>
		</xsl:call-template>
		<xsl:text>&#xD;</xsl:text>
		<xsl:call-template name="put-items"/>
		<xsl:text>&#xD;</xsl:text>
	</xsl:for-each>

	<xsl:call-template name="put-items"/>
	<xsl:text>&#xD;</xsl:text>

</xsl:for-each>

</xsl:template>

<xsl:template name="put-items">	
	<xsl:for-each select="item">
		<xsl:text>[</xsl:text>
		<xsl:value-of select="@type"/>
		<xsl:text>] </xsl:text>
		<xsl:value-of select="."/>
		<xsl:text>&#xD;</xsl:text>
	</xsl:for-each>	
</xsl:template>

<xsl:template name="put_n_chars">
	<xsl:param name="char">~</xsl:param>
	<xsl:param name="count">0</xsl:param>

	<xsl:if test="number($count) > 0">
		<xsl:value-of select="$char"/>
		<xsl:call-template name="put_n_chars">
			<xsl:with-param name="count"><xsl:value-of select="number($count)-1"/></xsl:with-param>
		</xsl:call-template>
	</xsl:if>
</xsl:template>

</xsl:stylesheet>
