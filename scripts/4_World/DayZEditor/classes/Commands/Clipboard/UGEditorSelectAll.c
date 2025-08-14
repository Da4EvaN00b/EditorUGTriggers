modded class Editor
{
    void UG_SelectAllUGObjects()
    {
        EditorObjectMap placed = GetPlacedObjects();
        if (!placed || placed.Count() == 0) {
            GetEditorHud().CreateNotification("[UG Triggers] No placed objects found");
            return;
        }

        ClearSelection();

        int picked = 0;
        foreach (EditorObject eo: placed) {
            if (!eo) continue;

            Object w = eo.GetWorldObject();
            if (!w) continue;

            if (w.IsInherited(UGTriggerObject) || w.IsInherited(UGBreadcrumb)) {
                SelectObject(eo);
                picked++;
            }
        }

        if (picked > 0) {
            GetEditorHud().CreateNotification(string.Format("[UG Triggers] Selected %1 UGTrigger objects", picked));
        } else {
            GetEditorHud().CreateNotification("[UG Triggers] No UGTrigger objects to select");
        }
    }
}