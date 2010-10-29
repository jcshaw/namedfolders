<?xml version="1.0" encoding="windows-1251"?>
<xsl:stylesheet
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  version="1.0"
>

<!--
	Copyright 2004-2005 by Victor Derevyanko, wingspan@yandex.ru
	http:/www.rammus.ru
	$Id: readme.src.rus.xslt 71 2004-08-06 13:05:59Z dv $
-->

<xsl:output method="text" omit-xml-declaration="yes" indent="yes"  encoding="windows-1251"/>

<xsl:template match="/data">

<xsl:text>Плагин для Far: Именованные директории 	     
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
</xsl:text>

<xsl:text>Автор: </xsl:text>
<xsl:value-of select="//RUS/AUTHOR"/>
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

<xsl:text>Россия, г.Красноярск, RAMMuS Group</xsl:text>
<xsl:text>&#xD;</xsl:text>


<xsl:text>&#xD;</xsl:text>
<xsl:value-of select="/RUS/VERSION"/>
<xsl:text>&#xD;</xsl:text>
<xsl:value-of select="/RUS/DATE"/>

<xsl:text>УСЛОВИЯ РАСПРОСТРАНЕНИЯ: FreeWare.
ВНИМАНИЕ: Автор не несет никакой ответственности за правильную или неправильную     
работу этого плагина.

0. Инсталляция плагина.
	- Удалить директорию с предыдущей версией плагина (если есть).
	- Создать любую директорию в Far\Plugins, например,  Far\Plugins\NF.
	- Скопировать в нее NamedFolders.dll.
	- Если нужно, запустить reg файлы.
	- Перезапустить FAR.
   Проверить работу можно так: дать команду cd::a, перейти в любую
   другую директорию, дать команду cd:a - должна открыться
   прежняя директория.

1. Свежие версии плагина и документация доступны 
   на сайте </xsl:text><xsl:value-of select="//RAMMUS_WWW"/>	

</xsl:template>

</xsl:stylesheet>
