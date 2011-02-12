#include "StdAfx.h"
#include "dialogs_completion.h"

#include <boost/scope_exit.hpp>
#include "open_plugin.h"

bool g_IsDialogCompletionOn = 0;
tstring g_DialogCompletionResult = L"";

namespace {
	void set_dialog_completion(bool bValue) {
		g_IsDialogCompletionOn = bValue;
	}
}

bool nf::DialogsCompletion::IsDialogsCompletionOn() {
	return g_IsDialogCompletionOn;
}

void nf::DialogsCompletion::SetDialogsCompletionResult(tstring const& destResult) {
	g_DialogCompletionResult = destResult;
}

void nf::DialogsCompletion::OpenFromDialog(HANDLE hDlg) {
	int focused_item = static_cast<int>(g_PluginInfo.SendDlgMessage(hDlg, DM_GETFOCUS, 0, 0));
	if (focused_item == 0) return;

	EditorSetPosition esp;
	if (! g_PluginInfo.SendDlgMessage(hDlg, DM_GETEDITPOSITION, focused_item, reinterpret_cast<LONG_PTR>(&esp))) return; //this is not text field

	FarDialogItemData fdi;
	fdi.PtrLength = static_cast<size_t>(g_PluginInfo.SendDlgMessage(hDlg, DM_GETTEXT, focused_item, 0));
	nf::tautobuffer_char buffer(fdi.PtrLength + 1);
	fdi.PtrData = &buffer[0];

	if (! g_PluginInfo.SendDlgMessage(hDlg, DM_GETTEXT, focused_item, reinterpret_cast<LONG_PTR>(&fdi))) return; 

	tstring nf_command = &buffer[0];

	g_DialogCompletionResult.clear();
	set_dialog_completion(true);
	BOOST_SCOPE_EXIT( (&hDlg) ) {
		set_dialog_completion(false);
	} BOOST_SCOPE_EXIT_END;

	nf::OpenFromDialog(nf_command.c_str());

	if (! g_DialogCompletionResult.empty()) {
		buffer.resize(g_DialogCompletionResult.size() + 1);
		lstrcpyW(&buffer[0], g_DialogCompletionResult.c_str());
		g_PluginInfo.SendDlgMessage(hDlg, DM_SETTEXT, focused_item, reinterpret_cast<LONG_PTR>(&fdi));
	}
}
