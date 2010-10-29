/*
* Far Named Folders 3.x
* Copyright (c) 2002-2010 by Victor Derevyanko, RAMMuS Group
* www: http://www.rammus.ru, http://derevyanko.blogspot.com/
* e-mail: dvpublic0@gmail.com
*/
#pragma once

namespace nf
{

class registry_remover
{
public:
	registry_remover(void){};

//проверить, чтобы целевой ключ не был вложен в исходный ключ
	bool IsKeysOverloaded(tstring const &SrcKey, tstring const &TargetKey);	

//удалить ключ и все вложенные подключи и значения
	bool Erase(tstring const &SrcKey)
	{
		return remove(SrcKey, 0, true);
	}

//переместить содержимое исходного ключа во вложенный
	bool Move(tstring const &SrcKey, tstring const &TargetKey)
	{
		return remove(SrcKey, &TargetKey, true);
	}

//скопировать содержимое исходного ключа во вложеннй
	bool Copy(tstring const &SrcKey, tstring const &TargetKey)
	{
		return remove(SrcKey, &TargetKey, false);
	}

private:
	bool remove(tstring const &SrcKey, tstring const *const pTargetKey, bool bDeleteSrc);
};


} //nf