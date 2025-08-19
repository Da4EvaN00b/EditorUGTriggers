static int UG_DeleteAllUGObjects(Editor editor)
{
    if (!editor) return 0;

    EditorObjectMap placed = editor.GetPlacedObjects();
    if (!placed || placed.Count() == 0) return 0;

    ref array<EditorObject> to_delete = new array<EditorObject>();
    foreach (EditorObject eo: placed) {
        if (!eo) continue;
        Object w = eo.GetWorldObject();
        if (!w) continue;

        bool isUG =(UGTriggerObject.Cast(w) != null) || (UGBreadcrumb.Cast(w) != null) || w.IsKindOf("UGTriggerObject") || w.IsKindOf("UGBreadcrumb");

        if (isUG) to_delete.Insert(eo);
    }

    if (to_delete.Count() == 0) return 0;

    editor.DeleteObjects(to_delete);
    return to_delete.Count();
}

modded class EditorExportDialogController : DialogBaseController
{
	bool remove_ug_after_export;
}