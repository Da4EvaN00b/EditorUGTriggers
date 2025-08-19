modded class EditorFileDialog : EditorModal
{
    protected Widget         m_UGRow;
    protected CheckBoxWidget m_UGCheck;
    protected bool           m_InitUG;

    override string GetLayoutFile()
    {
        return "EditorUGTriggers/GUI/layouts/dialogs/FileDialogUG.layout";
    }

    override void Update(float dt)
    {
        if (!m_InitUG) {

            m_UGRow   = m_LayoutRoot.FindAnyWidget("ExtraSettingUG");
            m_UGCheck = CheckBoxWidget.Cast(m_UGRow.FindAnyWidget("ExtraSettingUGCheckBox"));

            if (m_UGRow) m_UGRow.Show(true);

            m_InitUG = true;
        }
        super.Update(dt);
    }

    protected int BuildResultMask()
    {
        int mask = 0;

        if (ExtraSettingCheckBox && ExtraSettingCheckBox.IsChecked()) {
            mask |= eDialogExtraSetting.EXPORT_SELECTED_ONLY;
        }
        return mask;
    }

    override bool OnClick(Widget w, int x, int y, int button)
    {
        if (button == 0) {
            switch (w) {
                case SaveButton: {
                    if (!FileNameBox.GetText() && !(m_DialogFlags & eDialogFlags.ALLOW_EMPTY_FILES)) {
                        m_EditorMessageBox = new EditorMessageBox("Please select a valid file name", MessageBoxButtons.OK, null);
                        return true;
                    }

                    string final_file = SystemPath.Combine(m_CurrentDirectory, FileNameBox.GetText());
                    if (FileExist(final_file) && (m_DialogFlags & eDialogFlags.WARN_ON_OVERWRITE)) {
                        string msg = string.Format("Overwrite File %1?", final_file);
                        m_EditorMessageBox = new EditorMessageBox(msg, MessageBoxButtons.OKCancel, ScriptCaller.Create(ForceOverwriteFileFromTextBox));
                        return true;
                    }

                    if (m_ScriptCallback) {
                        UGExportState.RemoveUGAfterExport = (m_UGCheck && m_UGCheck.IsChecked());

                        int mask = BuildResultMask();
                        m_ScriptCallback.Invoke(final_file, mask);
                        Delete();
                        return true;
                    }
                    break;
                }
            }
        }
        return super.OnClick(w, x, y, button);
    }

    override void OnFileDoublePressed(EditorFileView view, string file)
    {
        bool is_directory = view.IsDirectory();
        if (is_directory) {
            GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(SetDirectory, 0, 0, file, true);
        } else if (m_DialogFlags & eDialogFlags.ALLOW_DOUBLE_CLICK) {
            if (m_ScriptCallback) {
                UGExportState.RemoveUGAfterExport = (m_UGCheck && m_UGCheck.IsChecked());

                int mask = BuildResultMask();
                m_ScriptCallback.Invoke(file, mask);
                Delete();
            }
        }
    }

    override void ForceOverwriteFileFromTextBox(DialogResult result)
    {
        if (result == DialogResult.OK) {
            string final_file = SystemPath.Combine(m_CurrentDirectory, FileNameBox.GetText());
            if (m_ScriptCallback) {
                UGExportState.RemoveUGAfterExport = (m_UGCheck && m_UGCheck.IsChecked());

                int mask = BuildResultMask();
                m_ScriptCallback.Invoke(final_file, mask);
            }
            Delete();
        }
    }
}
class UGExportState
{
    static bool RemoveUGAfterExport = false;
}