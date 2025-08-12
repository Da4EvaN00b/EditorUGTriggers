modded class EditorObjectDragHandler
{
    // Re-apply UGTriggerObject after drag
    override void OnDragging(notnull EditorObject target, notnull array<EditorObject> additional_drag_targets)
    {
        super.OnDragging(target, additional_drag_targets);
        UG_ReapplyNonUniformScaleFor(target);
        foreach (EditorObject eo : additional_drag_targets)
        {
            UG_ReapplyNonUniformScaleFor(eo);
        }
    }

    protected void UG_ReapplyNonUniformScaleFor(EditorObject eo)
    {
        if (!eo) return;

        Object w = eo.GetWorldObject();
        UGTriggerObject ugo = UGTriggerObject.Cast(w);
        if (!ugo) return; 

        vector m[4];
        w.GetTransform(m);

        vector ax0 = m[0].Normalized();
        vector ax1 = m[1].Normalized();
        vector ax2 = m[2].Normalized();

        vector s = ugo.GetSize();
        m[0] = ax0 * s[0];
        m[1] = ax1 * s[1];
        m[2] = ax2 * s[2];

        w.SetTransform(m);
        w.Update(); 
    }
}