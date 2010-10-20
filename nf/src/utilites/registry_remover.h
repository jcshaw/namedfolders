/*
* Far Named Folders 3.x
* Copyright (c) 2002-2010 by Victor Derevyanko
* www: http://code.google.com/p/namedfolders/
* e-mail: dvpublic0@gmail.com
*/
#pragma once

namespace nf {

class registry_remover {
public:
	registry_remover(void){};

//���������, ����� ������� ���� �� ��� ������ � �������� ����
	bool IsKeysOverloaded(tstring const &SrcKey, tstring const &TargetKey);	

//������� ���� � ��� ��������� �������� � ��������
	bool Erase(tstring const &SrcKey) {
		return remove(SrcKey, 0, true);
	}

//����������� ���������� ��������� ����� �� ���������
	bool Move(tstring const &SrcKey, tstring const &TargetKey) {
		return remove(SrcKey, &TargetKey, true);
	}

//����������� ���������� ��������� ����� �� ��������
	bool Copy(tstring const &SrcKey, tstring const &TargetKey) {
		return remove(SrcKey, &TargetKey, false);
	}

private:
	bool remove(tstring const &SrcKey, tstring const *const pTargetKey, bool bDeleteSrc);
};

} //nf