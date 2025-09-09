modded class EditorDragHandler
{
    protected ref map<EditorObject, float> m_UGStartRoll = new map<EditorObject, float>();
    protected float UG_ComputeRoll(vector right_unit, vector up_unit, vector fwd_unit)
    {
        vector refRight = (vector.Up * fwd_unit).Normalized();
        float s = vector.Dot(fwd_unit, Math3D.CrossProduct(refRight, right_unit));
        float c = vector.Dot(refRight, right_unit);
        return Math.Atan2(s, c);
    }

    override void OnDragStart(notnull EditorObject target, array<EditorObject> additional_targets = null)
    {
        super.OnDragStart(target, additional_targets);

        m_UGStartRoll.Clear();

        array<EditorObject> all = {};
        all.Insert(target);
        if (additional_targets) all.InsertAll(additional_targets);

        foreach (EditorObject eo : all) {
            if (!eo) continue;
            UGTriggerObject ug = UGTriggerObject.Cast(eo.GetWorldObject());
            if (!ug) continue;

            vector m[4];
            eo.GetWorldObject().GetTransform(m);
            vector r = m[0].Normalized();
            vector u = m[1].Normalized();
            vector f = m[2].Normalized();
            m_UGStartRoll[eo] = UG_ComputeRoll(r, u, f);
        }
    }

    override void OnDragFinish()
    {
        m_UGStartRoll.Clear();
        super.OnDragFinish();
    }
}