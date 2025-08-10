class UGTriggerObject : Building
{
	protected vector m_Size; // desired WORLD size X,Y,Z (meters)
	protected UndergroundTrigger m_UndergroundTrigger;
	protected ref Timer m_SyncTimer;
	protected bool   m_RescanQueued;
	protected vector m_LastPosePos;
	protected vector m_LastPoseOri;

	// Remember what the user picked in the dialog (0=Outer,1=Inner,2=Transitional)
	// -1 means "follow live trigger"
	protected int m_DesiredUGType = -1;

	// ----- Type (0=Outer, 1=Inner, 2=Transitional) -----
	int GetUGType()
	{
		// Show the user's intended type if set
		if (m_DesiredUGType >= 0) return m_DesiredUGType;

		UndergroundTrigger trig = GetLinkedTrigger();
		if (trig) return trig.m_Type; // base-game field
		return 0; // default Outer
	}

	void SetUGType(int type)
	{
		type = Math.Clamp(type, 0, 2);
		m_DesiredUGType = type; // remember the user's intent

		UndergroundTrigger trig = GetLinkedTrigger();
		if (!trig) return;

		// Apply default EyeAccommodation (still editable later)
		if (type == 0)       SetEyeAccommodation(1.0); // Outer
		else /* 1 or 2 */    SetEyeAccommodation(0.0); // Inner/Transitional

		// Only Transitional supports breadcrumbs — clear when leaving Transitional
		if (type != 2 && trig.m_Data)
		{
			trig.m_Data.Breadcrumbs = null;
		}

		// **Push live type immediately (Outer / Inner / Transitional)**
		trig.m_Type = type;
	}

	// ----- EyeAccommodation / Interpolation read/write directly to trigger -----
	void SetEyeAccommodation(float v)
	{
		v = UG_Round2(v);
		UndergroundTrigger trig = GetLinkedTrigger();
		if (trig)
		{
			trig.m_Accommodation = v;
	
			// If the user picked Transitional, keep it Transitional even if acc == 0
			if (m_DesiredUGType == 2)
				trig.m_Type = EUndergroundTriggerType.TRANSITIONING;
		}
	}

	float GetEyeAccommodation()
	{
		UndergroundTrigger trig = GetLinkedTrigger();
		if (trig) return trig.m_Accommodation;
		return 1.0; // default
	}

	void SetInterpolation(float v)
	{
		v = Math.Clamp(v, 0.0, 1.0);
		UndergroundTrigger trig = GetLinkedTrigger();
		if (trig) { trig.m_InterpolationSpeed = v; }
	}

	float GetInterpolation()
	{
		UndergroundTrigger trig = GetLinkedTrigger();
		if (trig) return trig.m_InterpolationSpeed;
		return 1.0; // default
	}

	void UGTriggerObject()
	{
		m_Size = Vector(1,1,1);
		ApplySizeTransform();

		CreateTriggerIfMissing();
		UpdateTrigger(); 

		m_LastPosePos = GetPosition();
   		m_LastPoseOri = GetOrientation();
		m_SyncTimer = new Timer(CALL_CATEGORY_SYSTEM);
		m_SyncTimer.Run(0.05, this, "UpdateTriggerPoseOnly", null, true);
	}

	void ~UGTriggerObject()
	{
		if (m_SyncTimer) m_SyncTimer.Stop();
		if (m_UndergroundTrigger && !m_UndergroundTrigger.IsSetForDeletion())
			m_UndergroundTrigger.Delete();
	}

	void SetSize(vector sizeMeters)
	{
		vector s = sizeMeters;
		if (s[0] <= 0.001) s[0] = 0.001;
		if (s[1] <= 0.001) s[1] = 0.001;
		if (s[2] <= 0.001) s[2] = 0.001;
		m_Size = s;
		ApplySizeTransform();
		UpdateTrigger();
		QueueCrumbRescan();
	}

	void TrigSize(float dx, float dy, float dz)
	{
		m_Size[0] = Math.Max(m_Size[0] + dx, 0.01);
		m_Size[1] = Math.Max(m_Size[1] + dy, 0.01);
		m_Size[2] = Math.Max(m_Size[2] + dz, 0.01);
		ApplySizeTransform();
		UpdateTrigger(); // pose + extents
		QueueCrumbRescan();
	}

	void SetTransformData()
	{
		vector m[4];
		GetTransform(m);
		m_Size = Vector(m[0].Length(), m[1].Length(), m[2].Length());
		UpdateTrigger(); // pose + extents
	}

	vector GetSize() { return m_Size; }
	UndergroundTrigger GetLinkedTrigger() { return m_UndergroundTrigger; }

	protected void CreateTriggerIfMissing()
	{
		if (m_UndergroundTrigger) return;

		m_UndergroundTrigger = UndergroundTrigger.Cast(GetGame().CreateObjectEx("UndergroundTrigger", GetPosition(), ECE_LOCAL));
		if (!m_UndergroundTrigger)
		{
			Print("[UGTriggerObject] Failed to spawn UndergroundTrigger");
			return;
		}

		// Defaults: Outer
		m_UndergroundTrigger.m_Accommodation      = 1.0; // Outer default
		m_UndergroundTrigger.m_InterpolationSpeed = 1.0;
		m_UndergroundTrigger.m_Type               = EUndergroundTriggerType.OUTER;

		// initial desired type follows the live trigger until user changes it
		m_DesiredUGType = -1;

		m_UndergroundTrigger.SetPosition(GetPosition());
		m_UndergroundTrigger.SetOrientation(GetOrientation());
	}

	// ===== Breadcrumb collection (Transitional-only) =====
	protected bool IsPointInsideOBB(vector p, out vector right, out vector up, out vector fwd, out vector pos, out vector half)
	{
		vector T[4];
		GetTransform(T);
		right = T[0].Normalized();
		up    = T[1].Normalized();
		fwd   = T[2].Normalized();
		pos   = T[3];
		half  = m_Size * 0.5;

		vector d = p - pos;
		float lx = d * right;
		float ly = d * up;
		float lz = d * fwd;

		return (Math.AbsFloat(lx) <= half[0] + 1e-3 && Math.AbsFloat(ly) <= half[1] + 1e-3 && Math.AbsFloat(lz) <= half[2] + 1e-3);
	}

	void GetBreadcrumbs()
	{
		Print("[UG] GetBreadcrumbs (OBB/Transitional-only): " + this);

		UndergroundTrigger t = GetLinkedTrigger();
		if (!t) { CreateTriggerIfMissing(); t = GetLinkedTrigger(); if (!t) { Print("[UG][ERR] no trigger; abort"); return; } }

		// Only Transitional collects crumbs; otherwise clear and exit
		if (GetUGType() != 2)
		{
			if (t.m_Data) { t.m_Data.Breadcrumbs = null; }
			Print("[UG] Not Transitional; cleared crumbs");
			return;
		}

		// Gather candidate objects within a bounding sphere
		ref array<Object> results = new array<Object>();
		float r = Math.Max(Math.Max(m_Size[0], m_Size[1]), m_Size[2]) * 1.5;
		GetGame().GetObjectsAtPosition3D(GetPosition(), r, results, null);

		ref array<ref JsonUndergroundAreaBreadcrumb> crumbs = new array<ref JsonUndergroundAreaBreadcrumb>();

		vector right, up, fwd, pos, half;
		foreach (Object obj : results)
		{
			if (!obj) continue;
			if (obj.GetType() != "UGBreadcrumb") continue;

			vector wp = obj.GetPosition();
			if (!IsPointInsideOBB(wp, right, up, fwd, pos, half)) continue;

			UGBreadcrumb crumb = UGBreadcrumb.Cast(obj);
			JsonUndergroundAreaBreadcrumb bc = new JsonUndergroundAreaBreadcrumb();
			bc.Position = new array<float>();
			bc.Position.Insert(wp[0]); bc.Position.Insert(wp[1]); bc.Position.Insert(wp[2]);
			if (crumb) {
				bc.EyeAccommodation = crumb.GetEyeAccommodation();
				bc.UseRaycast = crumb.GetUseRaycast();
				bc.Radius    = crumb.GetRadius();

			} else {
				bc.EyeAccommodation = 1.0;
				bc.UseRaycast = 0;
				bc.Radius    = -1.0;

			}

			crumbs.Insert(bc);
		}

		if (!t.m_Data)
		{
			JsonUndergroundAreaTriggerData d = BuildJsonFromUG(this);
			t.Init(d);
			Print("[UG] GetBreadcrumbs: created data blob via BuildJsonFromUG");
		}

		if (crumbs.Count() >= 2)
		{
			t.m_Data.Breadcrumbs = crumbs;

			// If the user wanted Transitional, now we can set live type safely
			if (m_DesiredUGType == 2)
				t.m_Type = EUndergroundTriggerType.TRANSITIONING;

			Print("[UG] attached " + crumbs.Count() + " breadcrumbs; live type set if desired Transitional");
		}
		else
		{
			t.m_Data.Breadcrumbs = null;
			Print("[UG][WARN] need >=2 breadcrumbs; cleared breadcrumb mode");
		}
	}

	// Newer entry point (also OBB/Transitional-only). Kept for completeness.
	void CollectBreadcrumbs()
	{
		UndergroundTrigger t = GetLinkedTrigger();
		if (!t) return;

		// Transitional only
		if (GetUGType() != 2)
		{
			if (t.m_Data) { t.m_Data.Breadcrumbs = null; }
			return;
		}

		ref array<Object> objs = new array<Object>();
		float r = Math.Max(Math.Max(m_Size[0], m_Size[1]), m_Size[2]) * 1.5;
		GetGame().GetObjectsAtPosition3D(GetPosition(), r, objs, null);

		ref array<ref JsonUndergroundAreaBreadcrumb> crumbs = new array<ref JsonUndergroundAreaBreadcrumb>();
		vector right, up, fwd, pos, half;

		for (int i = 0; i < objs.Count(); i++)
		{
			Object o = objs[i];
			if (!o) continue;
			if (o.GetType() != "UGBreadcrumb") continue;

			vector p = o.GetPosition();
			if (!IsPointInsideOBB(p, right, up, fwd, pos, half)) continue;

			UGBreadcrumb crumb = UGBreadcrumb.Cast(o);
			JsonUndergroundAreaBreadcrumb bc = new JsonUndergroundAreaBreadcrumb();
			bc.Position = new array<float>();
			bc.Position.Insert(p[0]); bc.Position.Insert(p[1]); bc.Position.Insert(p[2]);
			if (crumb) {
				bc.EyeAccommodation = crumb.GetEyeAccommodation();
				bc.UseRaycast = crumb.GetUseRaycast();
				bc.Radius    = crumb.GetRadius();
			} else {
				bc.EyeAccommodation = 1.0;
				bc.UseRaycast = 0;
				bc.Radius    = -1.0;
			}

			crumbs.Insert(bc);
		}

		if (!t.m_Data) t.m_Data = new JsonUndergroundAreaTriggerData();

		if (crumbs.Count() >= 2)
		{
			t.m_Data.Breadcrumbs = crumbs;

			// If the user wanted Transitional, set live type now
			if (m_DesiredUGType == 2)
				t.m_Type = EUndergroundTriggerType.TRANSITIONING;
		}
		else
		{
			t.m_Data.Breadcrumbs = null;
		}
	}

	// ===== Math / transform helpers =====

	void QueueCrumbRescan()
	{
	    if (m_RescanQueued) return;
	    m_RescanQueued = true;
	    GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.DoCrumbRescan, 100, false); // 0.1s throttle
	}

	void DoCrumbRescan()
	{
	    m_RescanQueued = false;
	    if (GetUGType() == 2) { CollectBreadcrumbs(); } // Transitional only
	}

	protected float UG_MapScaleToEyeAcco(float sc)
	{
		if (sc < 0.0) sc = 0.0;
		if (sc > 1.0) sc = 1.0;
		sc = Math.Round(sc * 100.0) / 100.0;
		return sc;
	}

	protected void UpdateTriggerPoseOnly()
	{
		if (!m_UndergroundTrigger) return;

		vector P[4];
		GetTransform(P);

		vector ax0 = P[0].Normalized();
		vector ax1 = P[1].Normalized();
		vector ax2 = Math3D.CrossProduct(ax0, ax1); ax2.Normalize();
		ax1 = Math3D.CrossProduct(ax2, ax0);        ax1.Normalize();

		vector M[4];
		M[0] = ax0;
		M[1] = ax1;
		M[2] = ax2;
		M[3] = GetPosition();

		m_UndergroundTrigger.SetTransform(M);
	}

	protected void UpdateTriggerExtentsOnly()
	{
		if (!m_UndergroundTrigger) return;
		vector e = m_Size * 0.5;
		m_UndergroundTrigger.SetExtents(-e, e);
		m_UndergroundTrigger.SetPosition(m_UndergroundTrigger.GetPosition()); // refresh on some builds
	}

	protected void UpdateTrigger()
	{
		CreateTriggerIfMissing();
		UpdateTriggerPoseOnly();
		UpdateTriggerExtentsOnly();
	}

	protected void ApplySizeTransform()
	{
		vector T[4];
		GetTransform(T);

		vector ax0 = T[0].Normalized();
		vector ax1 = T[1].Normalized();
		vector ax2 = T[2].Normalized();

		T[0] = ax0 * m_Size[0];
		T[1] = ax1 * m_Size[1];
		T[2] = ax2 * m_Size[2];

		SetTransform(T);
	}
}
void UG_RescanTriggersAround(vector center, float radius)
{
    ref array<Object> objs = new array<Object>();
    GetGame().GetObjectsAtPosition3D(center, radius, objs, null);
    for (int i = 0; i < objs.Count(); i++) {
        UGTriggerObject ug = UGTriggerObject.Cast(objs[i]);
        if (!ug) continue;
        if (ug.GetUGType() != 2) continue; // only Transitional cares
        ug.QueueCrumbRescan();
    }
}

float UG_Round2(float v)
{
	v = Math.Clamp(v, 0.0, 1.0);
	return Math.Round(v * 100.0) / 100.0;
}