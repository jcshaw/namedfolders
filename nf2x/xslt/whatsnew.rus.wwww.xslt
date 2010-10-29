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
title="������ ������� Named Folders"
next_page="1"
date="$Date$"
>
<par/>
� ��������� ����� ������� ���������� ������ ������ �������. 
������ ������ ��������, ��� ���������, "� ����". 
�� ��������� � <link page="tag=nf_history&amp;id=1">������ �������, </link>
� ���������������� ������� ������ ��� ��������� ���������: ��������� ��������� ���������, 
������� ����� �������, �������������� ����������� ������ ���������� ����������, 
��������� ���������� ��������.

<h1>C����� ��������� � Named Folders 2.x</h1>
<history_list>
<hli type="!">������ ���������</hli>
<hli type="+">������������</hli>
<hli type="-">���������� ������</hli>
<hli type="*">���������</hli>
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
title="������ ������ Named Folders"
prev_page="0"
>
<h1>Named Folders 1.x.</h1>

<h2>�������� ����������� ������ ������</h2>
<ul>
<li>����������� �������� ����������� ��� �������, ��������� � ����������� ����������.</li>
<li>������� ��������� ������: cd:, cd::, cd:+, cd:-</li>
<li>����������� ������. ��������� �������� � �������� �����������.</li>
<li>������ ���������, ������ ������������ �������. ����� � ������, ������������ 
����� �� ����������� ������ ������ �� ����������, ������� ������������� �������� �������</li>
<li>�������� ����������� ���������� �� �� ��������� ���������� ����������.
����������� ���� �� ������� ���� ���������� ���������.</li>
</ul>

� ��������� ����� ���������������
<link page="tag=nf_history&amp;id=0">������ ������ �������.</link>

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
