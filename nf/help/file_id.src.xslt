<?xml version="1.0" encoding="windows-1251"?>
<xsl:stylesheet
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  version="1.0"
>
<!--
	Copyright 2004-2005 by Victor Derevyanko, wingspan@yandex.ru
	http:/www.rammus.ru

	$Id: file_id.src.xslt 72 2004-08-06 13:06:26Z dv $
-->

<xsl:output method="text" omit-xml-declaration="yes" indent="yes"  encoding="windows-1251"/>

<xsl:template match="/data">

<xsl:text>Плагин FAR: Именованные Директории      
Версия: </xsl:text><xsl:value-of select="//RUS/VERSION"/>
<xsl:text>, </xsl:text>
<xsl:value-of select="//RUS/DATE"/>					
<xsl:text>

Плагин для быстрого доступа к локальным, сетевым и виртуальным 
директориям в FAR. Командой "cd::abc" пользователь присваивает 
открытой директории псевдоним "abc" и после этого может переходить 
в эту директорию командой "cd:abc". 

</xsl:text>
<xsl:value-of select="//RUS/COPYRIGHT"/>
<xsl:text>&#xD;</xsl:text>

<xsl:text>e-mail: </xsl:text>
<xsl:value-of select="//CONTACT_EMAIL"/>
<xsl:text>&#xD;</xsl:text>

<xsl:text>icq: </xsl:text>
<xsl:value-of select="//ICQ"/>
<xsl:text>&#xD;</xsl:text>

<xsl:text>www: </xsl:text>
<xsl:value-of select="//RAMMUS_WWW"/>
<xsl:text>&#xD;</xsl:text>

<xsl:text>

FAR Plugin: Named Folders        
Version: </xsl:text><xsl:value-of select="//ENG/VERSION"/>
<xsl:text>, </xsl:text>
<xsl:value-of select="//ENG/DATE"/>					
<xsl:text>

This plugin can be used for fast access to local, net and virtual 
directories in the FAR. Using "cd::abc" you set short name "abc" for 
the directory, opened in the active FAR panel. Then you can jump 
to this directory executing "cd:abc". 

</xsl:text>
<xsl:value-of select="//ENG/COPYRIGHT"/>
<xsl:text>&#xD;</xsl:text>
<xsl:text>e-mail: </xsl:text>
<xsl:value-of select="//CONTACT_EMAIL"/>
<xsl:text>&#xD;</xsl:text>

<xsl:text>icq: </xsl:text>
<xsl:value-of select="//ICQ"/>
<xsl:text>&#xD;</xsl:text>

<xsl:text>www: </xsl:text>
<xsl:value-of select="//RAMMUS_WWW"/>
<xsl:text>&#xD;</xsl:text>

</xsl:template>

</xsl:stylesheet>
