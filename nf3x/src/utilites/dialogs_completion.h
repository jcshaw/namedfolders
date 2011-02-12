#pragma once

namespace nf {
namespace DialogsCompletion {

	bool IsDialogsCompletionOn();
	void SetDialogsCompletionResult(tstring const& destResult);
	void OpenFromDialog(HANDLE hDlg);

}
}