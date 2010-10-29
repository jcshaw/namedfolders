<?xml version="1.0" encoding="windows-1251"?>
<xsl:stylesheet
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  version="1.0"
>
<!--	
	XSLT-преобразование для создания hlf-файла справки FAR.
	Copyright 2004-2005 by Victor Derevyanko, wingspan@yandex.ru
	http:/www.rammus.ru
	$Rev$
	$Id: hlf.xslt 69 2004-08-06 13:04:17Z dv $
-->
<xsl:output method="text" omit-xml-declaration="yes" indent="yes"  encoding="cp866"/>

<xsl:template match="/help_file">
	<xsl:text>.Language=</xsl:text><xsl:value-of select="Language"/>
	<xsl:text>&#xD;</xsl:text>	
	<xsl:text>.PluginContents=</xsl:text><xsl:value-of select="PluginContents"/>

	<xsl:text>&#xD;</xsl:text>	
		
	<xsl:for-each select="section">
		<xsl:text>@</xsl:text><xsl:value-of select="@title"/>
		<xsl:text>&#xD;</xsl:text>	
		<xsl:apply-templates/>
		<xsl:text>&#xD;</xsl:text>	
	</xsl:for-each>
</xsl:template>

<xsl:template match="title">
	<xsl:text>$^#</xsl:text><xsl:apply-templates/><xsl:text>#</xsl:text>
	<xsl:text>&#xD;</xsl:text>	
</xsl:template>

<xsl:template match="link">
	<xsl:text>~</xsl:text><xsl:value-of select="."/><xsl:text>~@</xsl:text><xsl:value-of select="@section"/><xsl:text>@</xsl:text>
</xsl:template>

<xsl:template match="external">
<!--
	Извлекаем данные из !version.xml
	Поскольку рантайм XPATH формировать нельзя, считаем что
	external/@name может содержать только простые конструкции
	вида RUS/AUTHOR или WWW
	и обрабатываем только их.
-->
	<xsl:variable name="xpath" select="@name"/>
	<xsl:variable name="filename">!version.xml</xsl:variable>
	<xsl:variable name="parent">
			<xsl:value-of select="substring-before(@name, '/')"/>
	</xsl:variable>
	<xsl:variable name="child">
		<xsl:choose>
			<xsl:when test="contains(@name, '/')">
				<xsl:value-of select="substring-after(@name, '/')"/>
			</xsl:when>
			<xsl:otherwise>
				<xsl:value-of select="@name"/>
			</xsl:otherwise>
		</xsl:choose>
	</xsl:variable>
	<xsl:choose>
		<xsl:when test="$parent != ''">
 			<xsl:value-of select="document($filename)/data//*[name()=$parent]/*[name()=$child]"/>
		</xsl:when>
		<xsl:otherwise>
 			<xsl:value-of select="document($filename)/data//*[name()=$child]"/>
		</xsl:otherwise>
	</xsl:choose>
</xsl:template>

<xsl:template match="ss">
	<xsl:text>  #</xsl:text>
	<xsl:value-of select="."/>
	<xsl:text>#</xsl:text>
	<xsl:text>&#xD;</xsl:text>	
	<xsl:text>  #</xsl:text>
	<xsl:call-template name="put_n_chars">
		<xsl:with-param name="char">~</xsl:with-param>
		<xsl:with-param name="count"><xsl:value-of select="string-length(.)*2"/></xsl:with-param>
	</xsl:call-template>
	<xsl:text>#</xsl:text>
	<xsl:text>&#xD;</xsl:text>	
</xsl:template>

<!-- 	Разместить последовательно заданное количество заданных символов
-->
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
