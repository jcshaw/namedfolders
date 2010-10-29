<?xml version="1.0" encoding="windows-1251"?>
<xsl:stylesheet
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  version="1.0"
>
<xsl:output method="xml" omit-xml-declaration="no" indent="yes"  encoding="windows-1251"/>
<!--
	Copyright 2004-2005 by Victor Derevyanko, wingspan@yandex.ru
	http:/www.rammus.ru
	$Id: whatsnew.rus.wwww.xslt 102 2005-05-29 10:19:36Z dv $
-->

<xsl:template match="/">

<pages>
<page id="0"
title="Версии плагина Named Folders"
next_page="1"
date="$Date$"
>
<par/>
В настоящее время ведется разработка второй версии плагина. 
Вторая версия написана, как говорится, "с нуля". 
По сравнению с <link page="tag=nf_history&amp;id=1">первой версией, </link>
в функциональность плагина внесен ряд серьезных изменений: добавлена поддержка каталогов, 
введены новые команды, оптимизированы возможности выбора требуемого псевдонима, 
увеличено количество настроек.

<h1>Cписок изменений в Named Folders 2.x</h1>
<history_list>
<hli type="!">Важное замечание</hli>
<hli type="+">Нововведение</hli>
<hli type="-">Исправлена ошибка</hli>
<hli type="*">Изменения</hli>
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
title="Первая версия Named Folders"
prev_page="0"
>
<h1>Named Folders 1.x.</h1>

<h2>Основные возможности первой версии</h2>
<ul>
<li>Возможность создания псевдонимов для сетевых, локальных и виртуальных директорий.</li>
<li>Команды командной строки: cd:, cd::, cd:+, cd:-</li>
<li>Виртуальная панель. Поддержка создания и удаления псевдонимов.</li>
<li>Вместо каталогов, плагин поддерживает фильтры. Входя в фильтр, пользователь 
видит на виртуальной панели только те псевдонимы, которые удовлетворяют условиям фильтра</li>
<li>Открытие именованной директории по не полностью введенному псевдониму.
Отображение меню со списком всех подходящих вариантов.</li>
</ul>

В настоящее время разрабатывается
<link page="tag=nf_history&amp;id=0">вторая версия плагина.</link>

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
