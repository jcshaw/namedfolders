/*
* Far Named Folders 3.x
* Copyright (c) 2002-2011 by Victor Derevyanko
* www: http://code.google.com/p/namedfolders/
* e-mail: dvpublic0@gmail.com
*/

#pragma once

#include <boost/utility.hpp>

namespace nf {

class Win7LibrariesManager : boost::noncopyable {
public:
	Win7LibrariesManager();
	~Win7LibrariesManager();
	
	void GetListLibraries(nf::tlist_pairs_strings& destList) const; //get list of libraries from Known Folder "Libraries"
	void GetListFoldersInLibrary(tstring const& libraryFilePath, nf::tlist_strings& destList) const;
	void AddFolderToLibrary(tstring const& libraryFilePath, tstring const& folderPath) const;
	void RemoveFolderFromLibrary(tstring const& libraryFilePath, tstring const& folderPath) const;
private:
	bool const m_bEnabled;
};

}