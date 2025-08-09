modded class UndergroundTrigger
{
	void SetExtentsFromSize(vector size)
	{
		vector e = size * 0.5;    // half-size per axis
		SetExtents(-e, e);
	}

	void SetAccommodation(float v)     { m_Accommodation = v; }
	void SetInterpolationSpeed(float v){ m_InterpolationSpeed = v; }

	void SetTypeForAccommodation()
    {
    if (m_Accommodation == 1)
        {
            m_Type = EUndergroundTriggerType.OUTER;
        }
        else
        {
            m_Type = EUndergroundTriggerType.INNER;
        }
    }
}

modded class EditorObject
{
    // --- Keep UGTriggerObject free of the editor AABB (optional, as you had) ---
    override void EnableBoundingBox(bool enable)
    {
        Object w = GetWorldObject();
        if (UGTriggerObject.Cast(w))
        {
            DestroyBoundingBox();
            return;
        }
        super.EnableBoundingBox(enable);
    }
}
modded class EditorGizmo
{
    override void Update(float dt)
    {
        // If nothing selected, just do normal behavior
        auto editor = GetEditor();
        if (!editor)
        {
            super.Update(dt);
            return;
        }

        // Who’s on top of the selection stack?
        ref array<EditorObject> ordered = editor.GetSelectedObjectsOrdered();
        if (!ordered || ordered.Count() == 0)
        {
            super.Update(dt);
            return;
        }

        EditorObject top = ordered[ordered.Count() - 1];
        bool isUG = top && UGTriggerObject.Cast(top.GetWorldObject());

        if (isUG)
        {
            // Kill any current drag
            m_InteractionIndex  = -1;
            m_DragOffset        = vector.Zero;
            m_DragRotationOffset= vector.Zero;

            // Hide the gizmo so there’s nothing to grab
            if (m_Gizmo)
                m_Gizmo.SetFlags(EntityFlags.VISIBLE, false);

            // Do NOT call base Update — skip collisions, transforms, etc.
            return;
        }
        else
        {
            // Ensure gizmo is visible for normal objects
            if (m_Gizmo)
                m_Gizmo.SetFlags(EntityFlags.VISIBLE, true);

            // Normal gizmo behavior
            super.Update(dt);
        }
    }
}
modded class EditorObjectDragHandler
{
    override void OnDragging(notnull EditorObject target, notnull array<EditorObject> additional_drag_targets)
    {
        // Let the vanilla handler do its move/rotate first
        super.OnDragging(target, additional_drag_targets);

        // Re-assert per-axis lengths on the main object (if it's our type)
        UG_ReapplyNonUniformScaleFor(target);

        // And on all additional dragged objects too (multi-select)
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
        if (!ugo) return; // only care about our helper type

        // Read current (dragged) world transform
        vector m[4];
        w.GetTransform(m);

        // Remove any "cube" scale editor imposed; keep directions
        vector ax0 = m[0].Normalized();
        vector ax1 = m[1].Normalized();
        vector ax2 = m[2].Normalized();

        // Re-apply our intended per-axis lengths
        vector s = ugo.GetSize();
        m[0] = ax0 * s[0];
        m[1] = ax1 * s[1];
        m[2] = ax2 * s[2];

        // Commit back
        w.SetTransform(m);
        w.Update(); // keep visuals consistent with transform
    }
}