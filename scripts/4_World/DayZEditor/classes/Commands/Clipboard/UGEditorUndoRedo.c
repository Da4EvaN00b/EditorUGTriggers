class UGPropSnapshot
{
	int    Id;
	string Type;
	vector Pos;
	vector Ori;
	// Trigger
	vector Size;
	int    UGType;
	float  Eye;
	float  Interp;
	// Breadcrumb
	int    IsBC;    
	float  BC_Eye;
	int    BC_Ray;
	float  BC_Rad;

	float  ExpireAt; //game time
}

class UGUndoCache
{
	protected static ref array<ref UGPropSnapshot> s_Snaps = new array<ref UGPropSnapshot>();

	static void RememberEO(EditorObject eo, float ttl = 90.0)
	{
		if (!eo) return;
		Object w = eo.GetWorldObject();
		if (!w) return;

		UGTriggerObject ug = UGTriggerObject.Cast(w);
		if (ug) {
			UGPropSnapshot s = new UGPropSnapshot();
			s.Id     = eo.GetID();
			s.Type   = ug.GetType();
			s.Pos    = ug.GetPosition();
			s.Ori    = ug.GetOrientation();
			s.Size   = ug.GetSize();
			s.UGType = ug.GetUGType();
			s.Eye    = ug.GetEyeAccommodation();
			s.Interp = ug.GetInterpolation();
			s.IsBC   = 0;
			s.ExpireAt = GetGame().GetTime() * 0.001 + ttl;
			s_Snaps.Insert(s);
			return;
		}

		UGBreadcrumb bc = UGBreadcrumb.Cast(w);
		if (bc) {
			UGPropSnapshot s2 = new UGPropSnapshot();
			s2.Id     = eo.GetID();
			s2.Type   = bc.GetType();
			s2.Pos    = bc.GetPosition();
			s2.Ori    = bc.GetOrientation();
			s2.IsBC   = 1;
			s2.BC_Eye = bc.GetEyeAccommodation();
			s2.BC_Ray = bc.GetUseRaycast();
			s2.BC_Rad = bc.GetRadius();
			s2.ExpireAt = GetGame().GetTime() * 0.001 + ttl;
			s_Snaps.Insert(s2);
		}
	}

	// Call every frame after input handling (cheap)
	static void Rehydrate(Editor editor)
	{
		if (!editor || s_Snaps.Count() == 0) return;

		float now = GetGame().GetTime() * 0.001;

		// Purge expired
		for (int i = s_Snaps.Count() - 1; i >= 0; i--) {
			if (s_Snaps[i].ExpireAt <= now) s_Snaps.Remove(i);
		}
		if (s_Snaps.Count() == 0) return;

		// Try to rehydrate by ID
		for (int j = s_Snaps.Count() - 1; j >= 0; j--) {
			UGPropSnapshot s = s_Snaps[j];

			EditorObject eo = editor.GetPlacedObjectById(s.Id);
			if (!eo) continue; // not recreated yet

			Object w = eo.GetWorldObject();
			if (!w) { s_Snaps.Remove(j); continue; }

			UGTriggerObject ug = UGTriggerObject.Cast(w);
			if (ug && s.IsBC == 0) {
				vector zero = "0 0 0";
				if (s.Size != zero) ug.SetSize(s.Size);
				ug.SetUGType(s.UGType);
				ug.SetEyeAccommodation(s.Eye);
				ug.SetInterpolation(s.Interp);
				ug.GetLinkedTrigger();
				s_Snaps.Remove(j);
				continue;
			}

			UGBreadcrumb bc = UGBreadcrumb.Cast(w);
			if (bc && s.IsBC == 1) {
				bc.SetEyeAccommodation(s.BC_Eye);
				bc.SetUseRaycast(s.BC_Ray);
				bc.SetRadius(s.BC_Rad);
				UG_RescanTriggersAround(s.Pos, 200.0);
				s_Snaps.Remove(j);
				continue;
			}

			// If type changed or not our class, drop it
			if (w.GetType() != s.Type) {
				s_Snaps.Remove(j);
			}
		}
	}
}
