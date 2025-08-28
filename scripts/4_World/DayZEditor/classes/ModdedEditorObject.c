modded class EditorObject
{
	override void SetBoundingBox(bool state, bool set_flags = false)
	{
		Object w = GetWorldObject();
		if (UGTriggerObject.Cast(w)) {
			if (set_flags) {
				m_Data.Flags &= ~EditorObjectFlags.BBOX;
			}
			HideBoundingBox();
			return;
		}

		super.SetBoundingBox(state, set_flags);
	}

	override void ShowBoundingBox()
	{
		if (UGTriggerObject.Cast(GetWorldObject())) {
			return;
		}
		super.ShowBoundingBox();
	}
}